// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Session/GameFramework/SessionSubsystem.h"
#include "LobbyGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnLobbySessionInfoUpdated);
DECLARE_MULTICAST_DELEGATE(FOnPlayerListUpdated);

/**
 * 
 */
UCLASS()
class RWG_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	ALobbyGameState();

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                           플레이어 목록
*********************************************************************/
protected:
	virtual void AddPlayerState(APlayerState* PlayerState) override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

public:
	FOnPlayerListUpdated OnPlayerListUpdated;

/*********************************************************************
*                             세션 정보
*********************************************************************/
protected:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyInfo)
	FLobbySessionInfo LobbyInfo;

	UFUNCTION()
	void OnRep_LobbyInfo();

public:
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Session")
	bool bLobbyInfoInitialized;

	FOnLobbySessionInfoUpdated OnLobbySessionInfoUpdated;

	void SetLobbySessionInfo(const FLobbySessionInfo Info);

	const FLobbySessionInfo GetLobbySessionInfo() const;

/*********************************************************************
*                             준비 상태
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session|Lobby")
	int32 ReadyPlayerCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session|Lobby")
	bool bAllPlayersReady;

public:
	void RefreshReadySummary();

	bool AreAllPlayersReady();
};
