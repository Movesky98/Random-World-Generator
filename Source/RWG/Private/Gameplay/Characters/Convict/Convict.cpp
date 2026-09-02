// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Convict/Convict.h"
#include "Gameplay/Components/LocomotionComponent.h"
#include "Gameplay/Components/CombatComponent.h"
#include "Gameplay/Components/InventoryComponent.h"
#include "Gameplay/Components/InteractionComponent.h"
#include "Gameplay/Components/HealthComponent.h"
#include "Gameplay/Interfaces/PlayerDetectable.h"
#include "CommonLogCategories.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "Perception/AISense_Sight.h"


AConvict::AConvict()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetMesh());
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

	ProximityDetector = CreateDefaultSubobject<USphereComponent>(TEXT("ProximityDetector"));
	ProximityDetector->SetupAttachment(RootComponent);
}

void AConvict::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ProximityDetector->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProximityBeginOverlap);
	ProximityDetector->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnProximityEndOverlap);
}

void AConvict::ProcessDamage(const FDamageInfo& DamageInfo)
{
	if (!HasAuthority()) return;
	if (HealthComponent->IsDead()) return;

	float DamageMultiplier = InventoryComponent->ProcessArmorHit(DamageInfo.HitBoneName);
	float FinalDamage = DamageInfo.BaseDamage * DamageMultiplier;

	HealthComponent->DecreaseHealth(FinalDamage, DamageInfo);
}

void AConvict::OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (IPlayerDetectable* Detectable = Cast<IPlayerDetectable>(OtherActor))
	{
		Detectable->OnPlayerApproach(this);
	}
}

void AConvict::OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	if (IPlayerDetectable* Detectable = Cast<IPlayerDetectable>(OtherActor))
	{
		Detectable->OnPlayerLeave(this);
	}
}
