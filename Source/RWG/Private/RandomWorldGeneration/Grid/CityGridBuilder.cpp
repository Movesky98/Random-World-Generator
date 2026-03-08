// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Grid/CityGridBuilder.h"

FCityGrid FCityGridBuilder::BuildGrid2D(const FGridBuildParams& InParams)
{
	FCityGrid NewGrid;

	const FVector Origin = FVector(
		InParams.CityCenter.X - InParams.CityRadius,
		InParams.CityCenter.Y - InParams.CityRadius,
		0.0f
	);

	NewGrid.SetOirin(Origin);
	NewGrid.SetCellSize(InParams.CellSize);
	NewGrid.SetWidth((InParams.CityRadius * 2) / InParams.CellSize);
	NewGrid.SetHeight((InParams.CityRadius * 2) / InParams.CellSize);

	TArray<FCityCell>& Cells = NewGrid.GetCityCellsMutable();

	Cells.SetNum(NewGrid.GetWidth() * NewGrid.GetHeight());

	for (int i = 0; i < Cells.Num(); i++)
	{
		int X = i % NewGrid.GetWidth();
		int Y = i / NewGrid.GetWidth();
		FVector CellPos = FVector(Origin.X + (X + 0.5f) * InParams.CellSize, Origin.Y + (Y + 0.5f) * InParams.CellSize, 0.0f);

		Cells[i].X = X;
		Cells[i].Y = Y;

		Cells[i].WorldPosition = CellPos;
	}

	MarkRoadCells(NewGrid, InParams);
	MarkOutsideAsBlocked(NewGrid, InParams);

	return NewGrid;
}

void FCityGridBuilder::MarkRoadCells(FCityGrid& CityGrid, const FGridBuildParams& InParams)
{
	TArray<FCityCell>& Cells = CityGrid.GetCityCellsMutable();

	for (int i = 0; i < Cells.Num(); i++)
	{
		FVector2D Cell2DPos = FVector2D(Cells[i].WorldPosition.X, Cells[i].WorldPosition.Y);

		const TArray<FRoadEdge>& Edges = InParams.RoadGraph.GetEdges();
		bool bIsRoad = false;
		for (const FRoadEdge& Edge : Edges)
		{
			for (const FVector& SegmentPos : Edge.SegmentPoints)
			{
				FVector2D Segment2DPos = FVector2D(SegmentPos.X, SegmentPos.Y);
				if (FVector2D::Distance(Segment2DPos, Cell2DPos) < Edge.HalfWidth)
				{
					Cells[i].Type = ECellType::Road;
					bIsRoad = true;
					break;
				}
			}

			if (bIsRoad)
				break;
		}
	}
}

void FCityGridBuilder::MarkOutsideAsBlocked(FCityGrid& CityGrid, const FGridBuildParams& InParams)
{
	TArray<FCityCell>& Cells = CityGrid.GetCityCellsMutable();

	for (int i = 0; i < Cells.Num(); i++)
	{
		FVector2D Cell2DPos = FVector2D(Cells[i].WorldPosition.X, Cells[i].WorldPosition.Y);
		FVector2D CityCenter2D = FVector2D(InParams.CityCenter.X, InParams.CityCenter.Y);

		if (FVector2D::Distance(Cell2DPos, CityCenter2D) > InParams.CityRadius)
		{
			Cells[i].Type = ECellType::Blocked;
		}
	}
}
