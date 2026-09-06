// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldGenSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnWorldReady);

class UGlobalUISubsystem;

/**
 * World Generation Management Subsystem.
 * 
 */
UCLASS()
class RWG_API UWorldGenSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void OnWorldBeginPlay(UWorld& World) override;

	virtual void Deinitialize() override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
private:
	UPROPERTY()
	class AWorldGenerator* WorldGenerator;

/*********************************************************************
*                           월드 설정 로딩
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Configs")
	TMap<FPrimaryAssetType, TObjectPtr<UObject>> LoadedConfigs;

	int32 ExpectedCount = 2;

	void InitializeWorldConfig();

	void LoadConfigByType(FPrimaryAssetType AssetType);

	void OnConfigInitialized(FPrimaryAssetType AssetType);

/*********************************************************************
*                             월드 생성
*********************************************************************/
private:
	bool bWorldReady = false;

protected:
	void OnWorldGenerationCompleted();

public:
	FOnWorldReady OnWorldReady;

	UFUNCTION()
	void InitiateWorldGeneration();

	bool IsWorldReady() const
	{
		return bWorldReady;
	}

/*********************************************************************
*                              UI 통지
*********************************************************************/
protected:
	UGlobalUISubsystem* GetGlobalUISubsystem() const;
};
