// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionGameMode.h"
#include "GamePlay/GameFramework/ExpeditionGameState.h"
#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "GamePlay/GameFramework/ExpeditionPlayerState.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"
#include "GamePlay/Components/SpawnDirectorComponent.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/Components/TimeManagementComponent.h"
#include "GamePlay/Data/ExtractionConditionRow.h"
#include "GamePlay/Characters/Convict/Convict.h"
#include "CommonLogCategories.h"

#include "RandomWorldGeneration/GameFramework/WorldGenSubsystem.h"
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

    bUseSeamlessTravel = true;
    bStartPlayersAsSpectators = true;
}

void AExpeditionGameMode::BeginPlay()
{
	Super::BeginPlay();

    if (UWorldGenSubsystem* WorldGenSubsys = GetWorld()->GetSubsystem<UWorldGenSubsystem>())
    {
        if (!WorldGenSubsys->IsWorldReady())
        {
            WorldGenSubsys->OnWorldReady.AddUObject(this, &ThisClass::OnWorldReady);
        }
        else
            OnWorldReady();
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

    // 탈출 조건에 해당하는 아이템을 등록하는 오브젝트가 구현이 안되어있어.
    // 당장은 플레이어가 아이템을 획득했을 때 체크하는 걸로 대체함
    if (AConvict* Player = Cast<AConvict>(NewPlayer->GetPawn()))
    {
        SubscribeInventoryComponent(Player);
    }
}

void AExpeditionGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (UWorldGenSubsystem* WorldGenSubsys = GetWorld()->GetSubsystem<UWorldGenSubsystem>())
    {
        if (WorldGenSubsys->IsWorldReady())
        {
            RestartPlayer(NewPlayer);
        }
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

void AExpeditionGameMode::OnWorldReady()
{
    COMMON_LOG(LogGameplay, Log, TEXT("World generation is completed."));
    SpawnDirectorComponent->InitializeSpawnData();
    InitializeExtractionConditions();

    if (AExpeditionGameState* GS = GetGameState<AExpeditionGameState>())
    {
        GS->OnGameOver.AddUObject(this, &ThisClass::GameOver);
    }

    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if(It->Get())
                RestartPlayer(It->Get());
        }
    }
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

void AExpeditionGameMode::ReturnToLobby()
{
    if (UWorld* World = GetWorld())
    {
        World->ServerTravel("/Game/Features/Session/Levels/LV_Lobby?listen");
    }
}

void AExpeditionGameMode::GameOver()
{
    // TODO : 타이머 설정
    // 30초 후에 ReturnToLobby() 호출하는 타이머 설정
    // 얘는 ExpeditionGameState::bGameOver가 설정될 때 호출됨.

    GetWorldTimerManager().SetTimer(ReturnToLobbyTimerHandle, this, &ThisClass::ReturnToLobby, 10.0f, false);
    COMMON_LOG(LogGameplay, Warning, TEXT("GameOver."));
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
