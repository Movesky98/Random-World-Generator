// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DayNightComponent.generated.h"

class ADirectionalLight;

DECLARE_MULTICAST_DELEGATE(FOnDayStarted);
DECLARE_MULTICAST_DELEGATE(FOnNightStarted);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UDayNightComponent : public UActorComponent
{
	GENERATED_BODY()

	public:
    UDayNightComponent();

    FOnDayStarted OnDayStarted;
    FOnNightStarted OnNightStarted;

    // 0 = 자정, 0.25 = 아침, 0.5 = 정오, 0.75 = 저녁
    UPROPERTY(EditAnywhere, Category = "DayNight")
    float TimeOfDay = 0.25f;

    // 하루 전체 길이 (초)
    UPROPERTY(EditAnywhere, Category = "DayNight")
    float FullDayDuration = 480.f; // 8분

    // 낮 비율 (0~1)
    UPROPERTY(EditAnywhere, Category = "DayNight")
    float DayRatio = 0.6f;

    UPROPERTY(EditAnywhere, Category = "DayNight")
    float DayDuration = 300.f; // 5분

    UPROPERTY(EditAnywhere, Category = "DayNight")
    float NightDuration = 180.f; // 3분

    bool bIsDay = true;

protected:
    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TObjectPtr<ADirectionalLight> Sun;

    void FindSun();
    void UpdateSunRotation();
    void CheckDayNightTransition(float PreviousTime);
    
    FTimerHandle DayNightTimerHandle;

    void OnDayEnd();
    void OnNightEnd();

    void StartDay();
    void StartNight();

};
