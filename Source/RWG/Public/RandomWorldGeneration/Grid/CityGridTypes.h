// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RandomWorldGeneration/PCG/RoadGraph.h"

enum class ECellType : uint8
{
	Empty,
	Road,
	Blocked
};

enum class ERoadClass : uint8
{
	None,
	Primary,
	Secondary,
	Tertiary
};

// NESW 4방향 연결 비트
enum class ERoadConnection : uint8
{
	Conn_N = 1 << 0,
	Conn_E = 1 << 1,
	Conn_S = 1 << 2,
	Conn_W = 1 << 3,
};

struct FCityCell
{
	// Static
	int32 X = 0;
	int32 Y = 0;
	FVector WorldPosition = FVector::ZeroVector;
	float DistToCenter = 0.0f;

	// Dynamic
	ECellType Type = ECellType::Empty;
	ERoadClass RoadClass = ERoadClass::None;

	// Neighborhood
	uint8 ConnectionMask = 0; // ERoadConnection Bits

	// Result
	int32 BlockId = -1;
};

/// <summary>
/// 블록 구조체.
/// 그리드 위에 그려진 셀 중, 건물을 배치할 수 있는 셀들로 블록화한 것을 저장하는 구조체
/// </summary>
struct FCityBlock
{
	int32 BlockId = -1;
	TArray<int32> CellIndices;
	int32 CellCount = 0;
};

/// <summary>
/// 필지 구조체. 
/// 여러 셀을 묶어 건물을 배치할 공간을 저장하는 구조체
/// </summary>
struct FCityLot
{
	int32 LotId = -1;
	int32 BlockId = -1;

	TArray<int32> CellIndices;

	int32 MinX = 0;
	int32 MaxX = 0;
	int32 MinY = 0;
	int32 MaxY = 0;

	FVector Center = FVector::ZeroVector;

	bool bTouchesRoad = false;	// 도로와 맞닿아 있는지
	FVector FacingDirection = FVector::ForwardVector;

	FORCEINLINE int32 GetWidthInCells() const
	{
		return MaxX - MinX + 1;
	}

	FORCEINLINE int32 GetHeightInCells() const
	{
		return MaxY - MinY + 1;
	}

	FORCEINLINE int32 GetAreaInCells() const
	{
		return CellIndices.Num();
	}
};

class RWG_API FCityGrid
{
public:
	const FVector GetOrigin() const;
	void SetOirin(const FVector InOrigin);

	const int32 GetWidth() const;
	void SetWidth(const int32 InWidth);

	const int32 GetHeight() const;
	void SetHeight(const int32 InHeight);

	const float GetCellSize() const { return CellSize; }
	void SetCellSize(const float InCellSize);

	const TArray<FCityCell> GetCityCells() const;
	TArray<FCityCell>& GetCityCellsMutable();

	FORCEINLINE int32 Index(int32 X, int32 Y) const
	{
		return X + Y * Width;
	}

	FORCEINLINE bool IsValid(int32 X, int32 Y) const
	{
		return X >= 0 && X < Width && Y >= 0 && Y < Height;
	}

	void GenerateGrid(FVector CityCenter, float CityRadius, float InCellSize, const FRoadGraph& RoadGraph);
	
	void SetWorldPosition(int32 X, int32 Y, FVector InPosition);

	FVector GetWorldPosition(int32 X, int32 Y);

	const TArray<FCityLot> GetLots() const;
	void SetLots(const TArray<FCityLot>& InLots);

private:
	TArray<FCityCell> CityCells;

	// 월드 내 가로 / 세로 셀 개수
	int32 Width, Height;

	// 셀의 크기
	float CellSize = 200.0f;

	FVector Origin = FVector::ZeroVector;

	TArray<FCityLot> Lots;
};