// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Actors/BuildingActor.h"
#include "Components/BoxComponent.h"

ABuildingActor::ABuildingActor()
{
    bReplicates = true;

    ExteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExteriorMesh"));
    
    RootComponent = ExteriorMesh;
}

void ABuildingActor::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    GetComponents<UBoxComponent>(ItemSpawnVolumes);
}

// Called when the game starts or when spawned
void ABuildingActor::BeginPlay()
{
	Super::BeginPlay();
	
}
