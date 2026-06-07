// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpawnDirectorComponent.generated.h"

class UItemData;
class ABuildingActor;
class UBoxComponent;
class AItem;

USTRUCT()
struct FItemSpawnEntry
{
	GENERATED_BODY()

	FVector Location = FVector::ZeroVector;
    TSubclassOf<AItem> ItemClass = nullptr;
    bool bSpawned = false;

    TWeakObjectPtr<ABuildingActor> OwnerBuilding;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API USpawnDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    USpawnDirectorComponent();

    void InitializeSpawnData();

    // 에디터에서 등록
    // TODO: Asset Manager 기반 자동 수집으로 전환
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSubclassOf<AItem>> ItemClasses;

private:
    UPROPERTY(VisibleAnywhere, Category = "Spawn")
    TArray<FItemSpawnEntry> ItemSpawnEntries;

    void CollectItemSpawnPoints();
    void BindBuildingDelegates(ABuildingActor* Building);
    FVector GetRandomPointInBox(UBoxComponent* Box);

    TSubclassOf<AItem> SelectItemClass();

    void OnBuildingEntered(ABuildingActor* Building);
    void OnBuildingExited(ABuildingActor* Building);
};
