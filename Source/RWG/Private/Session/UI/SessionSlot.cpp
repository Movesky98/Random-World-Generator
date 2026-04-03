// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/UI/SessionSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

DEFINE_LOG_CATEGORY(LogSessionSlot);

void USessionSlot::InitializeSessionData(const FSessionData InSessionData)
{
	SessionData = InSessionData;
}

void USessionSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensure(SessionNameText != nullptr)) return;
	if (!ensure(PlayerCountText != nullptr)) return;
	if (!ensure(PingText != nullptr)) return;
	if (!ensure(JoinButton != nullptr)) return;

	FString PlayerCountStr = FString::FromInt(SessionData.CurrentPlayers) + TEXT(" / ") + FString::FromInt(SessionData.MaxPlayers);
	FString PingStr = FString::FromInt(SessionData.Ping);

	SessionNameText->SetText(FText::FromName(SessionData.SessionName));
	PlayerCountText->SetText(FText::FromString(PlayerCountStr));
	PingText->SetText(FText::FromString(PingStr));
	JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
}

void USessionSlot::OnJoinButtonClicked()
{
	OnJoinButtonClickedEvent.Broadcast(SessionData.SessionIndex);
}
