// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Actors/BuildingActor.h"

ABuildingActor::ABuildingActor()
{
    bReplicates = true;

    ExteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExteriorMesh"));
    RootComponent = ExteriorMesh;
}

// Called when the game starts or when spawned
void ABuildingActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABuildingActor::PostActorCreated()
{
    Super::PostActorCreated();

    if (BuildingMesh)
    {
        ExteriorMesh->SetStaticMesh(BuildingMesh);
    }
}