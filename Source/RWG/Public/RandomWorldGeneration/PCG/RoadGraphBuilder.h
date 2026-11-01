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

		return Graph;
	}
};
