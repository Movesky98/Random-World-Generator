// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Actors/SkyVisualizer.h"
#include "Gameplay/GameFramework/ExpeditionGameState.h"
#include "CommonLogCategories.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"

// Sets default values
ASkyVisualizer::ASkyVisualizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;   // 낮/밤 시작 신호를 받으면 켬
}

// Called when the game starts or when spawned
void ASkyVisualizer::BeginPlay()
{
	Super::BeginPlay();

	GameState = Cast<AExpeditionGameState>(GetWorld()->GetGameState());
	if (!GameState)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Can't find GameState."));
		return;
	}

	FindSun();
	GameState->OnTimeOfDayUpdated.AddUObject(this, &ThisClass::OnTimeOfDayUpdated);
	GameState->OnDayNightStarted.AddUObject(this, &ThisClass::OnDayNightStarted);

	// 낮/밤이 이미 시작된 뒤 BeginPlay된 경우 바로 적용
	if (GameState->GetDayNightSettings().IsValid())
	{
		OnDayNightStarted(GameState->GetDayNightSettings());
	}
}

// Called every frame
void ASkyVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LocalTimeOfDay = FMath::Frac(LocalTimeOfDay + DeltaTime / FullDuration);

	// 1→0 경계를 넘을 때 최단 방향(-0.5 ~ 0.5) 차이로 보간
	float Diff = TargetTimeOfDay - LocalTimeOfDay;
	Diff -= FMath::RoundToFloat(Diff);
	LocalTimeOfDay = FMath::Frac(LocalTimeOfDay + Diff * FMath::Clamp(DeltaTime * InterpSpeed, 0.f, 1.f));

	UpdateSunRotation();
}

void ASkyVisualizer::FindSun()
{
	Sun = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ADirectionalLight::StaticClass()));

	if (!Sun)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Can't find Sun in %s Level."), *FPaths::GetBaseFilename(GetWorld()->GetMapName()))
	}
}

void ASkyVisualizer::OnDayNightStarted(const FDayNightSettings& Settings)
{
	FullDuration = Settings.GetFullDuration();
	LocalTimeOfDay = TargetTimeOfDay = GameState->GetTimeOfDay();
	UpdateSunRotation();
	SetActorTickEnabled(true);
}

void ASkyVisualizer::OnTimeOfDayUpdated(float TimeOfDay)
{
	TargetTimeOfDay = TimeOfDay;
}

void ASkyVisualizer::UpdateSunRotation()
{
	if (!Sun) return;

	float SunAngle = LocalTimeOfDay * 360.f;
	// Pitch축 기준 회전 (태양이 동→서로 호를 그림)
	FQuat SunQuat = FQuat(FVector::RightVector, FMath::DegreesToRadians(SunAngle));
	Sun->SetActorRotation(SunQuat);
}

