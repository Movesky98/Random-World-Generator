// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/UI/GameOverWidget.h"
#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "CommonLogCategories.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensure(ConfirmRewardButton != nullptr)) return;
	if (!ensure(ReturnToLobbyButton != nullptr)) return;
	if (!ensure(ResultTitleText != nullptr)) return;

	ReturnToLobbyButton->OnClicked.AddDynamic(this, &ThisClass::OnReturnToLobbyButtonClicked);

	this->SetVisibility(ESlateVisibility::Collapsed);
}

void UGameOverWidget::OnReturnToLobbyButtonClicked()
{
	// Return To Lobby
	if (AExpeditionPlayerController* PC = GetOwningPlayer<AExpeditionPlayerController>())
	{
		COMMON_LOG(LogGameplay, Log, TEXT("Request return to lobby."));
		PC->RequestReturnToLobby();
	}
}
