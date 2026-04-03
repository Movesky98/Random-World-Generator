// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionSubsystem.generated.h"

RWG_API DECLARE_LOG_CATEGORY_EXTERN(LogSessionSubsystem, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCreateSessionCompletedEvent, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFindSessionsCompletedEvent, const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinSessionCompletedEvent, EOnJoinSessionCompleteResult::Type Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDestorySessionCompletedEvent, bool bWasSuccessful);

USTRUCT(BlueprintType)
struct FLobbySessionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLAN;

public:
	FLobbySessionInfo() : SessionName(FName("")), MaxPlayers(-1), bIsLAN(false)
	{ }
};

/**
 * 
 */
UCLASS()
class RWG_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	USessionSubsystem();

	void CreateSession(int32 NumPublicConnections, FName SessionName, bool IsLanMatch);

	void FindSessions(int32 MaxSearchResult, bool bIsLANQuery);

	void JoinSession(const FOnlineSessionSearchResult& SessionResult, FName SessionName);

	void DestroySession();

	/* 외부 오브젝트에게 Session 결과를 알려주는 Session Subsystem 내부 델리게이트 */
	FOnCreateSessionCompletedEvent OnCreateSessionCompletedEvent;

	FOnFindSessionsCompletedEvent OnFindSessionsCompletedEvent;

	FOnJoinSessionCompletedEvent OnJoinSessionsCompletedEvent;

	FOnDestorySessionCompletedEvent OnDestorySessionCompletedEvent;

	const FLobbySessionInfo GetLobbySessionInfo() const;

protected:
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);

	void OnFindSessionsCompleted(bool bWasSuccessful);

	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful);

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	FName LastSessionName;

	/* 로비 입장 시 현재 세션의 정보*/
	FLobbySessionInfo LobbySessionInfo;

	/* Online Subsystem 델리게이트 */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	bool TryTravelToCurrentSession(const FName SessionName);
};
