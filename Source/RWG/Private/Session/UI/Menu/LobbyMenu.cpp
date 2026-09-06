// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/UI/Menu/LobbyMenu.h"
#include "Session/UI/PlayerSlot.h"

#include "Session/GameFramework/LobbyGameState.h"
#include "Common/GameFramework/PlayerControllerBase.h"
#include "CommonLogCategories.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"

ULobbyMenu::ULobbyMenu()
{

}

void ULobbyMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensure(SessionNameText != nullptr)) return;
	if (!ensure(PlayerCountText != nullptr)) return;
	if (!ensure(PlayerListScrollBox != nullptr)) return;
	if (!ensure(ExitButton != nullptr)) return;
	if (!ensure(ReadyButton != nullptr)) return;

	ExitButton->OnClicked.AddDynamic(this, &ThisClass::ExitSession);
	ReadyButton->OnClicked.AddDynamic(this, &ThisClass::OnReadyButtonClicked);

	if (ALobbyGameState* LobbyGS = GetLobbyGameState())
	{
		if (LobbyGS->bLobbyInfoInitialized)
			InitializeLobbyView();
		
		LobbyGS->OnLobbySessionInfoUpdated.AddUObject(this, &ThisClass::InitializeLobbyView);
		LobbyGS->OnPlayerListUpdated.AddUObject(this, &ThisClass::RefreshPlayerList);
		LobbyGS->OnPlayerListUpdated.AddUObject(this, &ThisClass::RefreshPlayerCount);
	}
	else
	{
		COMMON_LOG(LogSession, Error, TEXT("LobbyMenu is %s"), LobbyGS ? TEXT("Valid") : TEXT("Null"));
	}
}

void ULobbyMenu::NativeDestruct()
{
	if (ALobbyGameState* LobbyGS = GetLobbyGameState())
	{
		LobbyGS->OnLobbySessionInfoUpdated.RemoveAll(this);	
		LobbyGS->OnPlayerListUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

ALobbyGameState* ULobbyMenu::GetLobbyGameState()
{
	UWorld* World = GetWorld();

	if (World)
	{
		return GetWorld()->GetGameState<ALobbyGameState>();
	}

	return nullptr;
}

void ULobbyMenu::RefreshPlayerList()
{
	ALobbyGameState* LobbyGS = GetLobbyGameState();
	if (!LobbyGS || !PlayerListScrollBox)
	{
		COMMON_LOG(LogSession, Error, TEXT("Failed to reference PlayerListScrollBox or Lobby GameState."));
		return;
	}

	PlayerListScrollBox->ClearChildren();

	for (TObjectPtr<APlayerState> PS : LobbyGS->PlayerArray)
	{
		if (ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PS))
		{
			UPlayerSlot* PlayerSlot = CreateWidget<UPlayerSlot>(GetWorld(), PlayerSlotClass);
			PlayerSlot->BindPlayerState(LobbyPS);
			PlayerListScrollBox->AddChild(PlayerSlot);
		}
	}
}

void ULobbyMenu::RefreshPlayerCount()
{
	ALobbyGameState* LobbyGS = GetLobbyGameState();
	if (!LobbyGS || !PlayerCountText)
	{
		COMMON_LOG(LogSession, Error, TEXT("Failed to reference PlayerListScrollBox or Lobby GameState."));
		return;
	}

	const FLobbySessionInfo LobbyInfo = LobbyGS->GetLobbySessionInfo();
	FString PlayerCountStr;
	PlayerCountStr = FString::FromInt(LobbyGS->PlayerArray.Num()) + TEXT(" / ") + FString::FromInt(LobbyInfo.MaxPlayers);
	PlayerCountText->SetText(FText::FromString(PlayerCountStr));
}

void ULobbyMenu::InitializeLobbyView()
{
	if (!PlayerSlotClass)
	{
		COMMON_LOG(LogSession, Error, TEXT("Set up player class in LobbyMenu"));
		return;
	}

	if (ALobbyGameState* LobbyGS = GetLobbyGameState())
	{
		const FLobbySessionInfo LobbyInfo = LobbyGS->GetLobbySessionInfo();

		FString PlayerCountStr;
		PlayerCountStr = FString::FromInt(LobbyGS->PlayerArray.Num()) + TEXT(" / ") + FString::FromInt(LobbyInfo.MaxPlayers);

		SessionNameText->SetText(FText::FromName(LobbyInfo.SessionName));
		PlayerCountText->SetText(FText::FromString(PlayerCountStr));
	}

	RefreshPlayerList();
}

void ULobbyMenu::ExitSession()
{
	// TODO : 세션 나가기
}

void ULobbyMenu::OnReadyButtonClicked()
{
	APlayerControllerBase* PCBase = GetOwningPlayer<APlayerControllerBase>();
	if (!PCBase)
	{
		return;
	}

	ALobbyPlayerState* LobbyPS = PCBase->GetPlayerState<ALobbyPlayerState>();
	if (!LobbyPS)
	{
		return;
	}

	bool bNewReady = !LobbyPS->IsReady();
	PCBase->RequestSetReady(bNewReady);
}
