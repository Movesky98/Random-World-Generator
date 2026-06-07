// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/DayNightComponent.h"
#include "CommonLogCategories.h"

#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"


UDayNightComponent::UDayNightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UDayNightComponent::BeginPlay()
{
    Super::BeginPlay();

    FindSun();
    UpdateSunRotation();

    if (!GetOwner()->HasAuthority()) return;

    StartDay();
}

void UDayNightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float PreviousTime = TimeOfDay;

    TimeOfDay += DeltaTime / FullDayDuration;
    if (TimeOfDay >= 1.f)
        TimeOfDay -= 1.f;

    UpdateSunRotation();

    if (GetOwner()->HasAuthority())
        CheckDayNightTransition(PreviousTime);
}

void UDayNightComponent::StartDay()
{
    bIsDay = true;
    OnDayStarted.Broadcast();

    GetWorld()->GetTimerManager().SetTimer(
        DayNightTimerHandle,
        this,
        &UDayNightComponent::OnDayEnd,
        DayDuration,
        false
    );
}

void UDayNightComponent::StartNight()
{
    bIsDay = false;
    OnNightStarted.Broadcast();

    GetWorld()->GetTimerManager().SetTimer(
        DayNightTimerHandle,
        this,
        &UDayNightComponent::OnNightEnd,
        NightDuration,
        false
    );
}

void UDayNightComponent::OnDayEnd()
{
    StartNight();
}

void UDayNightComponent::OnNightEnd()
{
    StartDay();
}

void UDayNightComponent::FindSun()
{
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), Lights);

    if (Lights.Num() > 0)
        Sun = Cast<ADirectionalLight>(Lights[0]);
}

void UDayNightComponent::UpdateSunRotation()
{
    if (!Sun) return;

    // 0 = 수평선(일출), -90 = 정오, -180 = 수평선(일몰)
    // TimeOfDay 0~1을 0 ~ -360으로 매핑
    float SunAngle = TimeOfDay * -360.f;
    Sun->SetActorRotation(FRotator(SunAngle, 0.f, 0.f));
}

void UDayNightComponent::CheckDayNightTransition(float PreviousTime)
{
    float DayStart = 0.f;    // 일출
    float NightStart = 0.5f; // 일몰

    // 낮 → 밤
    if (PreviousTime < NightStart && TimeOfDay >= NightStart)
    {
        bIsDay = false;
        COMMON_LOG(LogGameplay, Warning, TEXT("Night Started: %.2f"), TimeOfDay);
        OnNightStarted.Broadcast();
    }

    // 밤 → 낮 (TimeOfDay가 0을 넘어갈 때)
    if (PreviousTime > TimeOfDay) // 1.0 → 0.0 넘어가는 순간
    {
        bIsDay = true;
        COMMON_LOG(LogGameplay, Warning, TEXT("Day Started: %.2f"), TimeOfDay);
        OnDayStarted.Broadcast();
    }
}