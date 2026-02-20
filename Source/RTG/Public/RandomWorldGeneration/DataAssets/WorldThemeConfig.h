// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "WorldThemeConfig.generated.h"

USTRUCT(BlueprintType)
struct FWorldStructureData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere)
	FVector Scale = FVector(1.0f);
};

/**
 * 
 */
UCLASS()
class RTG_API UWorldThemeConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme Settings")
	EWorldTheme WorldTheme;

	/* 월드 구성에 쓰일 건물들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<FWorldStructureData> Buildings;

	/* 폐차, 쓰레기 더미 등 월드 구성에 쓰일 소품들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<FWorldStructureData> Props;

	/* 탈출을 위한 주요 건물 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	FWorldStructureData MainBuilding;

	/* 월드 구성에 쓰일 벽들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<FWorldStructureData> Walls;
	
	/* 월드에 배치될 도로나 바닥 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<FWorldStructureData> Roads;

protected:
	FPrimaryAssetId GetPrimaryAssetId() const override;
};
