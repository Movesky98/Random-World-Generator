// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionSubsystem.generated.h"

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

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	USessionSubsystem();

/*********************************************************************
*                             세션 상태
*********************************************************************/
protected:
	FName LastSessionName;

	/* 로비 입장 시 현재 세션의 정보*/
	FLobbySessionInfo LobbySessionInfo;

public:
	const FLobbySessionInfo GetLobbySessionInfo() const;

/*********************************************************************
*                             세션 생성
*********************************************************************/
protected:
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);

public:
	void CreateSession(int32 NumPublicConnections, FName SessionName, bool IsLanMatch);

	/* 외부 오브젝트에게 Session 결과를 알려주는 Session Subsystem 내부 델리게이트 */
	FOnCreateSessionCompletedEvent OnCreateSessionCompletedEvent;

/*********************************************************************
*                             세션 검색
*********************************************************************/
protected:
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	void OnFindSessionsCompleted(bool bWasSuccessful);

public:
	void FindSessions(int32 MaxSearchResult, bool bIsLANQuery);

	FOnFindSessionsCompletedEvent OnFindSessionsCompletedEvent;

/*********************************************************************
*                             세션 참가
*********************************************************************/
protected:
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	bool TryTravelToCurrentSession(const FName SessionName);

public:
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	FOnJoinSessionCompletedEvent OnJoinSessionsCompletedEvent;

/*********************************************************************
*                             세션 종료
*********************************************************************/
protected:
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful);

public:
	void DestroySession();

	FOnDestorySessionCompletedEvent OnDestorySessionCompletedEvent;
};
