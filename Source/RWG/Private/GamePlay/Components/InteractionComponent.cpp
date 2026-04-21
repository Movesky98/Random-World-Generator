// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/InteractionComponent.h"
#include "GamePlay/DataAssets/InteractionInputConfig.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{

}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UInteractionComponent::BindInputActions(UEnhancedInputComponent* InputComponent)
{

}

TSubclassOf<UBaseInputConfig> UInteractionComponent::GetConfigClass()
{
	return UInteractionInputConfig::StaticClass();
}
