// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/GameFramework/GlobalUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Common/UI/CountdownWidget.h"
#include "CommonLogCategories.h"

void UGlobalUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

}

void UGlobalUISubsystem::Deinitialize()
{
	Super::Deinitialize();

}

void UGlobalUISubsystem::ShowLoadingScreen()
{
	COMMON_LOG(LogGameplay, Log, TEXT("ShowLoadingScreen called."));

	bWorldGenReady = false;
	bHUDReady = false;

	UClass* LoadingScreenClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Features/Gameplay/BP/UI/WBP_LoadingScreen.WBP_LoadingScreen_C"));
	if (!LoadingScreenClass)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Load LoadingScreen failed."));
		return;
	}

	if (LoadingScreen = CreateWidget<UUserWidget>(GetGameInstance(), LoadingScreenClass))
	{
		LoadingScreen->AddToViewport(1000);
	}
	else
		COMMON_LOG(LogGameplay, Warning, TEXT("Create LoadingScreen Widget failed."));
}

void UGlobalUISubsystem::HideLoadingScreen()
{
	COMMON_LOG(LogGameplay, Log, TEXT("HideLoadingScreen called."));

	if (LoadingScreen)
	{
		LoadingScreen->RemoveFromParent();
		LoadingScreen = nullptr;
	}
}

void UGlobalUISubsystem::ShowCountdownWidget(const FText& InTitle, const FGetRemainingSecondsDelegate& InDelegate)
{
	COMMON_LOG(LogGameplay, Log, TEXT("Show Countdown Widget."));

	if (CountdownWidget)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Countdown Widget already created."));
		return;
	}

	UClass* CountdownWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Features/Common/BP/UI/WBP_Countdown.WBP_Countdown_C"));
	if (!CountdownWidgetClass)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Load Countdown Widget failed."));
		return;
	}

	if (CountdownWidget = CreateWidget<UCountdownWidget>(GetGameInstance(), CountdownWidgetClass))
	{
		CountdownWidget->InitializeCountdown(InTitle, InDelegate);
		CountdownWidget->AddToViewport(100);
	}
	else
		COMMON_LOG(LogGameplay, Warning, TEXT("Create Countdown Widget Widget failed."));
}

void UGlobalUISubsystem::HideCountdownWidget()
{
	COMMON_LOG(LogGameplay, Log, TEXT("Hide Countdown Widget."));

	if (CountdownWidget)
	{
		CountdownWidget->RemoveFromParent();
		CountdownWidget = nullptr;
	}
}

void UGlobalUISubsystem::TryHideLoadingScreen()
{
	if (bWorldGenReady && bHUDReady)
	{
		HideLoadingScreen();
	}
}

void UGlobalUISubsystem::NotifyWorldGenReady()
{
	bWorldGenReady = true;

	TryHideLoadingScreen();
}

void UGlobalUISubsystem::NotifyHUDReady()
{
	bHUDReady = true;

	TryHideLoadingScreen();
}
