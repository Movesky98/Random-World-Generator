// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/UI/Menu/SessionMenu.h"
#include "Session/UI/SessionSlot.h"
#include "Session/GameFramework/SessionSubsystem.h"

#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"

DEFINE_LOG_CATEGORY(LogSessionMenu);

void USessionMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ensure(MaxPlayersSlider != nullptr)) return;
	if (!ensure(CreateSessionButton != nullptr)) return;
	if (!ensure(FindSessionsButton != nullptr)) return;
	if (!ensure(SessionNameText != nullptr)) return;
	if (!ensure(LANCheckBox != nullptr)) return;
	if (!ensure(SessionScrollBox != nullptr)) return;

	MaxPlayersSlider->OnValueChanged.AddDynamic(this, &ThisClass::OnSliderValueChanged);
	CreateSessionButton->OnClicked.AddDynamic(this, &ThisClass::OnCreateSessionButtonClicked);
	FindSessionsButton->OnClicked.AddDynamic(this, &ThisClass::OnFindSessionButtonClicked);
	LANCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnCheckStateChanged);

	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		// RemoveAll(this), 중복 바인딩 방지
		SessionSubsystem->OnCreateSessionCompletedEvent.RemoveAll(this);
		SessionSubsystem->OnFindSessionsCompletedEvent.RemoveAll(this);
		SessionSubsystem->OnJoinSessionsCompletedEvent.RemoveAll(this);

		// 세션 델리게이트 바인딩
		SessionSubsystem->OnCreateSessionCompletedEvent.AddUObject(this, &ThisClass::OnCreateSessionCompleted);
		SessionSubsystem->OnFindSessionsCompletedEvent.AddUObject(this, &ThisClass::OnFindSessionsCompleted);
		SessionSubsystem->OnJoinSessionsCompletedEvent.AddUObject(this, &ThisClass::OnJoinSessionCompleted);

		UE_LOG(LogSessionMenu, Warning, TEXT("NativeConstruct() :: SessionMenu's delegate binding is succeeded."));
	}
	else
		UE_LOG(LogSessionMenu, Error, TEXT("NativeConstruct() :: Can't refer SessionSubsystem."));
}

void USessionMenu::NativeDestruct()
{
	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnCreateSessionCompletedEvent.RemoveAll(this);
		SessionSubsystem->OnFindSessionsCompletedEvent.RemoveAll(this);
		SessionSubsystem->OnJoinSessionsCompletedEvent.RemoveAll(this);
	}
	else
		UE_LOG(LogSessionMenu, Error, TEXT("NativeDesturct() :: Can't refer SessionSubsystem."));

	Super::NativeDestruct();
}

USessionSubsystem* USessionMenu::GetSessionSubsystem() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<USessionSubsystem>();
	}

	return nullptr;
}

void USessionMenu::OnCreateSessionButtonClicked()
{
	if (SessionNameText->GetText().ToString() == FString("") || SessionState != ESessionState::Idle)
	{
		// Show Error Msg and UI

		return;
	}

	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionState = ESessionState::Creating;
		const FName SessionName = FName(SessionNameText->GetText().ToString());
		const int32 MaxPlayers = FMath::RoundToInt(MaxPlayersSlider->GetValue());

		SessionSubsystem->CreateSession(MaxPlayers, SessionName, bIsLAN);
	}
	else
		UE_LOG(LogSessionMenu, Error, TEXT("OnCreateSessionButtonClicked() :: Can't refer SessionSubsystem."));
}

void USessionMenu::OnCreateSessionCompleted(bool bWasSuccessful)
{
	ensure(SessionState == ESessionState::Creating);
	SessionState = ESessionState::Idle;

	if (!bWasSuccessful)
	{
		// Show Error Msg and UI
		UE_LOG(LogSessionMenu, Warning, TEXT("OnCreateSessionCompleted() :: Failed to create session."));

		// 상태 초기화
		FName SessionName(TEXT(""));
		FName MaxPlayer(TEXT("1"));

		SessionNameText->SetText(FText::FromName(SessionName));
		MaxPlayersSlider->SetValue(1.0f);
		MaxPlayersText->SetText(FText::FromName(MaxPlayer));
	}
}

void USessionMenu::OnFindSessionButtonClicked()
{
	if (SessionState != ESessionState::Idle)
		return;

	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionState = ESessionState::Finding;

		int32 MaxSearchResult = 1000;
		SessionSubsystem->FindSessions(MaxSearchResult, bIsLAN);

		// LoadingThrobber active.
	}
	else
		UE_LOG(LogSessionMenu, Error, TEXT("OnFindSessionsButtonClicked() :: Can't refer SessionSubsystem."));
}

void USessionMenu::OnFindSessionsCompleted(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	ensure(SessionState == ESessionState::Finding);
	SessionState = ESessionState::Idle;
	// LoadingThrobber deactive.

	if (!bWasSuccessful)
	{
		// Validate Find Sesions Result.
		// Especially check the error type.
		// Such as Failed to find sessions, no session slot class, no available sessions, etc...
		// Show Error Msg and UI
		UE_LOG(LogSessionMenu, Warning, TEXT("OnFindSessionsCompleted() :: Failed to find sessions."));
		return;
	}

	FindSessionsResults = SearchResults;
	DisplaySessionList(FindSessionsResults);
}

void USessionMenu::DisplaySessionList(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	int32 Index = 0;

	for (const FOnlineSessionSearchResult& Result : SessionResults)
	{
		USessionSlot* SessionSlot = CreateWidget<USessionSlot>(GetWorld(), SessionSlotClass);

		FString SessionName;
		Result.Session.SessionSettings.Get(FName("NAME"), SessionName);

		int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
		int32 CurrentPlayers = MaxPlayers - Result.Session.NumOpenPublicConnections;	// 최대 인원 수 - 현재 열려있는 슬롯 수 == 현재 인원 수
		int32 Ping = Result.PingInMs;
			
		FSessionData SessionData;
		SessionData.SessionIndex = Index++;
		SessionData.SessionName = FName(SessionName);
		SessionData.MaxPlayers = MaxPlayers;
		SessionData.CurrentPlayers = CurrentPlayers;
		SessionData.Ping = Ping;


		SessionSlot->InitializeSessionData(SessionData);
		SessionScrollBox->AddChild(SessionSlot);
		SessionSlot->OnJoinButtonClickedEvent.AddUObject(this, &ThisClass::OnJoinButtonClicked);
	}
}

void USessionMenu::OnJoinButtonClicked(int32 SessionIndex)
{
	if (SessionState != ESessionState::Idle)
		return;

	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionState = ESessionState::Joining;
		FString SessionName;
		FindSessionsResults[SessionIndex].Session.SessionSettings.Get(FName("NAME"), SessionName);

		SessionSubsystem->JoinSession(FindSessionsResults[SessionIndex], FName(SessionName));
	}
}

void USessionMenu::OnJoinSessionCompleted(EOnJoinSessionCompleteResult::Type Result)
{
	SessionState = ESessionState::Idle;

	if (Result == EOnJoinSessionCompleteResult::UnknownError)
	{
		// Show Error Msg
		return;
	}
}

void USessionMenu::OnSliderValueChanged(float Value)
{
	const int32 PlayerNum = FMath::RoundToInt(Value);
	MaxPlayersText->SetText(FText::AsNumber(PlayerNum));
}

void USessionMenu::OnCheckStateChanged(bool bIsChecked)
{
	bIsLAN = bIsChecked;
}
