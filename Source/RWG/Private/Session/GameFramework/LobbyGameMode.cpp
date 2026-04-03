// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/GameFramework/LobbyGameMode.h"
#include "Session/GameFramework/LobbyGameState.h"
#include "Session/GameFramework/LobbyPlayerState.h"
#include "Session/GameFramework/SessionSubsystem.h"
#include "Common/GameFramework/PlayerControllerBase.h"

DEFINE_LOG_CATEGORY(LogLobbyGameMode);

ALobbyGameMode::ALobbyGameMode()
{
	PlayerControllerClass = APlayerControllerBase::StaticClass();
	GameStateClass = ALobbyGameState::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	USessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<USessionSubsystem>();
	ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>();
	
	if (!SessionSubsystem || !LobbyGameState)
	{
		UE_LOG(LogLobbyGameMode, Error, TEXT("BeginPlay() :: Can't find SessionSubsystem or LobbyGameState."));
		return;
	}

	const FLobbySessionInfo LobbySessionInfo = SessionSubsystem->GetLobbySessionInfo();
	LobbyGameState->SetLobbySessionInfo(LobbySessionInfo);
}

void ALobbyGameMode::HandleReadyRequest(APlayerControllerBase* PC, bool bNewReady)
{
	if (!PC)
	{
		UE_LOG(LogLobbyGameMode, Error, TEXT("HandleReadyRequest() :: Invalid playercontroller parameter."));
		return;
	}

	ALobbyPlayerState* LobbyPS = PC->GetPlayerState<ALobbyPlayerState>();
	ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>();

	if (!LobbyPS || !LobbyGS)
	{
		return;
	}

	LobbyPS->SetReady(bNewReady);
	
	LobbyGS->RefreshReadySummary();

	if (LobbyGS->AreAllPlayersReady())
	{
		TryStartGame();
	}
}

void ALobbyGameMode::TryStartGame()
{
	// 랜덤 월드 제네레이션 레벨로 단체 이동
	UWorld* World = GetWorld();
	if (World)
	{
		World->ServerTravel("/Game/Features/WorldGenerator/Levels/LandscapeGeneration?listen");
	}
}