// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "WorldThemeConfig.generated.h"

class ABuildingActor;

USTRUCT(BlueprintType)
struct FBuildingAssetEntry
{
	GENERATED_BODY()

public:
	/* 건물 에셋 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSubclassOf<ABuildingActor> ActorClass;

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
	
	/* 차수 2 노드를 빼고 양쪽 엣지를 합치는 최대 꺾임 각도(도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Road")
	float MaxPassThroughBendAngle = 18.2f;

	/* 차수 2 노드의 모서리를 깎는 원호의 기본 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Road")
	float CurveRadius = 800.0f;

	/* 원호 엣지 위 점 사이 간격(원호 길이) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Road")
	float CurveSegmentSpacing = 100.0f;

	/* 교차로에서 이웃한 두 갈래의 경계선 사이를 잇는 원호의 최대 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters|Road")
	float IntersectionCornerRadius = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme Settings")
	EWorldTheme WorldTheme;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<FBuildingAssetEntry> BuildingEntries;

	/* 등록된 건물 바닥 크기로 정한 필지 크기 범위 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Structures")
	FLotSizeRange LotSizeRange;

	/* 월드에 배치될 도로나 바닥 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
	TArray<UStaticMesh*> Roads;

	/* 지형 메시에 입힐 머티리얼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	TObjectPtr<UMaterialInterface> TerrainMaterial;

protected:
	FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangeEvent) override;
#endif

	void RefreshBuildingFootprints();
	void RefreshSingleBuildingFootprint(FBuildingAssetEntry& Entry) const;
	void RefreshLotSizeRange();
};
