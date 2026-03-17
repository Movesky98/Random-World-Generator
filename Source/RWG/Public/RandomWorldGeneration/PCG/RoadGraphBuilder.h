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
		// 가장 가까운 거리에 있는 이웃 2개를 연결한다.
		for (int32 i = 0; i < Graph.GetNodes().Num(); ++i)
		{
			int32 BestNeighbor1 = INDEX_NONE, BestNeighbor2 = INDEX_NONE;
			float MinDist1 = TNumericLimits<float>::Max();
			float MinDist2 = TNumericLimits<float>::Max();

			// Node[i]'s Position
			const FVector2D Pi(Graph.GetNode(i)->Position.X, Graph.GetNode(i)->Position.Y);

			for (int32 j = 0; j < Graph.GetNodes().Num(); j++)
			{
				if (i == j) continue;

				// Node[j]'s Position
				const FVector2D Pj(Graph.GetNode(j)->Position.X, Graph.GetNode(j)->Position.Y);
				const float Distance = FVector2D::Distance(Pi, Pj);

				if (Distance < MinDist1)
				{
					MinDist2 = MinDist1;
					BestNeighbor2 = BestNeighbor1;

					MinDist1 = Distance;
					BestNeighbor1 = j;
				}
				else if (Distance < MinDist2)
				{
					MinDist2 = Distance;
					BestNeighbor2 = j;
				}
			}

			if (BestNeighbor1 != INDEX_NONE) Graph.AddEdge(i, BestNeighbor1, InParams.RoadWidth);
			if (BestNeighbor2 != INDEX_NONE) Graph.AddEdge(i, BestNeighbor2, InParams.RoadWidth);
		}

		return Graph;
	}
};
