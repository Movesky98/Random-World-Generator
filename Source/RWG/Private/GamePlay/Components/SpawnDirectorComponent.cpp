// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/SpawnDirectorComponent.h"
#include "GamePlay/Items/Item.h"
#include "GamePlay/Items/ItemData.h"
#include "RandomWorldGeneration/Actors/BuildingActor.h"
#include "CommonLogCategories.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

USpawnDirectorComponent::USpawnDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USpawnDirectorComponent::InitializeSpawnData()
{
    if (!GetOwner()->HasAuthority()) return;

    CollectItemSpawnPoints();
}

void USpawnDirectorComponent::CollectItemSpawnPoints()
{
    TArray<AActor*> Buildings;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingActor::StaticClass(), Buildings);

    for (AActor* Actor : Buildings)
    {
        ABuildingActor* Building = Cast<ABuildingActor>(Actor);
        if (!Building) continue;

        for (UBoxComponent* Box : Building->ItemSpawnVolumes)
        {
            if (!Box) continue;

            FItemSpawnEntry Entry;
            Entry.Location = GetRandomPointInBox(Box);
            Entry.ItemClass = SelectItemClass();
            Entry.bSpawned = false;
            Entry.OwnerBuilding = Building;

            ItemSpawnEntries.Add(Entry);
        }

        BindBuildingDelegates(Building);
    }

    COMMON_LOG(LogTemp, Warning, TEXT("Buildings found: %d"), Buildings.Num());
}

void USpawnDirectorComponent::BindBuildingDelegates(ABuildingActor* Building)
{
    Building->OnPlayerEntered.AddLambda([this, Building]()
        {
            OnBuildingEntered(Building);
        });

    Building->OnPlayerExited.AddLambda([this, Building]()
        {
            OnBuildingExited(Building);
        });
}

TSubclassOf<AItem> USpawnDirectorComponent::SelectItemClass()
{
    if (ItemClasses.IsEmpty()) return nullptr;

    int32 Index = FMath::RandRange(0, ItemClasses.Num() - 1);
    return ItemClasses[Index];
}


void USpawnDirectorComponent::OnBuildingEntered(ABuildingActor* Building)
{
    for (FItemSpawnEntry& Entry : ItemSpawnEntries)
    {
        if (Entry.OwnerBuilding != Building) continue;
        if (Entry.bSpawned) continue;
        if (!Entry.ItemClass) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AItem* Item = GetWorld()->SpawnActor<AItem>(
            Entry.ItemClass,
            Entry.Location,
            FRotator::ZeroRotator,
            Params
        );

        if (Item)
            Entry.bSpawned = true;
    }
}

void USpawnDirectorComponent::OnBuildingExited(ABuildingActor* Building)
{
    // 스폰 후 유지 — 아무것도 안 함
}

FVector USpawnDirectorComponent::GetRandomPointInBox(UBoxComponent* Box)
{
    FVector Origin = Box->GetComponentLocation();
    FVector Extent = Box->GetScaledBoxExtent();

    FVector RandomPoint = FVector(
        FMath::RandRange(Origin.X - Extent.X, Origin.X + Extent.X),
        FMath::RandRange(Origin.Y - Extent.Y, Origin.Y + Extent.Y),
        Origin.Z + Extent.Z  // 박스 상단에서 쏴야 안전
    );

    FHitResult Hit;
    FVector TraceEnd = FVector(RandomPoint.X, RandomPoint.Y, Origin.Z - Extent.Z - 100.f);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        RandomPoint,
        TraceEnd,
        ECC_Visibility
    );

    return bHit ? Hit.ImpactPoint : RandomPoint;
}