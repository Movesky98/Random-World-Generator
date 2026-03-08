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



private:
	TArray<FCityCell> CityCells;

	// 월드 내 가로 / 세로 셀 개수
	int32 Width, Height;

	// 셀의 크기
	float CellSize = 200.0f;

	FVector Origin = FVector::ZeroVector;
};