// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Actors/BuildingActor.h"
#include "Components/BoxComponent.h"

ABuildingActor::ABuildingActor()
{
    bReplicates = true;

    ExteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExteriorMesh"));
    
    RootComponent = ExteriorMesh;

    ProximityTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ProximityTrigger"));
    ProximityTrigger->SetupAttachment(RootComponent);
}

void ABuildingActor::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    GetComponents<UBoxComponent>(ItemSpawnVolumes);
    ItemSpawnVolumes.Remove(ProximityTrigger);
}

// Called when the game starts or when spawned
void ABuildingActor::BeginPlay()
{
	Super::BeginPlay();

    if (!HasAuthority()) return; // 서버만 바인딩

    ProximityTrigger->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProximityBeginOverlap);
    ProximityTrigger->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnProximityEndOverlap);
}

void ABuildingActor::OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn || !Pawn->IsPlayerControlled()) return;

    PlayersInside++;
    if (PlayersInside == 1)
        OnPlayerEntered.Broadcast();
}

void ABuildingActor::OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn || !Pawn->IsPlayerControlled()) return;

    PlayersInside = FMath::Max(0, PlayersInside - 1);
    if (PlayersInside == 0)
        OnPlayerExited.Broadcast();
}