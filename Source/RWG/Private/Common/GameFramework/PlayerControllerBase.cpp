// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/GameFramework/PlayerControllerBase.h"

#include "Session/GameFramework/LobbyPlayerState.h"
#include "Session/GameFramework/LobbyGameMode.h"

#include "Session/UI/Menu/SessionMenu.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogPlayerControllerBase);

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	UpdateUIForCurrentLevel();
}

void APlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentWidget)
	{
		CurrentWidget->TearDown();
		CurrentWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void APlayerControllerBase::UpdateUIForCurrentLevel()
{
	FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UE_LOG(LogPlayerControllerBase, Warning, TEXT("Current Level Name : %s"), *LevelName);

	EUIType UIType = ConvertLevelNameToEnum(LevelName);

	if(UIType != EUIType::None)
		ShowUI(UIType);
}

EUIType APlayerControllerBase::ConvertLevelNameToEnum(FString LevelName)
{
	if (LevelName.Contains("Session"))
		return EUIType::SessionMenu;

	if (LevelName.Contains("Lobby"))
		return EUIType::LobbyMenu;

	if (LevelName.Contains("LandscapeGeneration"))
		return EUIType::PlayerHUD;

	UE_LOG(LogPlayerControllerBase, Error, TEXT("ConvertLevelNameToEnum() :: Failed to convert level name to enum."));
	return EUIType();
}

void APlayerControllerBase::ShowUI(EUIType UIType)
{
	if (CurrentWidget)
	{
		CurrentWidget->TearDown();
		CurrentWidget = nullptr;
	}

	TSubclassOf<UUserWidgetBase>* FoundClass = UIClassMap.Find(UIType);
	if (!FoundClass || !(*FoundClass))
	{
		UE_LOG(LogPlayerControllerBase, Error, TEXT("ShowUI() :: Can't find UIClass from enum type. (%s)"), *UEnum::GetValueAsString(UIType));
		return;
	}

	CurrentWidget = CreateWidget<UUserWidgetBase>(GetWorld(), *FoundClass);
	if (CurrentWidget)
	{
		CurrentWidget->SetUp();
	}
}

void APlayerControllerBase::RequestSetReady(bool bNewReady)
{
	ServerSetReady(bNewReady);
}

void APlayerControllerBase::ServerSetReady_Implementation(bool bNewReady)
{
	ALobbyPlayerState* LobbyPS = GetPlayerState<ALobbyPlayerState>();
	ALobbyGameMode* LobbyGM = GetWorld()->GetAuthGameMode<ALobbyGameMode>();

	if (!LobbyPS || !LobbyGM)
	{
		UE_LOG(LogPlayerControllerBase, Error, TEXT("ServerSetReady() :: Please check LobbyPlayerState or LobbyGameMode."));
		return;
	}

	LobbyGM->HandleReadyRequest(this, bNewReady);
}
