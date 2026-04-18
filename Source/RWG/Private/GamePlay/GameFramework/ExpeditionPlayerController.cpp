// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "GamePlay/Components/InputHandlerComponent.h"

#include "EnhancedInputComponent.h"

AExpeditionPlayerController::AExpeditionPlayerController()
{
	InputHandlerComponent = CreateDefaultSubobject<UInputHandlerComponent>(TEXT("InputHandlerComponent"));
}

void AExpeditionPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

void AExpeditionPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		InputHandlerComponent->BindInputs(EnhancedInput);
	}
}
