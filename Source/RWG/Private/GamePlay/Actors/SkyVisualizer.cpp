// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Actors/SkyVisualizer.h"
#include "GamePlay/GameFramework/ExpeditionGameState.h"
#include "CommonLogCategories.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"

// Sets default values
ASkyVisualizer::ASkyVisualizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASkyVisualizer::BeginPlay()
{
	Super::BeginPlay();
	
	if (AExpeditionGameState* GS = Cast<AExpeditionGameState>(GetWorld()->GetGameState()))
	{
		GS->OnTimeOfDayUpdated.AddUObject(this, &ThisClass::OnTimeOfDayUpdated);
		FullDuration = GS->GetFullDuration();
		FindSun();
	}
	else
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Can't find GameState."));
	}
}

// Called every frame
void ASkyVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LocalTimeOfDay += DeltaTime / FullDuration;

	LocalTimeOfDay = FMath::FInterpTo(LocalTimeOfDay, TargetTimeOfDay, DeltaTime, 2.0f);

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

