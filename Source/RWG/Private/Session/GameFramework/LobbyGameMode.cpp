// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/GameFramework/LobbyGameMode.h"
#include "Session/GameFramework/LobbyGameState.h"
#include "Session/GameFramework/LobbyPlayerState.h"
#include "Session/GameFramework/SessionSubsystem.h"
#include "Common/GameFramework/PlayerControllerBase.h"
#include "CommonLogCategories.h"

ALobbyGameMode::ALobbyGameMode()
{
	PlayerControllerClass = APlayerControllerBase::StaticClass();
	GameStateClass = ALobbyGameState::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();

	bUseSeamlessTravel = true;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	USessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<USessionSubsystem>();
	ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>();
	
	if (!SessionSubsystem || !LobbyGameState)
	{
		COMMON_LOG(LogSession, Error, TEXT("Can't find SessionSubsystem or LobbyGameState."));
		return;
	}

	const FLobbySessionInfo LobbySessionInfo = SessionSubsystem->GetLobbySessionInfo();
	LobbyGameState->SetLobbySessionInfo(LobbySessionInfo);
}

void ALobbyGameMode::HandleReadyRequest(APlayerControllerBase* PC, bool bNewReady)
{
	if (!PC)
	{
		COMMON_LOG(LogSession, Error, TEXT("Invalid playercontroller parameter."));
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
		FTimerHandle StartGameTimerHandle;

		GetWorldTimerManager().SetTimer(StartGameTimerHandle, this, &ThisClass::StartGame, StartGameDelay, false);
	}
}

void ALobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->ServerTravel("/Game/Features/WorldGenerator/Levels/LV_ExpeditionWorld?listen");
	}
}
