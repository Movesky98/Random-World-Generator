// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkyVisualizer.generated.h"

class ADirectionalLight;
class AExpeditionGameState;
struct FDayNightSettings;

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
	UPROPERTY()
	TObjectPtr<AExpeditionGameState> GameState;

/*********************************************************************
*                             해·달 회전
*********************************************************************/
private:
	// 낮의 방향광
	UPROPERTY(EditInstanceOnly, Category = "Sky")
	TObjectPtr<ADirectionalLight> Sun;

	// 밤의 방향광 (해의 반대편)
	UPROPERTY(EditInstanceOnly, Category = "Sky")
	TObjectPtr<ADirectionalLight> Moon;

	UPROPERTY(VisibleAnywhere, Category = "Time")
	float LocalTimeOfDay = 0;

	float FullDuration = 0.0f;

protected:
	void OnDayNightStarted(const FDayNightSettings& Settings);

	UFUNCTION()
	void OnTimeOfDayUpdated(float TimeOfDay);

	void UpdateSunMoonRotation();
};
