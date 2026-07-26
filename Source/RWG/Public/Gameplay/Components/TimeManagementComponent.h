// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Enums/DayNightTypes.h"
#include "TimeManagementComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDayCycleChanged, EDayCycle /* DayCycle */);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UTimeManagementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTimeManagementComponent();

	FOnDayCycleChanged OnDayCycleChanged;

protected:
	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float TimeOfDay = 0.0f;
		
	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float SyncInterval = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Time")
	float FullDuration;

	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float DayDuration = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float NightDuration = 60.0f;

	float ElapsedTimeUpdate = 0.0f;

	bool bIsDay = true;

	TObjectPtr<class AExpeditionGameState> GameState;
};
