// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionGameState.h"
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

	for (auto& Condition : ExtractionConditions)
	{
		if (Condition.ItemID == ItemID)
		{
			Condition.CurrentQuantity += Quantity;
			OnExtractionConditionsUpdated.Broadcast();

			if (Condition.IsSatisfied())
			{
				COMMON_LOG(LogGameplay, Warning, TEXT("Extraction Conditions Satisfied: %s"), *ItemID.ToString());
			}

			COMMON_LOG(LogGameplay, Warning, TEXT("Extraction Condition %s is updated. Now we have %d %s."), *ItemID.ToString(), Condition.CurrentQuantity, *ItemID.ToString());
		}
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
