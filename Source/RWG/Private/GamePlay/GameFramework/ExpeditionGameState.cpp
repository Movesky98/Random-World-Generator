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
	OnCycleChanged.Broadcast(DayCycle);
}

float AExpeditionGameState::GetFullDuration() const
{
	return DayDuration + NightDuration;
}

void AExpeditionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExpeditionGameState, TimeOfDay);
	DOREPLIFETIME(AExpeditionGameState, DayCycle);
	DOREPLIFETIME(AExpeditionGameState, DayDuration);
	DOREPLIFETIME(AExpeditionGameState, NightDuration);
}

void AExpeditionGameState::OnRep_TimeOfDay()
{
	OnTimeOfDayUpdated.Broadcast(TimeOfDay);
}

void AExpeditionGameState::OnRep_DayCycle()
{
	OnCycleChanged.Broadcast(DayCycle);
}
