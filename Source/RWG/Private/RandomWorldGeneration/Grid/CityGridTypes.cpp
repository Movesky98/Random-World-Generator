
#include "RandomWorldGeneration/Grid/CityGridTypes.h"

const FVector FCityGrid::GetOrigin() const
{
	return Origin;
}

void FCityGrid::SetOirin(const FVector InOrigin)
{
	Origin = InOrigin;
}

const int32 FCityGrid::GetWidth() const
{
	return Width;
}

void FCityGrid::SetWidth(const int32 InWidth)
{
	Width = InWidth;
}

const int32 FCityGrid::GetHeight() const
{
	return Height;
}

void FCityGrid::SetHeight(const int32 InHeight)
{
	Height = InHeight;
}

void FCityGrid::SetCellSize(const float InCellSize)
{
	CellSize = InCellSize;
}

const TArray<FCityCell> FCityGrid::GetCityCells() const
{
	return CityCells;
}

TArray<FCityCell>& FCityGrid::GetCityCellsMutable()
{
	return CityCells;
}

void FCityGrid::GenerateGrid(FVector CityCenter, float CityRadius, float InCellSize, const FRoadGraph& RoadGraph)
{
	// AABB 형태의 Grid Origin을 사용함. (Origin != CityCenter)
	Origin = FVector(CityCenter.X - CityRadius, CityCenter.Y - CityRadius, 0.0f);

	// 도시는 원형
	Width = (CityRadius * 2) / InCellSize;
	Height = Width;

	CellSize = InCellSize;

	// Cell 개수 초기화
	CityCells.SetNum(Width * Height);

	for (int i = 0; i < CityCells.Num(); i++)
	{
		int32 X = i % Width;
		int32 Y = i / Width;
		FVector CellPos = FVector(Origin.X + (X + 0.5f) * CellSize, Origin.Y + (Y + 0.5f) * CellSize, 0.0f);

		CityCells[i].X = X;
		CityCells[i].Y = Y;

		SetWorldPosition(X, Y, CellPos);

		
		const FVector2D Cell2DPos = FVector2D(CellPos.X, CellPos.Y);
		
		const FVector2D CityCenter2D = FVector2D(CityCenter.X, CityCenter.Y);
		float DistanceFromCityCenter = FVector2D::Distance(CityCenter2D, Cell2DPos);

		// 도시 구역을 벗어나는 Cell → Block
		if (DistanceFromCityCenter >= CityRadius)
		{
			CityCells[i].Type = ECellType::Blocked;
			continue;
		}

		// RoadEdge ↔ Cell 거리 측정
		for (int j = 0; j < RoadGraph.GetEdges().Num(); j++)
		
		{
			const FRoadEdge* RoadEdge = RoadGraph.GetEdge(j);

			for (const FVector& SegmentPos : RoadEdge->SegmentPoints)
			{
				const FVector2D Segment2DPos = FVector2D(SegmentPos.X, SegmentPos.Y);

				if (FVector2D::Distance(Segment2DPos, Cell2DPos ) <= RoadEdge->HalfWidth)
				{
					CityCells[i].Type = ECellType::Road;
					break;
				}
				else continue;
			}
		}
	}
}

void FCityGrid::SetWorldPosition(int32 X, int32 Y, FVector InPosition)
{
	if(IsValid(X, Y))
		CityCells[X + Y * Width].WorldPosition = InPosition;
}

FVector FCityGrid::GetWorldPosition(int32 X, int32 Y)
{
	return Origin + FVector(X * CellSize, Y * CellSize, 0.0f);
}
