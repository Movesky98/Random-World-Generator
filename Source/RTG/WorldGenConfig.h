// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldGenConfig.generated.h"

/**
 * 
 */
UCLASS()
class RTG_API UWorldGenConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<class AWorldGenerator> WorldGeneratorClass;

	// 가로 쿼드 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Parameters")
	int32 XSize = 1000;

	// 세로 쿼드 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Parameters")
	int32 YSize = 1000;

	// 한 격자의 크기 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Parameters")
	float GridSpacing = 100.0f;

	/* 도시(섬)의 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
	float CityRadius = 300.0f;

	/* 도시(섬)의 고도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
	float CityHeight = 8000.0f;

	/* 절벽 너비 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
	float CliffWidth = 50.0f;

	/* 일반 지형의 기본 고도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
	float OuterHeight = 100.0f;

protected:
	// Primary Asset Id 에 'TerrainGenData' 태그를 추가함.
	// Asset Manager를 활용해 태그를 기반으로 Data Asset을 찾을 수 있도록 함.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
};
