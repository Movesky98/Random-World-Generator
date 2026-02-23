// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ProceduralMeshComponent.h"
#include "PCGComponent.h"

#include "WorldGenerator.generated.h"

RTG_API DECLARE_LOG_CATEGORY_EXTERN(LogWorldGenerator, Log, All);

/*
* World Generator Actor.
* 
*/
UCLASS()
class RTG_API AWorldGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldGenerator();

	void GenerateWorld(int32 Seed, TMap<FPrimaryAssetType, TObjectPtr<UObject>> Configs);

	void GenerateTerrain(class UWorldGenConfig* Config);

	void InitializePCGConfigs(const FVector& CityCenter, const FVector& CityRadius);

	void GenerateContent(class UWorldThemeConfig* Config);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	FVector MainCityCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	float MainCityRadius;
	
private:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "PCG")
	UPCGComponent* PCGComponent;
};
