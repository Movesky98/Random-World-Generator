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

	void GenerateWorld(int32 Seed, TMap<FPrimaryAssetType, TObjectPtr<UObject>> Configs);

	void GenerateTerrain(class UWorldGenConfig* Config);

	void InitializePCGConfigs(const FVector& CityCenter, const FVector& CityRadius);

	void GenerateContent(class UWorldThemeConfig* Config);

	const FRoadGraph& GetRoadGraph() const { return RoadGraph; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	template <typename TObject>
	void SetPCGObjectParameter(FName PropertyName, TObject* Value);

	template <typename TObject>
	void SetPCGObjectParameters(FName PropertyName, const TArray<TObject*>& Values);

	void DrawDebugGrid();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	FVector MainCityCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	float MainCityRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	float CityHeight;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "PMC")
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "PCG")
	UPCGComponent* PCGComponent;

	FRoadGraph RoadGraph;

	FCityGrid CityGrid;

	TArray<FCityBlock> CityBlocks;
};

