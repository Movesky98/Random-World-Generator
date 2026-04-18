// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/LocomotionComponent.h"
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

void ULocomotionComponent::Move(const FVector2D& InputValue)
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		// find out which way is forward
		const FRotator Rotation = OwnerPawn->GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		OwnerPawn->AddMovementInput(ForwardDirection, InputValue.Y);
		OwnerPawn->AddMovementInput(RightDirection, InputValue.X);
	}
}

void ULocomotionComponent::Look(const FVector2D& InputValue)
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->AddControllerYawInput(InputValue.X);
		OwnerPawn->AddControllerPitchInput(-InputValue.Y);
	}
}

void ULocomotionComponent::Jump()
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		OwnerCharacter->Jump();
	}
}
