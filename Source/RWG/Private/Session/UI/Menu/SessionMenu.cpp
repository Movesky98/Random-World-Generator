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
#include "Components/CircularThrobber.h"

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
	if (!ensure(LoadingThrobber != nullptr)) return;
	if (!ensure(MaxPlayersText != nullptr)) return;
	if (!ensure(SessionSlotClass != nullptr)) return;
 
	MaxPlayersSlider->OnValueChanged.AddDynamic(this, &ThisClass::OnSliderValueChanged);
	CreateSessionButton->OnClicked.AddDynamic(this, &ThisClass::OnCreateSessionButtonClicked);
	FindSessionsButton->OnClicked.AddDynamic(this, &ThisClass::OnFindSessionButtonClicked);
	LANCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnCheckStateChanged);
	
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);

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

	SessionState = ESessionState::Idle;
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
	FString SessionNameStr = SessionNameText->GetText().ToString().TrimStartAndEnd();

	if (SessionNameStr.IsEmpty() || SessionState != ESessionState::Idle)
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
		HandleError(ESessionUIError::CreateFailed);

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
		LoadingThrobber->SetVisibility(ESlateVisibility::Visible);
	}
	else
		UE_LOG(LogSessionMenu, Error, TEXT("OnFindSessionsButtonClicked() :: Can't refer SessionSubsystem."));
}

void USessionMenu::OnFindSessionsCompleted(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	ensure(SessionState == ESessionState::Finding);

	if (!bWasSuccessful)
	{
		HandleError(ESessionUIError::FindFailed);
		// Validate Find Sesions Result.
		// Especially check the error type.
		// Such as Failed to find sessions, no session slot class, no available sessions, etc...
		// Show Error Msg and UI
		UE_LOG(LogSessionMenu, Warning, TEXT("OnFindSessionsCompleted() :: Failed to find sessions."));
		return;
	}

	SessionState = ESessionState::Idle;
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);

	FindSessionsResults = SearchResults;

	SessionScrollBox->ClearChildren();	// 아마 SessionSlot 생성 시 바인딩했던 델리게이트를 해제해줘야할 필요가 있을 것으로 판단함.

	DisplaySessionList(FindSessionsResults);
}

void USessionMenu::DisplaySessionList(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	int32 Index = 0;

	for (const FOnlineSessionSearchResult& Result : SessionResults)
	{
		if (USessionSlot* SessionSlot = CreateWidget<USessionSlot>(GetWorld(), SessionSlotClass))
		{
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
}

void USessionMenu::OnJoinButtonClicked(int32 SessionIndex)
{
	if (SessionState != ESessionState::Idle || !FindSessionsResults.IsValidIndex(SessionIndex))
	{
		UE_LOG(LogSessionMenu, Error, TEXT("OnJoinButtonClicked() :: Invalid Input: State=[%s], Index=[%d]"), *UEnum::GetValueAsString(SessionState), SessionIndex);
		return;
	}

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
	// Join에 성공하면 레벨 이동 → SessionMenu는 할 일 없음
	// 여기서는 오류 처리 담당

	if (Result == EOnJoinSessionCompleteResult::UnknownError)
	{
		HandleError(ESessionUIError::JoinFailed);
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

void USessionMenu::ResetUIToIdle()
{
	SessionState = ESessionState::Idle;
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
}

void USessionMenu::HandleError(ESessionUIError Error)
{
	ResetUIToIdle();

	switch (Error)
	{
	case ESessionUIError::InvalidState:
		break;
	case ESessionUIError::SessionInterfaceNULL:
		break;
	case ESessionUIError::CreateFailed:
		break;
	case ESessionUIError::FindFailed:
		break;
	case ESessionUIError::JoinFailed:
		break;
	default:
		break;
	}
}
