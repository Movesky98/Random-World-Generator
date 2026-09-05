// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/GameFramework/ExpeditionGameMode.h"
#include "Gameplay/GameFramework/ExpeditionGameState.h"
#include "Gameplay/GameFramework/ExpeditionPlayerController.h"
#include "Gameplay/GameFramework/ExpeditionPlayerState.h"
#include "Gameplay/Characters/CharacterBase/CharacterBase.h"
#include "Gameplay/Components/SpawnDirectorComponent.h"
#include "Gameplay/Components/InventoryComponent.h"
#include "Gameplay/Components/TimeManagementComponent.h"
#include "Gameplay/Data/ExtractionConditionRow.h"
#include "Gameplay/Characters/Convict/Convict.h"
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


}

void AExpeditionGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    SpawnDirectorComponent->UnregisterPlayer(Exiting->GetPawn());

    if (AConvict* Player = Cast<AConvict>(Exiting->GetPawn()))
    {
        UnsubscribeInventoryComponent(Player);
    }

    APlayerState* PS = Exiting->PlayerState;
    if (PS && ReportedPlayers.Contains(PS))
    {
        ReportedPlayers.Remove(PS);
    }

    TryGameStart(PS);
}

void AExpeditionGameMode::OnWorldReady()
{
    COMMON_LOG(LogGameplay, Log, TEXT("World generation is completed."));
    SpawnDirectorComponent->InitializeSpawnData();
    InitializeExtractionConditions();

    if (AExpeditionGameState* ExpeditionGS = GetGameState<AExpeditionGameState>())
    {
        ExpeditionGS->OnGameOver.AddUObject(this, &ThisClass::GameOver);
        ExpeditionGS->SetGameplayState(EGameplayState::WaitingForPlayers);
    }

    for (auto& PS : ReportedPlayers)
    {
        if (!PS.IsValid()) continue;

        APlayerController* PC = PS->GetPlayerController();
        if (PC)
        {
            RestartPlayer(PC);
        }
    }

    TryGameStart();
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

void AExpeditionGameMode::ReportWorldGenerationCompleted(APlayerController* Player)
{
    // TODO : 
    // 1. 랜덤 월드 생성 완료 신고한 플레이어 저장
    // 2. 호스트의 월드 생성 여부에 따라 RestartPlayer() 실행. 
    // 3. TryStartGame()

    if (!Player)
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Player is nullptr."));
        return;
    }

    APlayerState* PS = Player->GetPlayerState<APlayerState>();
    if (!PS) return;
    
    // ReportPlayers에 추가할 때 이미 보고한 플레이어면 게임 시작하지 않음.
    bool bIsAlready = false;
    ReportedPlayers.Add(PS, &bIsAlready);
    if (bIsAlready) return;

    UWorldGenSubsystem* WorldGenSubsystem = GetWorld()->GetSubsystem<UWorldGenSubsystem>();
    if (!WorldGenSubsystem) return;

    if (WorldGenSubsystem->IsWorldReady())
    {
        RestartPlayer(Player);

        TryGameStart();
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

void AExpeditionGameMode::TryGameStart(APlayerState* IgnorePS)
{
    // TODO:
    // 1. GameState::PlayerArray가 ReportedPlayers에 전부 존재하는지 확인(bIsAllReported)
    // 2. bIsAllReported == true → 게임 시작

    AExpeditionGameState* ExpeditionGS = Cast<AExpeditionGameState>(GameState);
    if (!ExpeditionGS || ExpeditionGS->GetGameplayState() >= EGameplayState::Preparing) return;
    
    bool bIsAllReported = true;

    for (auto& PlayerState : ExpeditionGS->PlayerArray)
    {
        if (PlayerState == IgnorePS) continue;

        if (!ReportedPlayers.Contains(PlayerState))
        {
            bIsAllReported = false;
            break;
        }
    }

    if (bIsAllReported)
        PrepareGameStart();
}

void AExpeditionGameMode::PrepareGameStart()
{
    // TODO :
    // 1. 게임플레이 상태 : Preparing
    // 2. 게임 시작 타이머 표시
    // 3. 플레이어 스폰 위치 지정 (아직 없음)
    if (AExpeditionGameState* ExpeditionGS = Cast<AExpeditionGameState>(GameState))
    {
        float GameStartTime = (float)ExpeditionGS->GetServerWorldTimeSeconds() + PrepareGameTime;
        COMMON_LOG(LogGameplay, Warning, TEXT("[Prepare] Now=%.2f, PrepareTime=%.2f, GameStartTime=%.2f"),
            ExpeditionGS->GetServerWorldTimeSeconds(), PrepareGameTime, GameStartTime);
        ExpeditionGS->SetGameStartTime(GameStartTime);

        ExpeditionGS->SetGameplayState(EGameplayState::Preparing);
        // 게임 시작 타이머 설정
        FTimerHandle GameStartTimerHandle;
        GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ThisClass::GameStart, PrepareGameTime, false);
    }
}

void AExpeditionGameMode::GameStart()
{
    // TODO: 게임 시작 알림
    // 1. 게임플레이 상태 : Playing
    // 2. TimeManagementComponent 시간 재생 시작
    if (AExpeditionGameState* ExpeditionGS = Cast<AExpeditionGameState>(GameState))
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("[GameStart] Now=%.2f"), ExpeditionGS->GetServerWorldTimeSeconds());
        ExpeditionGS->SetGameplayState(EGameplayState::Playing);
    }

    TimeManagementComponent->SetComponentTickEnabled(true);
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
