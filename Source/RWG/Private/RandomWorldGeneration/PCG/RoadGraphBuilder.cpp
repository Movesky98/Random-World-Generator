// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/PCG/RoadGraphBuilder.h"

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
