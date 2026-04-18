// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Characters/CharacterBase/CharacterBase.h"

#include "GamePlay/Components/LocomotionComponent.h"
#include "GamePlay/Components/CombatComponent.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
	LocomotionComponent = CreateDefaultSubobject<ULocomotionComponent>(TEXT("LocomotionComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

