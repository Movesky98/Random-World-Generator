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

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this actor's properties
	ASkyVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
private:
	TObjectPtr<ADirectionalLight> Sun;

protected:
	void FindSun();

/*********************************************************************
*                             태양 회전
*********************************************************************/
private:
	UPROPERTY(VisibleAnywhere, Category = "Time")
	float LocalTimeOfDay = 0;

	UPROPERTY(VisibleAnywhere, Category = "Time")
	float TargetTimeOfDay = 0;

	float FullDuration;

	float DayDuration;

	float NightDuration;

protected:
	UFUNCTION()
	void OnTimeOfDayUpdated(float TimeOfDay);

	void UpdateSunRotation();
};
