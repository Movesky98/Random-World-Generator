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

	static TArray<FCityBlock> BuildBlocks(FCityGrid& CityGrid);

	static FCityBlock BlockFloodFill(FCityGrid& Grid, const int32 StartX, const int32 StartY, const int32 InBlockId);

	static void ExtractLotsFromBlock(FCityGrid& Grid, const TArray<FCityBlock>& Blocks);

	static void BuildBaseBuildableMask(FCityGrid& Grid);

	static bool IsBuildableCell(const FCityGrid& Grid, int32 X, int32 Y, int32 BlockId, const TArray<bool>& Used);

	static bool IsRectangleValid(const FCityGrid& Grid, int32 StartX, int32 StartY, int32 Width, int32 Height, int32 BlockId, const TArray<bool>& Used);

	static bool FindBestRectangle(const FCityGrid& Grid, const TArray<bool>& Used, int32 StartX, int32 StartY, int32 BlockId, int32& OutWidth, int32& OutHeight);

	static FCityLot MarkLotFromRectangle(const FCityGrid& Grid, TArray<bool>& Used, int32 BlockId, int32 LotId, int32 StartX, int32 StartY, int32 Width, int32 Height);

private:
	static TArray<bool> BaseBuildableCell;
};
