// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/GameFramework/ExpeditionGameState.h"
#include "CommonLogCategories.h"
#include "Net/UnrealNetwork.h"

void AExpeditionGameState::SetTimeOfDay(float InTimeOfDay)
{
	TimeOfDay = InTimeOfDay;
	OnTimeOfDayUpdated.Broadcast(TimeOfDay);
}

void AExpeditionGameState::SetDayCycle(EDayCycle InDayCycle)
{
	DayCycle = InDayCycle;
	COMMON_LOG(LogGameplay, Log, TEXT("%s"), InDayCycle == EDayCycle::Day ? TEXT("Now Day") : TEXT("Now Night"));
	OnDayCycleChanged.Broadcast(DayCycle);
}

float AExpeditionGameState::GetFullDuration() const
{
	return DayDuration + NightDuration;
}

void AExpeditionGameState::SetExtractionConditions(const TArray<FExtractionCondition>& Conditions)
{
	if (!HasAuthority())
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Server authority required."));
		return;
	}

	if (Conditions.Num())
	{
		ExtractionConditions = Conditions;
		OnExtractionConditionsUpdated.Broadcast();
	}
}

const TArray<FExtractionCondition>& AExpeditionGameState::GetExtractionConditions() const
{
	return ExtractionConditions;
}

void AExpeditionGameState::UpdateExtractionProgress(FName ItemID, int32 Quantity)
{
	if (!HasAuthority())
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Server authority required."));
		return;
	}

	bAllExtractionConditionsSatisfied = true;

	for (auto& Condition : ExtractionConditions)
	{
		if (Condition.ItemID == ItemID)
		{
			Condition.CurrentQuantity += Quantity;
			OnExtractionConditionsUpdated.Broadcast();

			if (Condition.IsSatisfied())
			{
				COMMON_LOG(LogGameplay, Warning, TEXT("Extraction Conditions Satisfied: %s"), *ItemID.ToString());
				continue;
			}

			COMMON_LOG(LogGameplay, Warning, TEXT("Extraction Condition %s is updated. Now we have %d %s."), *ItemID.ToString(), Condition.CurrentQuantity, *ItemID.ToString());
			bAllExtractionConditionsSatisfied = false;
		}
	}

	// 일단 바로 GameOver
	if (bAllExtractionConditionsSatisfied)
	{
		SetGameplayState(EGameplayState::GameOver);
	}
}

void AExpeditionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExpeditionGameState, TimeOfDay);
	DOREPLIFETIME(AExpeditionGameState, DayCycle);
	DOREPLIFETIME(AExpeditionGameState, DayDuration);
	DOREPLIFETIME(AExpeditionGameState, NightDuration);
	DOREPLIFETIME(AExpeditionGameState, ExtractionConditions);
	DOREPLIFETIME(AExpeditionGameState, GameplayState);
	DOREPLIFETIME(AExpeditionGameState, GameStartTime);
}

void AExpeditionGameState::OnRep_TimeOfDay()
{
	OnTimeOfDayUpdated.Broadcast(TimeOfDay);
}

void AExpeditionGameState::OnRep_DayCycle()
{
	OnDayCycleChanged.Broadcast(DayCycle);
}

void AExpeditionGameState::OnRep_ExtractionConditions()
{
	OnExtractionConditionsUpdated.Broadcast();
}

void AExpeditionGameState::OnRep_GameplayState(EGameplayState OldState)
{
	if (OldState == GameplayState) return;

	OnGameplayStateChanged.Broadcast(GameplayState);
	
	COMMON_LOG(LogGameplay, Warning, TEXT("[OnRep] %s -> %s, GameStartTime=%.2f, Now=%.2f"),
		*UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(GameplayState),
		GameStartTime, GetServerWorldTimeSeconds());

	if (OldState == EGameplayState::WaitingForPlayers && GameplayState == EGameplayState::Preparing)
	{
		// 이 시점이 클라 입장에서 게임이 시작되기 위한 타이머가 시작되는 순간

	}

	if (OldState == EGameplayState::Playing && GameplayState == EGameplayState::GameOver)
	{
		OnGameOver.Broadcast();
	}
}
