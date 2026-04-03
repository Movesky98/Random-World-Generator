// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/GameFramework/SessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"

DEFINE_LOG_CATEGORY(LogSessionSubsystem);

USessionSubsystem::USessionSubsystem() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompleted)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompleted)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionCompleted))

{

}

void USessionSubsystem::CreateSession(int32 NumPublicConnections, FName SessionName, bool IsLanMatch)
{
	IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
	if (!OSS)
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("OSS is nullptr."));
	}
	
	FName OSSName = OSS->GetSubsystemName();
	UE_LOG(LogSessionSubsystem, Warning, TEXT("OSS Name : %s"), *OSSName.ToString());

	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface)
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("CreateSession() :: There's no session interface."));
		OnCreateSessionCompletedEvent.Broadcast(false);
		return;
	}

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bIsLANMatch = IsLanMatch;
	LastSessionSettings->bShouldAdvertise = true;	// 세션 검색 가능 여부
	
	LastSessionSettings->NumPrivateConnections = 0;						// 초대 전용 슬롯
	LastSessionSettings->NumPublicConnections = NumPublicConnections;	// 공개 슬롯 (검색해서 입장할 수 있는 자리)
	
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	
	LastSessionSettings->bUsesPresence = true;						// Presence 시스템 사용 여부
	LastSessionSettings->bAllowJoinViaPresence = true;				// 친구 목록(Presence 기반)을 통해 입장 가능 여부
	LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;

	/* 메타 데이터 추가 */
	LastSessionSettings->Set(FName("SESSION_NAME"), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), SessionName, *LastSessionSettings))
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("CreateSession() :: Failed to create session."));
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnCreateSessionCompletedEvent.Broadcast(false);
	}
}

void USessionSubsystem::FindSessions(int32 MaxSearchResult, bool bIsLANQuery)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface)
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("FindSessions() :: There's no session interface."));
		OnFindSessionsCompletedEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResult;
	LastSessionSearch->bIsLanQuery = bIsLANQuery;

	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("FindSessions() :: Failed to find session."));
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindSessionsCompletedEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}

}

void USessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult, FName SessionName)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface)
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("JoinSession() :: There's no session interface."));
		OnJoinSessionsCompletedEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), SessionName, SessionResult))
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("JoinSession() :: Failed to join session."));
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnJoinSessionsCompletedEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void USessionSubsystem::DestroySession()
{
}

const FLobbySessionInfo USessionSubsystem::GetLobbySessionInfo() const
{
	return LobbySessionInfo;
}

void USessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("OnCreateSessionCompleted() :: Failed to create session."));
		OnCreateSessionCompletedEvent.Broadcast(false);
		return;
	}

	UE_LOG(LogSessionSubsystem, Warning, TEXT("OnCreateSessionCompleted() :: Session created successfully."));
	LastSessionName = SessionName;
	OnCreateSessionCompletedEvent.Broadcast(true);

	LobbySessionInfo.SessionName = SessionName;
	LobbySessionInfo.MaxPlayers = LastSessionSettings->NumPublicConnections;
	LobbySessionInfo.bIsLAN = LastSessionSettings->bIsLANMatch;

	UWorld* World = GetWorld();
	if (World)
	{
		World->ServerTravel("/Game/Features/Session/Levels/LV_Lobby?listen");
	}
}

void USessionSubsystem::OnFindSessionsCompleted(bool bWasSuccessful)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (!bWasSuccessful || LastSessionSearch->SearchResults.Num() == 0)
	{
		OnFindSessionsCompletedEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), bWasSuccessful);
		return;
	}

	UE_LOG(LogSessionSubsystem, Warning, TEXT("OnFindSessionsCompleted() :: Sessions found successfully."));
	OnFindSessionsCompletedEvent.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);

}

void USessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	if (Result == EOnJoinSessionCompleteResult::UnknownError)
	{
		UE_LOG(LogSessionSubsystem, Warning, TEXT("OnJoinSessionCompleted() :: Failed to join session."));
		OnJoinSessionsCompletedEvent.Broadcast(Result);
		return;
	}

	LastSessionName = SessionName;
	LastSessionSettings = MakeShareable(SessionInterface->GetSessionSettings(LastSessionName));

	OnJoinSessionsCompletedEvent.Broadcast(Result);

	// TryTravel
	TryTravelToCurrentSession(SessionName);
}

void USessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{

}

bool USessionSubsystem::TryTravelToCurrentSession(const FName SessionName)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface)
	{
		return false;
	}

	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		return false;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	return true;
}