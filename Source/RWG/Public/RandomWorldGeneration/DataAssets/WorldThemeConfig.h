// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "WorldThemeConfig.generated.h"

USTRUCT(BlueprintType)
struct FBuildingAssetEntry
{
	GENERATED_BODY()

public:
	/* 건물 에셋 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	int32 FootprintSizeX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	int32 FootprintSizeY;
};

/**
 * 
 */
UCLASS()
class RWG_API UWorldThemeConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Grid")
	float CellSize = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Road")
	int32 MinRoadNum = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Road")
	int32 MaxRoadNum = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Road")
	float RoadWidth = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme Settings")
	EWorldTheme WorldTheme;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<FBuildingAssetEntry> BuildingEntries;

	/* 월드에 배치될 도로나 바닥 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<UStaticMesh*> Roads;

protected:
	FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangeEvent) override;
#endif

	void RefreshBuildingFootprints();
	void RefreshSingleBuildingFootprint(FBuildingAssetEntry& Entry) const;
};
