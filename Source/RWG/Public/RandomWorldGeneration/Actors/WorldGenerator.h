// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ProceduralMeshComponent.h"
#include "PCGComponent.h"
#include "RandomWorldGeneration/PCG/RoadGraph.h"
#include "RandomWorldGeneration/Grid/CityGridTypes.h"
#include "WorldGenerator.generated.h"

RWG_API DECLARE_LOG_CATEGORY_EXTERN(LogWorldGenerator, Log, All);

/*
* World Generator Actor.
* 
*/
UCLASS()
class RWG_API AWorldGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldGenerator();

	void GenerateWorld(TMap<FPrimaryAssetType, TObjectPtr<UObject>> Configs);

	void GenerateTerrain(class UWorldGenConfig* Config);

	void GenerateContent(class UWorldThemeConfig* Config);

	const FRoadGraph& GetRoadGraph() const { return RoadGraph; }

	const FCityGrid& GetCityGrid() const { return CityGrid; }

	int32 GetSeed() const { return MasterSeed; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	FVector CityCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	float CityRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	float CityHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parameters")
	int32 MasterSeed = 0;
	
	void DrawDebugGrid();

	void DebugSeedResult();

private:
	UPROPERTY(VisibleAnywhere, Category = "PMC")
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "PCG")
	UPCGComponent* RoadPCGComponent;

	UPROPERTY(VisibleAnywhere, Category = "PCG")
	UPCGComponent* BuildingPCGComponent;

	FRoadGraph RoadGraph;

	FCityGrid CityGrid;

	TArray<FCityBlock> CityBlocks;
};

