// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/GameFramework/LobbyPlayerState.h"
#include "Common/GameFramework/PlayerControllerBase.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogLobbyPlayerState);

void ALobbyPlayerState::SetReady(bool bNewReady)
{
	if (bIsReady == bNewReady)
	{
		return;
	}

	bIsReady = bNewReady;
	OnReadyChanged.Broadcast(bIsReady);	// 서버 측 즉시 반응용
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bIsReady);
}

void ALobbyPlayerState::OnRep_IsReady()
{
	OnReadyChanged.Broadcast(bIsReady);
}
