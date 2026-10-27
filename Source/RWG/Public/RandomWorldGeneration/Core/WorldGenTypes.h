// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.generated.h"

UENUM(BlueprintType)
enum class EWorldTheme : uint8
{
	None			UMETA(DisplayName = "None"),
	IsolatedCity	UMETA(DisplayName = "Isolated City"),	// 고립된 도시
	RuralVillage	UMETA(DisplayName = "Rural Village"),	// 촌락
	Joseon			UMETA(DisplayName = "Joseon"),			// 조선
};

USTRUCT(BlueprintType)
struct FLotSizeRange
{
	GENERATED_BODY()

public:
	/* 필지 가로 최소 크기(셀) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lot")
	int32 MinX = 1;

	/* 필지 세로 최소 크기(셀) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lot")
	int32 MinY = 1;

	/* 필지 가로 최대 크기(셀) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lot")
	int32 MaxX = 1;

	/* 필지 세로 최대 크기(셀) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lot")
	int32 MaxY = 1;
};

namespace WorldConfigTags
{
	const FName GenConfigName = FName("WorldGenConfig");
	const FName ThemeConfigName = FName("WorldThemeConfig");
}

namespace WorldThemeStructureTags
{
	const FName MainBuildingName = FName("MainBuilding");
	const FName BuildingsName = FName("Buildings");
	const FName PropsName = FName("Props");
	const FName WallsName = FName("Walls");
	const FName RoadsName = FName("Roads");
}
