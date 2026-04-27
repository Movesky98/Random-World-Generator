// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/UI/UserWidgetBase.h"

void UUserWidgetBase::SetUp()
{
	SetVisibility(ESlateVisibility::Visible);

	FInputModeUIOnly InputModeData;

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void UUserWidgetBase::TearDown()
{
	FInputModeGameOnly InputModeData;

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

EWidgetType UUserWidgetBase::GetWidgetType() const
{
	return WidgetType;
}
