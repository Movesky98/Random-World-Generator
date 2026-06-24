// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionGameMode.h"
#include "GamePlay/GameFramework/ExpeditionGameState.h"
#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "GamePlay/GameFramework/ExpeditionPlayerState.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"
#include "GamePlay/Components/SpawnDirectorComponent.h"
#include "GamePlay/Components/TimeManagementComponent.h"
#include "GamePlay/Data/ExtractionConditionRow.h"
#include "CommonLogCategories.h"

#include "GamePlay/Characters/Convict/Convict.h"
#include "GamePlay/Components/InventoryComponent.h"


#include "RandomWorldGeneration/Actors/WorldGenerator.h"

#include "Kismet/GameplayStatics.h"

AExpeditionGameMode::AExpeditionGameMode()
{
	DefaultPawnClass = ACharacterBase::StaticClass();
	PlayerControllerClass = AExpeditionPlayerController::StaticClass();
	GameStateClass = AExpeditionGameState::StaticClass();
	PlayerStateClass = AExpeditionPlayerState::StaticClass();

	SpawnDirectorComponent = CreateDefaultSubobject<USpawnDirectorComponent>(TEXT("SpawnDirectorComponent"));
    TimeManagementComponent = CreateDefaultSubobject<UTimeManagementComponent>(TEXT("TimeManagementComponent"));
}

void AExpeditionGameMode::BeginPlay()
{
	Super::BeginPlay();

    if (AWorldGenerator* Generator = Cast<AWorldGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldGenerator::StaticClass())))
    {
        Generator->OnWorldGenerationComplete.AddLambda([this]()
            {
                COMMON_LOG(LogGameplay, Log, TEXT("World generation is completed."));
                SpawnDirectorComponent->InitializeSpawnData();
                InitializeExtractionConditions();
            });
    }
}

void AExpeditionGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);    // 이 시점에 Pawn이 Spawn + Possess

    if (!NewPlayer->GetPawn())
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Can't find player pawn."));

        return;
    }

    SpawnDirectorComponent->RegisterPlayer(NewPlayer->GetPawn());

    if (AConvict* Player = Cast<AConvict>(NewPlayer->GetPawn()))
    {
        SubscribeInventoryComponent(Player);
    }
}

void AExpeditionGameMode::Logout(AController* Exiting)
{
    SpawnDirectorComponent->UnregisterPlayer(Exiting->GetPawn());

    if (AConvict* Player = Cast<AConvict>(Exiting->GetPawn()))
    {
        UnsubscribeInventoryComponent(Player);
    }
    
    Super::Logout(Exiting);
}

void AExpeditionGameMode::InitializeExtractionConditions()
{
    // 탈출 조건 뽑기
    if (!ExtractionConditionTable)
    {
        COMMON_LOG(LogGameplay, Error, TEXT("ExtractionConditionTable is nullptr."));
        return;
    }

    // 우선 탈출 조건 하나만
    TArray<FExtractionConditionRow*> Rows;
    ExtractionConditionTable->GetAllRows<FExtractionConditionRow>(TEXT("Extraction Condition"), Rows);

    if (Rows.IsEmpty())
    {
        COMMON_LOG(LogGameplay, Error, TEXT("ExtractionConditionTable has no rows."));
        return;
    }

    int32 RandIndex = FMath::RandRange(0, Rows.Num() - 1);
    FExtractionConditionRow* Selected = Rows[RandIndex];
    if (Selected->ItemData == nullptr)
    {
        COMMON_LOG(LogGameplay, Error, TEXT("Selected row's ItemData is nullptr"));
        return;
    }

    COMMON_LOG(LogGameplay, Warning, TEXT("Extraction Condition is selected."));
    COMMON_LOG(LogGameplay, Warning, TEXT("Goal Item : %s, Goal Quantity : %d"), *GetNameSafe(Selected->ItemData), Selected->RequiredQuantity);

    // GS에 전달하기 위해 FExtractionCondition으로 변환
    TArray<FExtractionCondition> Conditions;
    
    FExtractionCondition NewCondition;
    NewCondition.ItemID = Selected->ItemData->ItemID;
    NewCondition.RequiredQuantity = Selected->RequiredQuantity;
    NewCondition.CurrentQuantity = 0;
    Conditions.Add(NewCondition);

    if (AExpeditionGameState* GS = GetGameState<AExpeditionGameState>())
    {
        GS->SetExtractionConditions(Conditions);
    }
}

void AExpeditionGameMode::SubscribeInventoryComponent(AConvict* Player)
{
    if (!Player)
    {
        return;
    }

    if (UInventoryComponent* InventoryComp = Player->GetComponentByClass<UInventoryComponent>())
    {
        InventoryComp->OnItemAdded.AddUObject(this, &ThisClass::OnInventoryItemAdded);
    }
}

void AExpeditionGameMode::UnsubscribeInventoryComponent(AConvict* Player)
{
    if (!Player)
    {
        return;
    }

    if (UInventoryComponent* InventoryComp = Player->GetComponentByClass<UInventoryComponent>())
    {
        InventoryComp->OnItemAdded.RemoveAll(this);
    }
}

void AExpeditionGameMode::OnInventoryItemAdded(UItemData* ItemData, int32 Quantity)
{
    AExpeditionGameState* GS = GetGameState<AExpeditionGameState>();
    if (GS)
    {
        GS->UpdateExtractionProgress(ItemData->ItemID, Quantity);
    }
}
