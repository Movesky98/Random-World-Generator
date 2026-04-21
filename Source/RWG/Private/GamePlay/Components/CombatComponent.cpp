// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/CombatComponent.h"
#include "GamePlay/DataAssets/CombatInputConfig.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{

}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

TSubclassOf<UBaseInputConfig> UCombatComponent::GetConfigClass()
{
	return UCombatInputConfig::StaticClass();
}

void UCombatComponent::BindInputActions(UEnhancedInputComponent* InputComponent)
{

}

