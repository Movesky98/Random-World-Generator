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
public:
	ALobbyGameState();

	void SetLobbySessionInfo(const FLobbySessionInfo Info);

	const FLobbySessionInfo GetLobbySessionInfo() const;

	FOnLobbySessionInfoUpdated OnLobbySessionInfoUpdated;

	FOnPlayerListUpdated OnPlayerListUpdated;

	void RefreshReadySummary();

	bool AreAllPlayersReady();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session")
	bool bLobbyInfoInitialized;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;


	UPROPERTY(ReplicatedUsing = OnRep_LobbyInfo)
	FLobbySessionInfo LobbyInfo;

	UFUNCTION()
	void OnRep_LobbyInfo();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session|Lobby")
	int32 ReadyPlayerCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session|Lobby")
	bool bAllPlayersReady;
};
