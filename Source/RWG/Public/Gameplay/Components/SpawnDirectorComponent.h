// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/DataAssets/ItemData.h"
#include "SpawnDirectorComponent.generated.h"

enum class EDayCycle : uint8;

class UItemData;
class ABuildingActor;
class UBoxComponent;
class AItem;
class AZombie;
class AConvict;

USTRUCT()
struct FItemSpawnEntry
{
	GENERATED_BODY()

	FVector Location = FVector::ZeroVector;
	TObjectPtr<UItemData> ItemData = nullptr;
	bool bSpawned = false;

	TWeakObjectPtr<ABuildingActor> OwnerBuilding;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API USpawnDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	USpawnDirectorComponent();

protected:
	void BeginPlay() override;

/*********************************************************************
*                            아이템 스폰
*********************************************************************/
private:
	void CollectItemSpawnPoints();

	void BindBuildingDelegates(ABuildingActor* Building);

	FVector GetRandomPointInBox(UBoxComponent* Box);

	void CollectSpawnableItems(TArray<UItemData*>& OutItems) const;

	UItemData* SelectItemData(const TArray<UItemData*>& SpawnableItems) const;

	void OnBuildingEntered(ABuildingActor* Building);

	void OnBuildingExited(ABuildingActor* Building);

	UPROPERTY(VisibleAnywhere, Category = "Spawn|Item")
	TArray<FItemSpawnEntry> ItemSpawnEntries;

public:
	void InitializeSpawnData();

	// 아이템 타입별로 스폰할 액터 클래스
	UPROPERTY(EditAnywhere, Category = "Spawn|Item")
	TMap<EItemType, TSubclassOf<AItem>> ItemClassByType;

/*********************************************************************
*                             좀비 스폰
*********************************************************************/
protected:
	void StartZombieWave();

	void StopZombieWave();

	bool FindSpawnLocation(const FVector PlayerLocation, FVector& OutLocation);

	void OnZombieDeath(AZombie* Zombie);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn|Zombie")
	TSubclassOf<AZombie> ZombieClass;

	TMap<TObjectPtr<AConvict>, TArray<TObjectPtr<AZombie>>> ZombieAssignmentMap;

	// 플레이어로부터 좀비가 생성될 거리 (원형)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn|Zombie")
	float SpawnZombieDistance = 400.0f;

	// 플레이어 당 최대 생성 가능한 좀비 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn|Zombie")
	int MaxZombiePerPlayer = 8;

	// 임시용. 거리 기준 좀비 스폰 시 무한루프 방지를 위한 재시도 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn|Zombie")
	int SpawnRetryCount = 10;

public:
	void OnDayCycleChanged(EDayCycle DayCycle);

	void RegisterPlayer(APawn* PlayerPawn);

	void UnregisterPlayer(APawn* PlayerPawn);
};
