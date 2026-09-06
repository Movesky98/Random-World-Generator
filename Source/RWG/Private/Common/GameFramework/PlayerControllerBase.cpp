// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/GameFramework/PlayerControllerBase.h"

#include "Session/GameFramework/LobbyPlayerState.h"
#include "Session/GameFramework/LobbyGameMode.h"

#include "Session/UI/Menu/SessionMenu.h"
#include "CommonLogCategories.h"
#include "Kismet/GameplayStatics.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APlayerControllerBase::RequestSetReady(bool bNewReady)
{
	ServerSetReady(bNewReady);
}

void APlayerControllerBase::ServerSetReady_Implementation(bool bNewReady)
{
	ALobbyPlayerState* LobbyPS = GetPlayerState<ALobbyPlayerState>();
	ALobbyGameMode* LobbyGM = GetWorld()->GetAuthGameMode<ALobbyGameMode>();

	if (!LobbyPS || !LobbyGM)
	{
		COMMON_LOG(LogSession, Error, TEXT("Please check LobbyPlayerState or LobbyGameMode."));
		return;
	}

	LobbyGM->HandleReadyRequest(this, bNewReady);
}
