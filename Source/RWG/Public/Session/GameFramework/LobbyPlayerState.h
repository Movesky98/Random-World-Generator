// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, bool IsReady);

/**
 * 
 */
UCLASS()
class RWG_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                             준비 상태
*********************************************************************/
protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsReady)
	bool bIsReady = false;

	UFUNCTION()
	void OnRep_IsReady();

public:
	FOnReadyChanged OnReadyChanged;

	void SetReady(bool bNewReady);

	bool IsReady() const { return bIsReady; }
};
