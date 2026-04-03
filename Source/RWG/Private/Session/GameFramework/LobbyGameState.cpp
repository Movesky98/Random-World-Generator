// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/GameFramework/LobbyGameState.h"
#include "Session/GameFramework/LobbyPlayerState.h"

#include "Net/UnrealNetwork.h"

ALobbyGameState::ALobbyGameState()
{
	bLobbyInfoInitialized = false;
}

void ALobbyGameState::SetLobbySessionInfo(const FLobbySessionInfo Info)
{
	if(!bLobbyInfoInitialized) 
		bLobbyInfoInitialized = true;
	
	LobbyInfo = Info;

	OnLobbySessionInfoUpdated.Broadcast();	// 서버 수신용
}

const FLobbySessionInfo ALobbyGameState::GetLobbySessionInfo() const
{
	return LobbyInfo;
}

void ALobbyGameState::RefreshReadySummary()
{
	int32 TotalCount = 0;
	int32 ReadyCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		if (ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PS))
		{
			++TotalCount;

			if (LobbyPS->IsReady())
			{
				++ReadyCount;
			}
		}
	}

	bAllPlayersReady = (TotalCount > 0 && TotalCount == ReadyCount);
}

bool ALobbyGameState::AreAllPlayersReady()
{
	return bAllPlayersReady;
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, LobbyInfo);
}

void ALobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnPlayerListUpdated.Broadcast();
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnPlayerListUpdated.Broadcast();
}

void ALobbyGameState::OnRep_LobbyInfo()
{
	OnLobbySessionInfoUpdated.Broadcast();
}
