// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldGenSubsystem.generated.h"

RWG_API DECLARE_LOG_CATEGORY_EXTERN(LogWorldGenSubsystem, Log, All);

/**
 * World Generation Management Subsystem.
 * 
 */
UCLASS()
class RWG_API UWorldGenSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void InitiateWorldGeneration();

protected:
	void InitializeWorldConfig();

	void LoadConfigByType(FPrimaryAssetType AssetType);

	void OnConfigInitialized(FPrimaryAssetType AssetType);

	virtual void OnWorldBeginPlay(UWorld& World) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Configs")
	TMap<FPrimaryAssetType, TObjectPtr<UObject>> LoadedConfigs;

	int32 ExpectedCount = 2;

private:
	UPROPERTY()
	class AWorldGenerator* WorldGenerator;
};
