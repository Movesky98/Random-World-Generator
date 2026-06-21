// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/SpawnDirectorComponent.h"
#include "GamePlay/Items/Item.h"
#include "GamePlay/Items/ItemData.h"
#include "GamePlay/Components/TimeManagementComponent.h"
#include "GamePlay/Components/HealthComponent.h"
#include "GamePlay/Characters/Convict/Convict.h"
#include "GamePlay/Characters/Zombie/Zombie.h"

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
        while (Zombies.Num() < MaxZombiePerPlayer)
        {
            // 현재는 일정 거리 이내의 가능한 좌표에서 생성하도록 해놓았으나
            // 나중에 플레이어로부터 어느정도 멀어진 곳 <-> 최대 거리를 기준으로 만들어야 할 듯.
            FVector Location = FindSpawnLocation(Convict->GetActorLocation());
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
        }
    }
}

void USpawnDirectorComponent::StopZombieWave()
{

}

FVector USpawnDirectorComponent::FindSpawnLocation(const FVector PlayerLocation)
{
    FVector SpawnLocation = FVector::ZeroVector;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Failed to find NavigationSystem."));
        return SpawnLocation;
    }

    FNavLocation OutLocation;

    bool IsSuccess = NavSys->GetRandomReachablePointInRadius(PlayerLocation, SpawnZombieDistance, OutLocation);
    if (IsSuccess)
    {
        SpawnLocation = OutLocation.Location;
    }

    return SpawnLocation;
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
    TArray<TObjectPtr<AZombie>> Zombies = ZombieAssignmentMap.FindRef(Cast<AConvict>(Zombie->GetAssignedCharacter()));
    if (!Zombies.IsEmpty())
    {
        int32 Index = Zombies.Find(Zombie);
        if (Index != INDEX_NONE)
        {
            Zombies.RemoveAt(Index);
        }
    }
    else
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Zombie Array is already empty. Convict is %s"), *GetNameSafe(Zombie->GetAssignedCharacter()));
    }
}
