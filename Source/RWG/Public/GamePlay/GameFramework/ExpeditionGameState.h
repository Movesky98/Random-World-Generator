// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GamePlay/Enums/DayNightTypes.h"
#include "GamePlay/Data/ExtractionConditionRow.h"
#include "ExpeditionGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayUpdated, float /* TimeOfDay */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDayCycleChanged, EDayCycle /* DayCycle */);
DECLARE_MULTICAST_DELEGATE(FOnExtractionConditionsUpdated);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameplayStateChanged, EGameplayState /* GameplayState */);
DECLARE_MULTICAST_DELEGATE(FOnGameOver);

UENUM(BlueprintType)
enum class EGameplayState : uint8
{
	GeneratingWorld,		   // 월드 생성 중
	WaitingForPlayers,		   // 플레이어 입장 대기 중
	Preparing,				   // 게임 시작 준비 중
	Playing,				   // 게임 시작
	GameOver,				   // 게임 종료
};

/**
 * 
 */
UCLASS()
class RWG_API AExpeditionGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	FOnTimeOfDayUpdated OnTimeOfDayUpdated;

	FOnDayCycleChanged OnDayCycleChanged;

	FOnExtractionConditionsUpdated OnExtractionConditionsUpdated;

	FOnGameplayStateChanged OnGameplayStateChanged;

	FOnGameOver OnGameOver;

	void SetTimeOfDay(float InTimeOfDay);
	void SetDayCycle(EDayCycle InDayCycle);

	float GetFullDuration() const;

	void SetExtractionConditions(const TArray<FExtractionCondition>& Conditions);
	const TArray<FExtractionCondition>& GetExtractionConditions() const;
	void UpdateExtractionProgress(FName ItemID, int32 Quantity);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_DayCycle, Category = "Time")
	EDayCycle DayCycle;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_TimeOfDay, Category = "Time")
	float TimeOfDay;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Time")
	float DayDuration = 0.0f;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Time")
	float NightDuration = 0.0f;

	// 게임이 시작되는 서버 시간, GS->GetServerWorldTimeSeconds()로 비교할 것.
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Gameplay")
	float GameStartTime = 0.f;

	bool bAllExtractionConditionsSatisfied = false;

	UFUNCTION()
	void OnRep_TimeOfDay();

	UFUNCTION()
	void OnRep_DayCycle();

	UFUNCTION()
	void OnRep_ExtractionConditions();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ExtractionConditions, Category = "Extraction|Conditions")
	TArray<FExtractionCondition> ExtractionConditions;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GameplayState, Category = "Gameplay")
	EGameplayState GameplayState;

	UFUNCTION()
	void OnRep_GameplayState(EGameplayState OldState);

public:
	void SetGameplayState(const EGameplayState State)
	{
		if (!HasAuthority()) return;

		EGameplayState OldState = GameplayState;
		GameplayState = State;
		OnRep_GameplayState(OldState);
	}
	
	EGameplayState GetGameplayState() const
	{
		return GameplayState;
	}

	void SetGameStartTime(float Time)
	{
		if (!HasAuthority()) return;
		GameStartTime = Time;
	}

	bool IsGameStart() const
	{
		return GameplayState >= EGameplayState::Playing;
	}

	bool IsGameOver() const
	{
		return GameplayState == EGameplayState::GameOver;
	}

};
