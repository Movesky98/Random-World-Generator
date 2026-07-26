// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/CharacterBase/CharacterBase.h"
#include "Gameplay/Components/HealthComponent.h"
#include "CommonLogCategories.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	FDamageInfo DamageInfo;
	DamageInfo.BaseDamage = DamageAmount;
	DamageInfo.DamageCauser = DamageCauser;
	DamageInfo.Instigator = EventInstigator->GetPawn();
	DamageInfo.DamageType = DamageEvent.DamageTypeClass;

	COMMON_LOG(LogGameplay, Log, TEXT("DamageInfo: DamageCauser is %s, Instigator is %s"), *GetNameSafe(DamageInfo.DamageCauser), *GetNameSafe(DamageInfo.Instigator));

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent& PointDamage = static_cast<const FPointDamageEvent&>(DamageEvent);
		DamageInfo.HitBoneName = PointDamage.HitInfo.BoneName;
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		DamageInfo.HitBoneName = NAME_None;
	}

	ProcessDamage(DamageInfo);
	return DamageAmount;
}

void ACharacterBase::ProcessDamage(const FDamageInfo& DamageInfo)
{
	if (!HasAuthority()) return;
	if (HealthComponent->IsDead()) return;

	HealthComponent->DecreaseHealth(DamageInfo.BaseDamage, DamageInfo);
}

