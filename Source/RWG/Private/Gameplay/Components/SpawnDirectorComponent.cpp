// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/SpawnDirectorComponent.h"
#include "Gameplay/Items/Item.h"
#include "Gameplay/DataAssets/ItemData.h"
#include "Gameplay/Components/TimeManagementComponent.h"
#include "Gameplay/Components/HealthComponent.h"
#include "Gameplay/Characters/Convict/Convict.h"
#include "Gameplay/Characters/Zombie/Zombie.h"
#include "Common/GameFramework/GameAssetManager.h"

#include "RandomWorldGeneration/Actors/BuildingActor.h"
#include "CommonLogCategories.h"


#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

USpawnDirectorComponent::USpawnDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USpawnDirectorComponent::InitializeSpawnData()
{
    if (!GetOwner()->HasAuthority()) return;

    COMMON_LOG(LogGameplay, Log, TEXT("Generating world items..."));

    CollectItemSpawnPoints();
}

void USpawnDirectorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UTimeManagementComponent* TMC = GetOwner()->FindComponentByClass<UTimeManagementComponent>())
    {
        TMC->OnDayCycleChanged.AddUObject(this, &ThisClass::OnDayCycleChanged);
    }
}

void USpawnDirectorComponent::CollectItemSpawnPoints()
{
    TArray<UItemData*> SpawnableItems;
    CollectSpawnableItems(SpawnableItems);

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
            Entry.ItemData = SelectItemData(SpawnableItems);
            Entry.bSpawned = false;
            Entry.OwnerBuilding = Building;

            ItemSpawnEntries.Add(Entry);
        }

        BindBuildingDelegates(Building);
    }

    COMMON_LOG(LogGameplay, Log, TEXT("World items generated."));
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

void USpawnDirectorComponent::CollectSpawnableItems(TArray<UItemData*>& OutItems) const
{
    TArray<UObject*> LoadedAssets;
    UGameAssetManager::Get().GetLoadedAssets(FPrimaryAssetType(TEXT("Item")), LoadedAssets);

    // 타입에 대응하는 액터 클래스가 있는 데이터만 스폰 후보로 둔다
    for (UObject* Asset : LoadedAssets)
    {
        UItemData* ItemData = Cast<UItemData>(Asset);
        if (!ItemData) continue;
        if (!ItemClassByType.FindRef(ItemData->ItemType)) continue;

        OutItems.Add(ItemData);
    }

    COMMON_LOG(LogGameplay, Log, TEXT("Spawnable items : %d"), OutItems.Num());
}

UItemData* USpawnDirectorComponent::SelectItemData(const TArray<UItemData*>& SpawnableItems) const
{
    if (SpawnableItems.IsEmpty()) return nullptr;

    int32 Index = FMath::RandRange(0, SpawnableItems.Num() - 1);
    return SpawnableItems[Index];
}


void USpawnDirectorComponent::OnBuildingEntered(ABuildingActor* Building)
{
    for (FItemSpawnEntry& Entry : ItemSpawnEntries)
    {
        if (Entry.OwnerBuilding != Building) continue;
        if (Entry.bSpawned) continue;
        if (!Entry.ItemData) continue;

        TSubclassOf<AItem> ItemClass = ItemClassByType.FindRef(Entry.ItemData->ItemType);
        if (!ItemClass) continue;

        const FTransform SpawnTransform(FRotator::ZeroRotator, Entry.Location);

        AItem* Item = GetWorld()->SpawnActorDeferred<AItem>(
            ItemClass,
            SpawnTransform,
            nullptr,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

        if (!Item) continue;

        // 데이터는 BeginPlay 전에 넣는다
        Item->SetItemData(Entry.ItemData);
        Item->FinishSpawning(SpawnTransform);

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

void USpawnDirectorComponent::OnDayCycleChanged(EDayCycle DayCycle)
{
    if (DayCycle == EDayCycle::Night)
    {
        StartZombieWave();
    }
    else
    {
        // Stop Night Wave
        StopZombieWave();
    }
}

void USpawnDirectorComponent::RegisterPlayer(APawn* PlayerPawn)
{
    if (AConvict* Player = Cast<AConvict>(PlayerPawn))
    {
        ZombieAssignmentMap.Add(Player, TArray<TObjectPtr<AZombie>>());
    }
}

void USpawnDirectorComponent::UnregisterPlayer(APawn* PlayerPawn)
{
    if (AConvict* Player = Cast<AConvict>(PlayerPawn))
    {
        // 삭제하기 전에 생성된 좀비들이 있으면 Destroy해야 함.
        // 현재는 플레이어 이탈이 고려사항이 아니니 나중에 처리할 것

        ZombieAssignmentMap.Remove(Player);
    }
}

void USpawnDirectorComponent::StartZombieWave()
{
    if (ZombieAssignmentMap.IsEmpty() || !ZombieClass)
    {
        if (ZombieAssignmentMap.IsEmpty())
        {
            COMMON_LOG(LogGameplay, Warning, TEXT("Players does not saved in ZombieAssignmentMap."));
        }
        
        if(!ZombieClass)
        {
            COMMON_LOG(LogGameplay, Warning, TEXT("Can't find ZombieClass."));
        }

        return;
    }

    // 좀비 스폰 시작
    for (auto& [Convict, Zombies] : ZombieAssignmentMap)
    {
        // 위치·스폰 실패가 반복돼도 반복문이 끝나도록 시도 횟수 상한을 둔다.
        const int32 MaxAttempts = MaxZombiePerPlayer * 3;
        int32 Attempts = 0;

        while (Zombies.Num() < MaxZombiePerPlayer && Attempts < MaxAttempts)
        {
            ++Attempts;

            // 현재는 일정 거리 이내의 가능한 좌표에서 생성하도록 해놓았으나
            // 나중에 플레이어로부터 어느정도 멀어진 곳 <-> 최대 거리를 기준으로 만들어야 할 듯.
            FVector Location;
            if (!FindSpawnLocation(Convict->GetActorLocation(), Location))
            {
                continue;
            }

            FRotator Rotation = FRotator::ZeroRotator;

            AZombie* NewZombie = GetWorld()->SpawnActor<AZombie>(ZombieClass, Location, Rotation);
            if (NewZombie)
            {
                // 좀비 스폰 초기 세팅
                // 예: SpawnOwner - Player 설정, 플레이어 위치를 등록해놓고 발견 전까지 그 주변을 돌도록
                // + 좀비가 죽었을 때를 알아야 TArray 내에서 삭제도 가능함.
                NewZombie->OnZombieDeath.AddUObject(this, &ThisClass::OnZombieDeath);
                NewZombie->InitializeZombie(Convict);
                Zombies.Add(NewZombie);

                COMMON_LOG(LogGameplay, Log, TEXT("Spawn Zombie. Assigned player : %s, Current assigned zombies is %d"), *GetNameSafe(Convict), Zombies.Num());
            }
            else
            {
                COMMON_LOG(LogGameplay, Warning, TEXT("Failed to spawn zombie at %s."), *Location.ToString());
            }
        }

        if (Zombies.Num() < MaxZombiePerPlayer)
        {
            COMMON_LOG(LogGameplay, Warning, TEXT("Zombie spawn attempts exhausted. Assigned player : %s, Spawned %d / %d"), *GetNameSafe(Convict), Zombies.Num(), MaxZombiePerPlayer);
        }
    }
}

void USpawnDirectorComponent::StopZombieWave()
{

}

bool USpawnDirectorComponent::FindSpawnLocation(const FVector PlayerLocation, FVector& OutLocation)
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Failed to find NavigationSystem."));
        return false;
    }

    FNavLocation NavLocation;

    bool IsSuccess = NavSys->GetRandomReachablePointInRadius(PlayerLocation, SpawnZombieDistance, NavLocation);
    if (IsSuccess)
    {
        OutLocation = NavLocation.Location;
    }

    return IsSuccess;
}

void USpawnDirectorComponent::OnZombieDeath(AZombie* Zombie)
{
    if (!Zombie)
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Zombie is nullptr"));
        return;
    }

    Zombie->OnZombieDeath.RemoveAll(this);

    // 좀비 찾아서 삭제
    TArray<TObjectPtr<AZombie>>* Zombies = ZombieAssignmentMap.Find(Cast<AConvict>(Zombie->GetAssignedCharacter()));
    if (Zombies)
    {
        Zombies->Remove(Zombie);
    }
    else
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Failed to find zombie array. Convict is %s"), *GetNameSafe(Zombie->GetAssignedCharacter()));
    }
}
