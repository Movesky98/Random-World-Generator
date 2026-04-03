// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/UI/PlayerHUD.h"

void UPlayerHUD::SetUp()
{
	this->AddToViewport();

	FInputModeGameOnly InputModeData;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController)) return;

	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(false);
}
