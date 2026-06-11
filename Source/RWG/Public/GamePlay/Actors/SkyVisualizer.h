// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkyVisualizer.generated.h"

class ADirectionalLight;

UCLASS()
class RWG_API ASkyVisualizer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkyVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FindSun();

	UFUNCTION()
	void OnTimeOfDayUpdated(float TimeOfDay);

	void UpdateSunRotation();

private:
	UPROPERTY(VisibleAnywhere, Category = "Time")
	float LocalTimeOfDay = 0;

	UPROPERTY(VisibleAnywhere, Category = "Time")
	float TargetTimeOfDay = 0;

	float FullDuration;

	float DayDuration;

	float NightDuration;

	TObjectPtr<ADirectionalLight> Sun;
};
