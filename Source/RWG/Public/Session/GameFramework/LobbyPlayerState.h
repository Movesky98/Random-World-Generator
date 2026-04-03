// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLobbyPlayerState, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, bool IsReady);

/**
 * 
 */
UCLASS()
class RWG_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	void SetReady(bool bNewReady);

	bool IsReady() const { return bIsReady; }
	
	FOnReadyChanged OnReadyChanged;
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_IsReady)
	bool bIsReady = false;

	UFUNCTION()
	void OnRep_IsReady();
};
