// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	ALobbyGameMode();

protected:
	virtual void BeginPlay() override;

/*********************************************************************
*                             게임 시작
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby|Time")
	float StartGameDelay = 5.0f;

	void StartGame();

public:
	void HandleReadyRequest(class APlayerControllerBase* PC, bool bNewReady);
};
