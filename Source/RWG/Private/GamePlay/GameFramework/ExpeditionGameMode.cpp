// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionGameMode.h"
#include "GamePlay/GameFramework/ExpeditionGameState.h"
#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "GamePlay/GameFramework/ExpeditionPlayerState.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"

AExpeditionGameMode::AExpeditionGameMode()
{
	DefaultPawnClass = ACharacterBase::StaticClass();
	PlayerControllerClass = AExpeditionPlayerController::StaticClass();
	GameStateClass = AExpeditionGameState::StaticClass();
	PlayerStateClass = AExpeditionPlayerState::StaticClass();
}
