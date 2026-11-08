// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RandomWorldGeneration/PCG/RoadGraph.h"

struct FRoadBuildParams
{
	FVector2D CityCenterXY = FVector2D::ZeroVector;
	float CityRadius = 2000.0f;
	float CityHeight = 8000.0f;
	
	int32 Seed = 1234;

	// 노드(도로) 개수
	int32 MinRoadNum = 50;
	int32 MaxRoadNum = 100;

	float RoadWidth = 400.0f;

	// 차수 2 노드를 빼고 양쪽 엣지를 합치는 최대 꺾임 각도(도)
	float MaxPassThroughBendAngle = 18.2f;

	// 차수 2 노드의 모서리를 깎는 원호의 기본 반지름
	float CurveRadius = 800.0f;

	// 원호 엣지 위 점 사이 간격(원호 길이)
	float CurveSegmentSpacing = 100.0f;

	// 교차로에서 이웃한 두 갈래의 경계선 사이를 잇는 원호의 최대 반지름
	float IntersectionCornerRadius = 400.0f;
};

/**
 * 
 */
class RWG_API FRoadGraphBuilder
{
private:
	/** 2D 좌표들의 Delaunay 삼각분할 엣지 목록 (X: 작은 인덱스, Y: 큰 인덱스) */
	static TArray<FIntPoint> BuildDelaunayEdges(const TArray<FVector2D>& Points);

	/** 엣지 목록 중 lune 안에 다른 점이 없는 엣지 목록 */
	static TArray<FIntPoint> FilterRNGEdges(const TArray<FVector2D>& Points, const TArray<FIntPoint>& Edges);

	/** 꺾임이 MaxBendAngle(도) 이하인 차수 2 노드를 빼고 양쪽 엣지를 직선 하나로 합친 그래프 (OutMergedEdgeIds: 합쳐서 새로 생긴 엣지 Id) */
	static FRoadGraph MergePassThroughNodes(const FRoadGraph& Graph, float MaxBendAngle, TArray<int32>& OutMergedEdgeIds);

	/** 합쳐서 새로 생긴 엣지와 XY 평면에서 교차하는 엣지를 로그로 남긴다 */
	static void LogMergedEdgeCrossings(const FRoadGraph& Graph, const TArray<int32>& MergedEdgeIds);

	/** 차수 2 노드의 모서리를 원호로 깎아, 그 노드 대신 원호 양 끝 노드와 원호 엣지를 넣은 그래프 (CurveSegmentSpacing: 원호 위 점 간격) */
	static FRoadGraph BuildCurveEdges(const FRoadGraph& Graph, float CurveRadius, float CurveSegmentSpacing);

	/** 차수 3 이상 노드에 붙은 갈래 끝을 잘라내고, 그 자리를 덮는 교차로 윤곽을 넣은 그래프 (MaxCornerRadius: 갈래 경계선 사이 원호의 최대 반지름, ArcSpacing: 원호 위 점 간격) */
	static FRoadGraph BuildIntersections(const FRoadGraph& Graph, float MaxCornerRadius, float ArcSpacing);

public:
	static FRoadGraph BuildGraph2D(UWorld* InWorld, const FRoadBuildParams& InParams)
	{
		if (!InWorld) return FRoadGraph();

		FRoadGraph Graph;

		float TraceStartZOffset = InParams.CityHeight * 1.5f;
		float TraceDistance = InParams.CityHeight;

		FRandomStream RoadStream(InParams.Seed + 1);
		const int32 RoadNum = RoadStream.RandRange(InParams.MinRoadNum, InParams.MaxRoadNum);

		// 1) 노드 생성
		for (int32 i = 0; i < RoadNum; ++i)
		{
			const float Angle = RoadStream.FRandRange(0.0f, 2.0f * PI);
			const float Radius = FMath::Sqrt(RoadStream.FRand()) * InParams.CityRadius;

			const float X = InParams.CityCenterXY.X + FMath::Cos(Angle) * Radius;
			const float Y = InParams.CityCenterXY.Y + FMath::Sin(Angle) * Radius;
			
			FVector TraceStartPos = FVector(X, Y, TraceStartZOffset);
			FVector TraceEndPos = TraceStartPos;
			TraceEndPos.Z -= TraceDistance;

			FHitResult Result;
			FVector NodePos = FVector(X, Y, InParams.CityHeight);

			if (InWorld->LineTraceSingleByChannel(Result, TraceStartPos, TraceEndPos, ECollisionChannel::ECC_Visibility))
			{
				NodePos = Result.ImpactPoint;
			}

			NodePos.Z += 1.0f;

			Graph.AddNode(NodePos);
		}

		// 2) 엣지 생성
		// Delaunay 삼각분할 엣지 중 RNG 조건을 만족하는 엣지만 연결한다.
		TArray<FVector2D> Points;
		Points.Reserve(Graph.GetNodes().Num());
		for (const FRoadNode& Node : Graph.GetNodes())
		{
			Points.Add(FVector2D(Node.Position.X, Node.Position.Y));
		}

		const TArray<FIntPoint> DelaunayEdges = BuildDelaunayEdges(Points);
		const TArray<FIntPoint> RNGEdges = FilterRNGEdges(Points, DelaunayEdges);

		for (const FIntPoint& Edge : RNGEdges)
		{
			Graph.AddEdge(Edge.X, Edge.Y, InParams.RoadWidth);
		}

		// 3) 차수 2 노드 합치기
		// 직선에 가까운 차수 2 노드를 빼고 양쪽 엣지를 하나로 합친다.
		TArray<int32> MergedEdgeIds;
		Graph = MergePassThroughNodes(Graph, InParams.MaxPassThroughBendAngle, MergedEdgeIds);

#if !UE_BUILD_SHIPPING
		LogMergedEdgeCrossings(Graph, MergedEdgeIds);
#endif

		// 4) 곡선 엣지 만들기
		// 남은 차수 2 노드의 모서리를 원호로 깎는다.
		Graph = BuildCurveEdges(Graph, InParams.CurveRadius, InParams.CurveSegmentSpacing);

		// 5) 교차로 만들기
		// 차수 3 이상 노드에 붙은 갈래 끝을 잘라내고 그 자리를 덮을 윤곽을 만든다.
		Graph = BuildIntersections(Graph, InParams.IntersectionCornerRadius, InParams.CurveSegmentSpacing);

		return Graph;
	}
};
