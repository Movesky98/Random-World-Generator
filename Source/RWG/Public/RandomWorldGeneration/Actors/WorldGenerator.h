// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ProceduralMeshComponent.h"
#include "PCGComponent.h"
#include "RandomWorldGeneration/PCG/RoadGraph.h"
#include "RandomWorldGeneration/Grid/CityGridTypes.h"
#include "WorldGenerator.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnWorldGenerationCompleted);

/*
* World Generator Actor.
* 
*/
UCLASS()
class RWG_API AWorldGenerator : public AActor
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this actor's properties
	AWorldGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
private:
	UPROPERTY(VisibleAnywhere, Category = "PMC")
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "PMC")
	UProceduralMeshComponent* TerrainNavProxyPMC;

	UPROPERTY(VisibleAnywhere, Category = "PCG")
	UPCGComponent* RoadPCGComponent;

	UPROPERTY(VisibleAnywhere, Category = "PCG")
	UPCGComponent* BuildingPCGComponent;

/*********************************************************************
*                             생성 진입
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parameters")
	int32 MasterSeed = 0;

public:
	void GenerateWorld(TMap<FPrimaryAssetType, TObjectPtr<UObject>> Configs);

	int32 GetSeed() const { return MasterSeed; }

/*********************************************************************
*                             지형 생성
*********************************************************************/
private:
	UFUNCTION()
	void OnNavMeshBuilt(ANavigationData* NavData);

	UFUNCTION()
	void OnComponentPhysicsStateChanged(UPrimitiveComponent* ChangedComponent, EComponentPhysicsStateChange StateChange);

protected:
	void GenerateNavProxyMesh(class UWorldGenConfig* Config);

public:
	void GenerateTerrain(class UWorldGenConfig* Config);

/*********************************************************************
*                          도시 콘텐츠 생성
*********************************************************************/
private:
	UPROPERTY()
	TObjectPtr<class UWorldThemeConfig> ThemeConfig = nullptr;

	FRoadGraph RoadGraph;

	FCityGrid CityGrid;

	TArray<FCityBlock> CityBlocks;

	void StartGeneratePCG(UWorldThemeConfig* Config);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	FVector CityCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	float CityRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PCG Settings")
	float CityHeight;

	void OnPCGGraphGenerated(UPCGComponent* InComponent);

public:
	void GenerateContent(class UWorldThemeConfig* Config);

	const FRoadGraph& GetRoadGraph() const { return RoadGraph; }

	const FCityGrid& GetCityGrid() const { return CityGrid; }

/*********************************************************************
*                             완료 판정
*********************************************************************/
private:
	bool bRoadPCGCompleted = false;
	bool bBuildingPCGCompleted = false;
	bool bNavBuildCompleted = false;

	void CheckAllComplete();

public:
	FOnWorldGenerationCompleted OnWorldGenerationCompleted;

/*********************************************************************
*                               디버그
*********************************************************************/
private:
	double T0 = 0.0;
	double T1 = 0.0;
	double T2 = 0.0;
	double T3 = 0.0;

	double TA = 0.0;
	double TB = 0.0;
	double TC = 0.0;

protected:
	void DrawDebugGrid();

	void DebugSeedResult();
};
