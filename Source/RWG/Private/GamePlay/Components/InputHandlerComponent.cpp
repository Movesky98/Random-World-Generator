// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/InputHandlerComponent.h"
#include "GamePlay/DataAssets/BaseInputConfig.h"
#include "GamePlay/Components/LocomotionComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

// Sets default values for this component's properties
UInputHandlerComponent::UInputHandlerComponent()
{

}

// Called when the game starts
void UInputHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UInputHandlerComponent::BindInputs(UEnhancedInputComponent* EnhancedInputComponent)
{
	OwnerController = Cast<APlayerController>(GetOwner());
	checkf(OwnerController, TEXT("InputHandlerComponent must be attached to PlayerController"));

	if (!IsValid(BaseInputConfig))
	{
		// Display error log.
		return;
	}

	if (ULocalPlayer* LocalPlayer = OwnerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			// ÀÏ´Ü BaseInput
			Subsystem->AddMappingContext(BaseInputConfig->MappingContext, 0);
		}
	}

	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(BaseInputConfig->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMove);

		EnhancedInputComponent->BindAction(BaseInputConfig->LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLook);

		EnhancedInputComponent->BindAction(BaseInputConfig->JumpAction, ETriggerEvent::Triggered, this, &ThisClass::HandleJump);
	}
}

ULocomotionComponent* UInputHandlerComponent::GetLocomotionComponent()
{
	APawn* Pawn = OwnerController ? OwnerController->GetPawn() : nullptr;
	if (!Pawn)
	{
		return nullptr;
	}

	if (ULocomotionComponent* LocomotionComp = Pawn->FindComponentByClass<ULocomotionComponent>())
	{
		return LocomotionComp;
	}

	return nullptr;
}

void UInputHandlerComponent::HandleMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (ULocomotionComponent* LocomotionComp = GetLocomotionComponent())
	{
		LocomotionComp->Move(MovementVector);
	}
}

void UInputHandlerComponent::HandleLook(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (ULocomotionComponent* LocomotionComp = GetLocomotionComponent())
	{
		LocomotionComp->Look(MovementVector);
	}
}

void UInputHandlerComponent::HandleJump()
{
	if (ULocomotionComponent* LocomotionComp = GetLocomotionComponent())
	{
		LocomotionComp->Jump();
	}
}

void UInputHandlerComponent::HandleAttack()
{

}

void UInputHandlerComponent::HandleInteract()
{

}
