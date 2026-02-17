// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldThemeConfig.generated.h"

enum class EWorldTheme : uint8;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshs")
	TArray<UStaticMesh*> Buildings;

	/* 폐차, 쓰레기 더미 등 월드 구성에 쓰일 소품들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshs")
	TArray<UStaticMesh*> Props;

	/* 탈출을 위한 주요 건물 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshs")
	UStaticMesh* MainBuilding;

	/* 월드 구성에 쓰일 벽들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshs")
	TArray<UStaticMesh*> Walls;

};
