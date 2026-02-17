// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldGenSubsystem.generated.h"

RTG_API DECLARE_LOG_CATEGORY_EXTERN(LogWorldGenSubsystem, Log, All);

/**
 * World Generation Management Subsystem.
 * 
 */
UCLASS()
class RTG_API UWorldGenSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void GenerateWorld(int32 Seed);

protected:
	void InitializeWorldConfig();

	UFUNCTION()
	void OnConfigInitialized();

	virtual void OnWorldBeginPlay(UWorld& World) override;

	class UWorldGenConfig* WorldGenConfig;

private:
	UPROPERTY()
	class AWorldGenerator* WorldGenerator;
};
