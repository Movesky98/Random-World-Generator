// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/TimeManagementComponent.h"
#include "GamePlay/GameFramework/ExpeditionGameState.h"

#include "CommonLogCategories.h"

#define DayStartTime 0.0f
#define NightStartTime 0.5f

// Sets default values for this component's properties
UTimeManagementComponent::UTimeManagementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bWantsInitializeComponent = true;
}

void UTimeManagementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FullDuration = DayDuration + NightDuration;
	bIsDay = TimeOfDay <= 0.5f;
}


// Called when the game starts
void UTimeManagementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GameState = Cast<AExpeditionGameState>(GetWorld()->GetGameState()))
	{
		COMMON_LOG(LogGameplay, Log, TEXT(""));
	}
	else
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Can't find ExpeditionGameState"));
	}
}

// Called every frame
void UTimeManagementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeOfDay += DeltaTime / FullDuration;
	const bool bIsNewDay = TimeOfDay <= NightStartTime;

	if (TimeOfDay >= 1.0f)
	{
		TimeOfDay -= 1.0f;
	}
	
	if (bIsDay != bIsNewDay)
	{
		bIsDay = bIsNewDay;
		
		GameState->SetDayCycle(bIsDay ? EDayCycle::Day : EDayCycle::Night);
		OnDayCycleChanged.Broadcast(bIsDay ? EDayCycle::Day : EDayCycle::Night);
	}

	ElapsedTimeUpdate += DeltaTime;
	if (ElapsedTimeUpdate >= SyncInterval)
	{
		if (GameState)
			GameState->SetTimeOfDay(TimeOfDay);
	}
}

