// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RandomWorldGeneration/Grid/CityGridTypes.h"

struct FGridBuildParams
{
	FVector CityCenter = FVector::ZeroVector;
	float CityRadius = 400.0f;
	float CellSize = 400.0f;
	FRoadGraph RoadGraph = FRoadGraph();
};

/**
 * 
 */
class RWG_API FCityGridBuilder
{
public:
	static FCityGrid BuildGrid2D(const FGridBuildParams& InParams);

	static void MarkRoadCells(FCityGrid& CityGrid, const FGridBuildParams& InParams);

	static void MarkOutsideAsBlocked(FCityGrid& CityGrid, const FGridBuildParams& InParams);
};
