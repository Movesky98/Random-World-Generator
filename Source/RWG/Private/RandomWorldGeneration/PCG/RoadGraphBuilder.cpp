// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/PCG/RoadGraphBuilder.h"
#include "CommonLogCategories.h"

namespace
{
	struct FDelaunayTriangle
	{
		// 꼭짓점 인덱스
		int32 A = INDEX_NONE;
		int32 B = INDEX_NONE;
		int32 C = INDEX_NONE;

		// 외접원 중심과 반지름의 제곱
		FVector2D Circumcenter = FVector2D::ZeroVector;
		double RadiusSquared = 0.0;
	};

	FDelaunayTriangle MakeTriangle(const TArray<FVector2D>& Points, int32 A, int32 B, int32 C)
	{
		const FVector2D& PA = Points[A];
		const FVector2D& PB = Points[B];
		const FVector2D& PC = Points[C];

		const double D = 2.0 * (PA.X * (PB.Y - PC.Y) + PB.X * (PC.Y - PA.Y) + PC.X * (PA.Y - PB.Y));
		const double LenA = PA.SizeSquared();
		const double LenB = PB.SizeSquared();
		const double LenC = PC.SizeSquared();

		FDelaunayTriangle Triangle;
		Triangle.A = A;
		Triangle.B = B;
		Triangle.C = C;
		Triangle.Circumcenter.X = (LenA * (PB.Y - PC.Y) + LenB * (PC.Y - PA.Y) + LenC * (PA.Y - PB.Y)) / D;
		Triangle.Circumcenter.Y = (LenA * (PC.X - PB.X) + LenB * (PA.X - PC.X) + LenC * (PB.X - PA.X)) / D;
		Triangle.RadiusSquared = FVector2D::DistSquared(PA, Triangle.Circumcenter);

		return Triangle;
	}

	FIntPoint MakeEdgeKey(int32 A, int32 B)
	{
		return A < B ? FIntPoint(A, B) : FIntPoint(B, A);
	}

	struct FMergedEdge
	{
		// 원래 그래프 기준 양 끝 노드 Id
		int32 StartNodeId = INDEX_NONE;
		int32 EndNodeId = INDEX_NONE;

		float Width = 400.0f;
		int32 RoadType = 0;

		// 차수 2 노드를 빼고 합쳐서 생긴 엣지인지
		bool bMerged = false;
	};

	// 두 선분이 끝점이 아닌 곳에서 서로 가로지르는지
	bool DoSegmentsCross(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D)
	{
		const double SideC = FVector2D::CrossProduct(B - A, C - A);
		const double SideD = FVector2D::CrossProduct(B - A, D - A);
		const double SideA = FVector2D::CrossProduct(D - C, A - C);
		const double SideB = FVector2D::CrossProduct(D - C, B - C);

		return SideC * SideD < 0.0 && SideA * SideB < 0.0;
	}
}

TArray<FIntPoint> FRoadGraphBuilder::BuildDelaunayEdges(const TArray<FVector2D>& Points)
{
	const int32 PointNum = Points.Num();
	if (PointNum < 2) return TArray<FIntPoint>();

	// 모든 점을 덮는 슈퍼 삼각형 꼭짓점을 점 목록 뒤에 붙인다 (인덱스 PointNum ~ PointNum + 2)
	FVector2D Min = Points[0];
	FVector2D Max = Points[0];
	for (const FVector2D& Point : Points)
	{
		Min = FVector2D::Min(Min, Point);
		Max = FVector2D::Max(Max, Point);
	}

	const FVector2D Center = (Min + Max) * 0.5;
	const double Size = FMath::Max((Max - Min).GetMax(), 1.0) * 100.0;

	TArray<FVector2D> AllPoints = Points;
	AllPoints.Add(FVector2D(Center.X, Center.Y + Size));
	AllPoints.Add(FVector2D(Center.X - Size, Center.Y - Size));
	AllPoints.Add(FVector2D(Center.X + Size, Center.Y - Size));

	// 삼각형 목록
	TArray<FDelaunayTriangle> Triangles;
	Triangles.Add(MakeTriangle(AllPoints, PointNum, PointNum + 1, PointNum + 2));

	// 점을 하나씩 추가한다
	for (int32 P = 0; P < PointNum; ++P)
	{
		const FVector2D& NewPoint = AllPoints[P];

		TArray<FDelaunayTriangle> KeptTriangles;
		TMap<FIntPoint, int32> EdgeCounts;

		// 1. 외접원 안에 새 점이 들어온 삼각형(bad triangle)을 빼고, 그 변이 나온 횟수를 센다
		for (const FDelaunayTriangle& Triangle : Triangles)
		{
			if (FVector2D::DistSquared(NewPoint, Triangle.Circumcenter) < Triangle.RadiusSquared)
			{
				EdgeCounts.FindOrAdd(MakeEdgeKey(Triangle.A, Triangle.B))++;
				EdgeCounts.FindOrAdd(MakeEdgeKey(Triangle.B, Triangle.C))++;
				EdgeCounts.FindOrAdd(MakeEdgeKey(Triangle.C, Triangle.A))++;
			}
			else
			{
				KeptTriangles.Add(Triangle);
			}
		}

		// 2. 한 번만 나온 변(polygonal hole의 경계)마다 새 점과 이어 삼각형을 만든다
		for (const TPair<FIntPoint, int32>& EdgeCount : EdgeCounts)
		{
			if (EdgeCount.Value == 1)
			{
				KeptTriangles.Add(MakeTriangle(AllPoints, EdgeCount.Key.X, EdgeCount.Key.Y, P));
			}
		}

		Triangles = MoveTemp(KeptTriangles);
	}

	// 슈퍼 삼각형 꼭짓점을 쓰지 않는 삼각형의 변을 중복 없이 모은다
	TSet<FIntPoint> EdgeSet;
	for (const FDelaunayTriangle& Triangle : Triangles)
	{
		if (Triangle.A >= PointNum || Triangle.B >= PointNum || Triangle.C >= PointNum) continue;

		EdgeSet.Add(MakeEdgeKey(Triangle.A, Triangle.B));
		EdgeSet.Add(MakeEdgeKey(Triangle.B, Triangle.C));
		EdgeSet.Add(MakeEdgeKey(Triangle.C, Triangle.A));
	}

	return EdgeSet.Array();
}

TArray<FIntPoint> FRoadGraphBuilder::FilterRNGEdges(const TArray<FVector2D>& Points, const TArray<FIntPoint>& Edges)
{
	TArray<FIntPoint> Result;

	for (const FIntPoint& Edge : Edges)
	{
		const double EdgeLengthSquared = FVector2D::DistSquared(Points[Edge.X], Points[Edge.Y]);

		// 두 끝점 모두에게 엣지 길이보다 가까운 점이 있으면 lune 안에 있다
		bool bBlocked = false;
		for (int32 R = 0; R < Points.Num(); ++R)
		{
			if (R == Edge.X || R == Edge.Y) continue;

			if (FVector2D::DistSquared(Points[R], Points[Edge.X]) < EdgeLengthSquared
				&& FVector2D::DistSquared(Points[R], Points[Edge.Y]) < EdgeLengthSquared)
			{
				bBlocked = true;
				break;
			}
		}

		if (!bBlocked)
		{
			Result.Add(Edge);
		}
	}

	return Result;
}

FRoadGraph FRoadGraphBuilder::MergePassThroughNodes(const FRoadGraph& Graph, float MaxBendAngle, TArray<int32>& OutMergedEdgeIds)
{
	OutMergedEdgeIds.Reset();

	const TArray<FRoadNode>& Nodes = Graph.GetNodes();
	const TArray<FRoadEdge>& Edges = Graph.GetEdges();
	const float StraightDotThreshold = FMath::Cos(FMath::DegreesToRadians(MaxBendAngle));

	TArray<bool> bRemovedNodes;
	bRemovedNodes.Init(false, Nodes.Num());

	TArray<bool> bVisitedEdges;
	bVisitedEdges.Init(false, Edges.Num());

	TArray<FMergedEdge> MergedEdges;

	// 차수 2가 아닌 노드에서 출발해 차수 2 노드를 따라가며 뺄지 판정한다
	for (const FRoadNode& StartNode : Nodes)
	{
		if (Graph.GetNodeDegree(StartNode.Id) == 2) continue;

		for (const int32 StartEdgeId : StartNode.EdgeIds)
		{
			if (bVisitedEdges[StartEdgeId]) continue;
			bVisitedEdges[StartEdgeId] = true;

			const FRoadEdge& StartEdge = Edges[StartEdgeId];

			// 마지막으로 남긴 노드
			int32 PrevNodeId = StartNode.Id;
			int32 CurrentNodeId = Graph.GetOtherNode(StartEdgeId, StartNode.Id);
			int32 LastEdgeId = StartEdgeId;
			bool bMerged = false;

			while (Graph.GetNodeDegree(CurrentNodeId) == 2)
			{
				const FRoadNode& CurrentNode = Nodes[CurrentNodeId];
				const int32 NextEdgeId = (CurrentNode.EdgeIds[0] == LastEdgeId) ? CurrentNode.EdgeIds[1] : CurrentNode.EdgeIds[0];
				const int32 NextNodeId = Graph.GetOtherNode(NextEdgeId, CurrentNodeId);
				bVisitedEdges[NextEdgeId] = true;

				if (Graph.IsPassThrough(PrevNodeId, CurrentNodeId, NextNodeId, StraightDotThreshold))
				{
					// 현재 노드를 빼고 PrevNodeId에서 계속 잇는다
					bRemovedNodes[CurrentNodeId] = true;
					bMerged = true;
				}
				else
				{
					// 현재 노드를 남기고 여기까지를 엣지 하나로 확정한다
					MergedEdges.Add({ PrevNodeId, CurrentNodeId, StartEdge.Width, StartEdge.RoadType, bMerged });
					PrevNodeId = CurrentNodeId;
					bMerged = false;
				}

				LastEdgeId = NextEdgeId;
				CurrentNodeId = NextNodeId;
			}

			MergedEdges.Add({ PrevNodeId, CurrentNodeId, StartEdge.Width, StartEdge.RoadType, bMerged });
		}
	}

	// 모든 노드가 차수 2인 순환은 출발할 노드가 없으므로 원래 엣지를 그대로 둔다
	for (const FRoadEdge& Edge : Edges)
	{
		if (bVisitedEdges[Edge.Id]) continue;

		MergedEdges.Add({ Edge.StartNodeId, Edge.EndNodeId, Edge.Width, Edge.RoadType, false });
	}

	// 남은 노드와 확정한 엣지로 그래프를 새로 만든다
	FRoadGraph Result;

	TArray<int32> NewNodeIds;
	NewNodeIds.Init(INDEX_NONE, Nodes.Num());

	for (const FRoadNode& Node : Nodes)
	{
		if (bRemovedNodes[Node.Id]) continue;

		NewNodeIds[Node.Id] = Result.AddNode(Node.Position);
	}

	for (const FMergedEdge& Edge : MergedEdges)
	{
		const int32 NewEdgeId = Result.AddEdge(NewNodeIds[Edge.StartNodeId], NewNodeIds[Edge.EndNodeId], Edge.Width, Edge.RoadType);

		if (Edge.bMerged && NewEdgeId != INDEX_NONE)
		{
			OutMergedEdgeIds.Add(NewEdgeId);
		}
	}

	return Result;
}

void FRoadGraphBuilder::LogMergedEdgeCrossings(const FRoadGraph& Graph, const TArray<int32>& MergedEdgeIds)
{
	const TArray<FRoadNode>& Nodes = Graph.GetNodes();
	const TArray<FRoadEdge>& Edges = Graph.GetEdges();

	TArray<bool> bMergedEdges;
	bMergedEdges.Init(false, Edges.Num());
	for (const int32 EdgeId : MergedEdgeIds)
	{
		bMergedEdges[EdgeId] = true;
	}

	int32 CrossingNum = 0;

	for (const int32 MergedEdgeId : MergedEdgeIds)
	{
		const FRoadEdge& MergedEdge = Edges[MergedEdgeId];
		const FVector& A = Nodes[MergedEdge.StartNodeId].Position;
		const FVector& B = Nodes[MergedEdge.EndNodeId].Position;

		for (const FRoadEdge& Other : Edges)
		{
			// 합친 엣지끼리는 한 번만 비교한다
			if (Other.Id == MergedEdgeId || (bMergedEdges[Other.Id] && Other.Id < MergedEdgeId)) continue;

			// 끝점을 공유하는 엣지는 교차가 아니다
			if (Other.StartNodeId == MergedEdge.StartNodeId || Other.StartNodeId == MergedEdge.EndNodeId
				|| Other.EndNodeId == MergedEdge.StartNodeId || Other.EndNodeId == MergedEdge.EndNodeId) continue;

			const FVector& C = Nodes[Other.StartNodeId].Position;
			const FVector& D = Nodes[Other.EndNodeId].Position;

			if (DoSegmentsCross(FVector2D(A), FVector2D(B), FVector2D(C), FVector2D(D)))
			{
				++CrossingNum;
				COMMON_LOG(LogRandomWorldGen, Warning, TEXT("Merged edge crossing: Edge %d (Node %d %s - Node %d %s) x Edge %d (Node %d %s - Node %d %s)"),
					MergedEdgeId, MergedEdge.StartNodeId, *A.ToString(), MergedEdge.EndNodeId, *B.ToString(),
					Other.Id, Other.StartNodeId, *C.ToString(), Other.EndNodeId, *D.ToString());
			}
		}
	}

	COMMON_LOG(LogRandomWorldGen, Warning, TEXT("Merged edge crossings: %d (merged edges: %d)"), CrossingNum, MergedEdgeIds.Num());
}

FRoadGraph FRoadGraphBuilder::BuildCurveEdges(const FRoadGraph& Graph, float CurveRadius, float CurveSegmentSpacing)
{
	const TArray<FRoadNode>& Nodes = Graph.GetNodes();
	const TArray<FRoadEdge>& Edges = Graph.GetEdges();
	const float Spacing = FMath::Max(10.0f, CurveSegmentSpacing);

	// 같은 위치로 보는 거리
	const float TouchTolerance = 1.0f;

	// 노드에서 원호가 양쪽 엣지에 닿는 점까지의 XY 거리 (0: 곡선을 만들지 않는 노드)
	TArray<float> BaseDistances;
	BaseDistances.Init(0.0f, Nodes.Num());

	// 1. 차수 2 노드마다 기본 반지름으로 거리를 구한다
	for (const FRoadNode& Node : Nodes)
	{
		if (Node.EdgeIds.Num() != 2) continue;

		const FVector2D NodeXY(Node.Position);
		const FVector2D DirToPrev = (FVector2D(Nodes[Graph.GetOtherNode(Node.EdgeIds[0], Node.Id)].Position) - NodeXY).GetSafeNormal();
		const FVector2D DirToNext = (FVector2D(Nodes[Graph.GetOtherNode(Node.EdgeIds[1], Node.Id)].Position) - NodeXY).GetSafeNormal();
		if (DirToPrev.IsNearlyZero() || DirToNext.IsNearlyZero()) continue;

		// 두 엣지 사이 각의 절반
		const float HalfAngle = 0.5f * FMath::Acos(FMath::Clamp(FVector2D::DotProduct(DirToPrev, DirToNext), -1.0f, 1.0f));
		const float HalfTan = FMath::Tan(HalfAngle);
		if (HalfTan < KINDA_SMALL_NUMBER) continue;

		const float Distance = CurveRadius / HalfTan;
		if (Distance < TouchTolerance) continue;

		BaseDistances[Node.Id] = Distance;
	}

	// 2. 두 원호가 겹치는 엣지는 양 끝 거리를 같은 비율로 줄이고, 노드는 두 엣지 중 더 작은 값을 쓴다
	TArray<float> CurveDistances = BaseDistances;

	for (const FRoadEdge& Edge : Edges)
	{
		const float EdgeLength = FVector2D::Distance(FVector2D(Nodes[Edge.StartNodeId].Position), FVector2D(Nodes[Edge.EndNodeId].Position));
		const float DistanceSum = BaseDistances[Edge.StartNodeId] + BaseDistances[Edge.EndNodeId];
		if (DistanceSum <= EdgeLength) continue;

		const float Scale = EdgeLength / DistanceSum;
		CurveDistances[Edge.StartNodeId] = FMath::Min(CurveDistances[Edge.StartNodeId], BaseDistances[Edge.StartNodeId] * Scale);
		CurveDistances[Edge.EndNodeId] = FMath::Min(CurveDistances[Edge.EndNodeId], BaseDistances[Edge.EndNodeId] * Scale);
	}

	// 3. 겹쳤던 엣지에 남은 길이는, 노드의 반대쪽 엣지에서도 겹치지 않는 만큼만 원호를 키워 채운다
	for (const FRoadEdge& Edge : Edges)
	{
		const float EdgeLength = FVector2D::Distance(FVector2D(Nodes[Edge.StartNodeId].Position), FVector2D(Nodes[Edge.EndNodeId].Position));
		if (BaseDistances[Edge.StartNodeId] + BaseDistances[Edge.EndNodeId] <= EdgeLength) continue;

		const int32 EndNodeIds[2] = { Edge.StartNodeId, Edge.EndNodeId };

		for (int32 i = 0; i < 2; ++i)
		{
			const int32 NodeId = EndNodeIds[i];
			const int32 FacingNodeId = EndNodeIds[1 - i];
			if (CurveDistances[NodeId] <= 0.0f) continue;

			const float RemainLength = EdgeLength - CurveDistances[NodeId] - CurveDistances[FacingNodeId];
			if (RemainLength <= 0.0f) continue;

			// 이 노드의 반대쪽 엣지에서 더 쓸 수 있는 길이
			const FRoadNode& Node = Nodes[NodeId];
			const int32 OtherEdgeId = (Node.EdgeIds[0] == Edge.Id) ? Node.EdgeIds[1] : Node.EdgeIds[0];
			const int32 FarNodeId = Graph.GetOtherNode(OtherEdgeId, NodeId);
			const float OtherEdgeLength = FVector2D::Distance(FVector2D(Node.Position), FVector2D(Nodes[FarNodeId].Position));
			const float OtherRoom = OtherEdgeLength - CurveDistances[NodeId] - CurveDistances[FarNodeId];

			CurveDistances[NodeId] += FMath::Min(RemainLength, FMath::Max(0.0f, OtherRoom));
		}
	}

	// 4. 곡선을 만들지 않는 노드는 그대로 옮긴다
	FRoadGraph Result;

	TArray<int32> NewNodeIds;
	NewNodeIds.Init(INDEX_NONE, Nodes.Num());

	for (const FRoadNode& Node : Nodes)
	{
		if (CurveDistances[Node.Id] > 0.0f) continue;

		NewNodeIds[Node.Id] = Result.AddNode(Node.Position);
	}

	// 5. 엣지마다 원호가 닿는 점에 노드를 만들고 그 사이를 직선 엣지로 잇는다 (X: 시작 쪽 노드 Id, Y: 끝 쪽 노드 Id)
	TArray<FIntPoint> CutNodeIds;
	CutNodeIds.Init(FIntPoint(INDEX_NONE, INDEX_NONE), Edges.Num());

	for (const FRoadEdge& Edge : Edges)
	{
		const FVector& StartPos = Nodes[Edge.StartNodeId].Position;
		const FVector& EndPos = Nodes[Edge.EndNodeId].Position;
		const float EdgeLength = FVector2D::Distance(FVector2D(StartPos), FVector2D(EndPos));
		if (EdgeLength < KINDA_SMALL_NUMBER) continue;

		const float StartDistance = CurveDistances[Edge.StartNodeId];
		const float EndDistance = CurveDistances[Edge.EndNodeId];

		int32 StartCutId = (StartDistance > 0.0f) ? INDEX_NONE : NewNodeIds[Edge.StartNodeId];
		int32 EndCutId = (EndDistance > 0.0f) ? INDEX_NONE : NewNodeIds[Edge.EndNodeId];

		const FVector StartCutPos = FMath::Lerp(StartPos, EndPos, StartDistance / EdgeLength);
		const FVector EndCutPos = FMath::Lerp(EndPos, StartPos, EndDistance / EdgeLength);

		if (EdgeLength - StartDistance - EndDistance <= TouchTolerance)
		{
			// 두 점이 맞닿으면 길이 0 엣지를 만들지 않고 노드 하나를 같이 쓴다
			int32 SharedId = (StartCutId != INDEX_NONE) ? StartCutId : EndCutId;
			if (SharedId == INDEX_NONE)
			{
				SharedId = Result.AddNode(StartCutPos);
			}

			StartCutId = SharedId;
			EndCutId = SharedId;
		}
		else
		{
			if (StartCutId == INDEX_NONE)
			{
				StartCutId = Result.AddNode(StartCutPos);
			}

			if (EndCutId == INDEX_NONE)
			{
				EndCutId = Result.AddNode(EndCutPos);
			}

			Result.AddEdge(StartCutId, EndCutId, Edge.Width, Edge.RoadType);
		}

		CutNodeIds[Edge.Id] = FIntPoint(StartCutId, EndCutId);
	}

	// 6. 곡선 노드마다 양쪽 엣지에 만든 노드를 원호 엣지로 잇는다
	for (const FRoadNode& Node : Nodes)
	{
		const float Distance = CurveDistances[Node.Id];
		if (Distance <= 0.0f) continue;

		const FRoadEdge& PrevEdge = Edges[Node.EdgeIds[0]];
		const FRoadEdge& NextEdge = Edges[Node.EdgeIds[1]];

		const int32 ArcStartId = (PrevEdge.StartNodeId == Node.Id) ? CutNodeIds[PrevEdge.Id].X : CutNodeIds[PrevEdge.Id].Y;
		const int32 ArcEndId = (NextEdge.StartNodeId == Node.Id) ? CutNodeIds[NextEdge.Id].X : CutNodeIds[NextEdge.Id].Y;
		if (ArcStartId == INDEX_NONE || ArcEndId == INDEX_NONE || ArcStartId == ArcEndId) continue;

		const FVector ArcStartPos = Result.GetNode(ArcStartId)->Position;
		const FVector ArcEndPos = Result.GetNode(ArcEndId)->Position;

		// 원 중심: 노드에서 두 엣지 사이 각의 이등분선 위
		const FVector2D NodeXY(Node.Position);
		const FVector2D DirToPrev = (FVector2D(ArcStartPos) - NodeXY).GetSafeNormal();
		const FVector2D DirToNext = (FVector2D(ArcEndPos) - NodeXY).GetSafeNormal();
		const FVector2D Bisector = (DirToPrev + DirToNext).GetSafeNormal();
		const float HalfAngle = 0.5f * FMath::Acos(FMath::Clamp(FVector2D::DotProduct(DirToPrev, DirToNext), -1.0f, 1.0f));

		const float Radius = Distance * FMath::Tan(HalfAngle);
		const FVector2D Center = NodeXY + Bisector * (Distance / FMath::Cos(HalfAngle));

		// 원호 시작 각과 진행 각 (-PI ~ PI)
		const FVector2D StartOffset = FVector2D(ArcStartPos) - Center;
		const FVector2D EndOffset = FVector2D(ArcEndPos) - Center;
		const float StartAngle = FMath::Atan2(StartOffset.Y, StartOffset.X);
		const float SweepAngle = FMath::UnwindRadians(FMath::Atan2(EndOffset.Y, EndOffset.X) - StartAngle);

		const int32 NumSegment = FMath::Max(1, FMath::CeilToInt(FMath::Abs(SweepAngle) * Radius / Spacing));

		TArray<FVector> ArcPoints;
		ArcPoints.Reserve(NumSegment + 1);
		ArcPoints.Add(ArcStartPos);

		for (int32 i = 1; i < NumSegment; ++i)
		{
			const float Alpha = (float)i / (float)NumSegment;
			const float Angle = StartAngle + SweepAngle * Alpha;
			const FVector2D PointXY = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;

			ArcPoints.Add(FVector(PointXY.X, PointXY.Y, FMath::Lerp(ArcStartPos.Z, ArcEndPos.Z, Alpha)));
		}

		ArcPoints.Add(ArcEndPos);

		Result.AddCurveEdge(ArcStartId, ArcEndId, ArcPoints, PrevEdge.Width, PrevEdge.RoadType);
	}

	return Result;
}
