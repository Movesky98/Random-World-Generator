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

	UFUNCTION()
	void OnRep_TimeOfDay();

	UFUNCTION()
	void OnRep_DayCycle();

	UFUNCTION()
	void OnRep_ExtractionConditions();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ExtractionConditions, Category = "Extraction|Conditions")
	TArray<FExtractionCondition> ExtractionConditions;
};
