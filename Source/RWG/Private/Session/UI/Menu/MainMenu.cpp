// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/UI/Menu/MainMenu.h"
#include "Gameplay/Components/UIManagerComponent.h"
#include "CommonLogCategories.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensure(StartButton != nullptr)) return;
	if (!ensure(QuitButton != nullptr)) return;

	StartButton->OnClicked.AddDynamic(this, &ThisClass::OnStartButtonClicked);
	QuitButton->OnClicked.AddDynamic(this, &ThisClass::OnQuitButtonClicked);
}

void UMainMenu::OnStartButtonClicked()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController) return;

	UUIManagerComponent* UIManager = PlayerController->FindComponentByClass<UUIManagerComponent>();
	if (!UIManager)
	{
		COMMON_LOG(LogSession, Error, TEXT("Can't find UIManagerComponent."));
		return;
	}

	UIManager->ShowWidget(EUIType::SessionMenu);
}

void UMainMenu::OnQuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
