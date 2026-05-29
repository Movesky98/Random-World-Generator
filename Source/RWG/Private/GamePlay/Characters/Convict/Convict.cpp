// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Characters/Convict/Convict.h"
#include "GamePlay/Components/LocomotionComponent.h"
#include "GamePlay/Components/CombatComponent.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/Components/InteractionComponent.h"
#include "GamePlay/Components/HealthComponent.h"
#include "CommonLogCategories.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "Perception/AISense_Sight.h"


AConvict::AConvict()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 200.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	LocomotionComponent = CreateDefaultSubobject<ULocomotionComponent>(TEXT("LocomotionComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	StimuliSourceComponent->RegisterForSense(TSubclassOf<UAISense_Sight>());
	StimuliSourceComponent->RegisterWithPerceptionSystem();
}

void AConvict::ProcessDamage(const FDamageInfo& DamageInfo)
{
	if (!HasAuthority()) return;
	if (HealthComponent->IsDead()) return;

	float DamageMultiplier = InventoryComponent->ProcessArmorHit(DamageInfo.HitBoneName);
	float FinalDamage = DamageInfo.BaseDamage * DamageMultiplier;

	HealthComponent->DecreaseHealth(FinalDamage, DamageInfo);
}
