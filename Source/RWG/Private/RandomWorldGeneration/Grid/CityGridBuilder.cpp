// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Grid/CityGridBuilder.h"

#include "Containers/Queue.h"

TArray<bool> FCityGridBuilder::BaseBuildableCell;

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

	for (int32 i = 0; i < Cells.Num(); i++)
	{
		int32 X = i % NewGrid.GetWidth();
		int32 Y = i / NewGrid.GetWidth();
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

	for (int32 i = 0; i < Cells.Num(); i++)
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

	for (int32 i = 0; i < Cells.Num(); i++)
	{
		FVector2D Cell2DPos = FVector2D(Cells[i].WorldPosition.X, Cells[i].WorldPosition.Y);
		FVector2D CityCenter2D = FVector2D(InParams.CityCenter.X, InParams.CityCenter.Y);

		if (FVector2D::Distance(Cell2DPos, CityCenter2D) > InParams.CityRadius)
		{
			Cells[i].Type = ECellType::Blocked;
		}
	}
}

TArray<FCityBlock> FCityGridBuilder::BuildBlocks(FCityGrid& CityGrid)
{
	int32 CurrentBlockId = 0;
	TArray<FCityCell> Cells = CityGrid.GetCityCellsMutable();
	TArray<FCityBlock> Blocks;

	for (int32 Y = 0; Y < CityGrid.GetHeight(); Y++)
	{
		for (int32 X = 0; X < CityGrid.GetWidth(); X++)
		{
			if (Cells[CityGrid.Index(X, Y)].BlockId != -1 || Cells[CityGrid.Index(X, Y)].Type != ECellType::Empty)
				continue;

			FCityBlock Block = BlockFloodFill(CityGrid, X, Y, CurrentBlockId);
			CurrentBlockId++;

			Blocks.Add(Block);
		}
	}

	return Blocks;
}

FCityBlock FCityGridBuilder::BlockFloodFill(FCityGrid& Grid, const int32 StartX, const int32 StartY, const int32 InBlockId)
{
	TQueue<FIntPoint> CellPosQueue;
	TArray<FCityCell>& Cells = Grid.GetCityCellsMutable();
	
	FCityBlock Block;
	Block.BlockId = InBlockId;

	CellPosQueue.Enqueue(FIntPoint(StartX, StartY));

	while (!CellPosQueue.IsEmpty())
	{
		FIntPoint CurrentPos;
		CellPosQueue.Dequeue(CurrentPos);

		int32 X = CurrentPos.X;
		int32 Y = CurrentPos.Y;

		if (!Grid.IsValid(X, Y)) continue;

		FCityCell& Cell = Cells[Grid.Index(X, Y)];

		if (Cell.Type != ECellType::Empty || Cell.BlockId != -1) continue;

		Cell.BlockId = InBlockId;
		Block.CellIndices.Add(Grid.Index(X, Y));
		Block.CellCount++;

		CellPosQueue.Enqueue(FIntPoint(X + 1, Y));
		CellPosQueue.Enqueue(FIntPoint(X - 1, Y));
		CellPosQueue.Enqueue(FIntPoint(X, Y + 1));
		CellPosQueue.Enqueue(FIntPoint(X, Y - 1));
	}

	return Block;
}

void FCityGridBuilder::ExtractLotsFromBlock(FCityGrid& Grid, const TArray<FCityBlock>& Blocks)
{
	BuildBaseBuildableMask(Grid);

	TArray<bool> Used;
	Used.Init(false, Grid.GetWidth() * Grid.GetHeight());
	TArray<FCityLot> Lots;

	// 블록 단위로 필지(Lot) 추출
	for (const FCityBlock& Block : Blocks)
	{
		for (int32 Index : Block.CellIndices)
		{
			FCityLot Lot;

			int32 X = Index % Grid.GetWidth();
			int32 Y = Index / Grid.GetWidth();

			int32 Width = 0;
			int32 Height = 0;

			if (FindBestRectangle(Grid, Used, X, Y, Block.BlockId, Width, Height))
			{	
				Lot = MarkLotFromRectangle(Grid, Used, Block.BlockId, Lots.Num(), X, Y, Width, Height);
				Lots.Add(Lot);
			}
		}
	}

	Grid.SetLots(Lots);
}

void FCityGridBuilder::BuildBaseBuildableMask(FCityGrid& Grid)
{
	BaseBuildableCell.Init(true, Grid.GetWidth() * Grid.GetHeight());

	for (const FCityCell& Cell : Grid.GetCityCells())
	{
		// CellType == Road or CellType == Blocked
		if (Cell.Type != ECellType::Empty)
		{
			BaseBuildableCell[Grid.Index(Cell.X, Cell.Y)] = false;
		}

		else
		{
			for (int i = -1; i <= 1; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					if (Grid.IsValid(Cell.X + i, Cell.Y + j))
					{
						if (Grid.GetCityCells()[Grid.Index(Cell.X + i, Cell.Y + j)].Type == ECellType::Road)
							BaseBuildableCell[Grid.Index(Cell.X, Cell.Y)] = false;
					}
				}
			}
		}
	}
}

bool FCityGridBuilder::IsBuildableCell(const FCityGrid& Grid, int32 X, int32 Y, int32 BlockId, const TArray<bool>& Used)
{
	if (!Grid.IsValid(X, Y)) 
		return false;

	const int32 Index = Grid.Index(X, Y);
	const FCityCell& Cell = Grid.GetCityCells()[Index];

	if (Used[Index] || Cell.BlockId != BlockId || !BaseBuildableCell[Index])
		return false;

	return Cell.Type == ECellType::Empty;
}

bool FCityGridBuilder::IsRectangleValid(const FCityGrid& Grid, int32 StartX, int32 StartY, int32 Width, int32 Height, int32 BlockId, const TArray<bool>& Used)
{
	int32 Y = StartY + Height - 1;

	for (int32 X = StartX; X < StartX + Width; ++X)
	{
		if (!IsBuildableCell(Grid, X, Y, BlockId, Used))
		{
			return false;
		}
	}

	return true;
}

bool FCityGridBuilder::FindBestRectangle(const FCityGrid& Grid, const TArray<bool>& Used, int32 StartX, int32 StartY, int32 BlockId, int32& OutWidth, int32& OutHeight)
{
	OutWidth = 0;
	OutHeight = 0;

	int32 MaxWidth = 0;

	// 직사각형이 될 수 있는 최대 가로 길이 계산
	while (IsBuildableCell(Grid, StartX + MaxWidth, StartY, BlockId, Used))
	{
		MaxWidth++;
	}

	int32 BestScore = -1;

	for (int32 Width = 2; Width <= MaxWidth; ++Width)
	{
		int32 Height = 0;

		// 직사각형이 될 수 있는 최대 세로 길이 계산
		while (IsRectangleValid(Grid, StartX, StartY, Width, Height + 1, BlockId, Used))
		{
			++Height;
		}

		if (Height < 2) continue;

		const int32 Score = Width * Height;

		if (Score > BestScore)
		{
			BestScore = Score;
			OutWidth = Width;
			OutHeight = Height;
		}
	}

	return BestScore > 0;
}

FCityLot FCityGridBuilder::MarkLotFromRectangle(const FCityGrid& Grid, TArray<bool>& Used, int32 BlockId, int32 LotId, int32 StartX, int32 StartY, int32 Width, int32 Height)
{
	FCityLot Lot;

	for (int32 Y = StartY - 1; Y <= StartY + Height; ++Y)
	{
		for (int32 X = StartX - 1; X <= StartX + Width; ++X)
		{
			if (Grid.IsValid(X, Y))
			{
				// 직사각형 필지와 외 테두리 점유화
				Used[Grid.Index(X, Y)] = true;

				// 직사각형 필지 → Lot 저장
				if(StartX <= X && X < StartX + Width && StartY <= Y && Y < StartY + Height)
					Lot.CellIndices.Add(Grid.Index(X, Y));
			}
		}
	}

	Lot.MinX = StartX;
	Lot.MaxX = StartX + Width - 1;
	Lot.MinY = StartY;
	Lot.MaxY = StartY + Height - 1;
	Lot.BlockId = BlockId;
	Lot.LotId = LotId;
	
	return Lot;
}
