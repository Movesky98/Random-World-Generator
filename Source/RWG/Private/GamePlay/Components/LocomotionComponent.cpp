// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/LocomotionComponent.h"
#include "GamePlay/DataAssets/LocomotionInputConfig.h"
#include "CommonLogCategories.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
ULocomotionComponent::ULocomotionComponent()
{

}

// Called when the game starts
void ULocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

}

TSubclassOf<UBaseInputConfig> ULocomotionComponent::GetConfigClass()
{
	return ULocomotionInputConfig::StaticClass();
}

void ULocomotionComponent::BindInputActions(UEnhancedInputComponent* InputComponent)
{
	ULocomotionInputConfig* LocomotionConfig = Cast<ULocomotionInputConfig>(LoadedConfig);

	if (!InputComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InputComponent is null. SetupInputComponent may not have been called."));
		return;
	}

	if (!LocomotionConfig)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("LocomotionConfig is null. Check Asset Manager or config assignment."));
		return;
	}

	InputComponent->BindAction(LocomotionConfig->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);

	InputComponent->BindAction(LocomotionConfig->LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);

	InputComponent->BindAction(LocomotionConfig->JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
}

void ULocomotionComponent::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		// find out which way is forward
		const FRotator Rotation = OwnerPawn->GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		OwnerPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		OwnerPawn->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ULocomotionComponent::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->AddControllerYawInput(LookVector.X);
		OwnerPawn->AddControllerPitchInput(-LookVector.Y);
	}
}

void ULocomotionComponent::Jump()
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		OwnerCharacter->Jump();
	}
}
