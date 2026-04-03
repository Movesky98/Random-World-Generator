// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLobbyGameMode, Log, All);

/**
 * 
 */
UCLASS()
class RWG_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ALobbyGameMode();

	void HandleReadyRequest(class APlayerControllerBase* PC, bool bNewReady);

protected:
	virtual void BeginPlay() override;

	void TryStartGame();
};
