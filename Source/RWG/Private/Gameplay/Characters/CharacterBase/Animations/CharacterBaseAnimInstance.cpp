// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/CharacterBase/Animations/CharacterBaseAnimInstance.h"
#include "Gameplay/Characters/CharacterBase/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CommonLogCategories.h"

DEFINE_LOG_CATEGORY(LogCharacterAnimation);

UCharacterBaseAnimInstance::UCharacterBaseAnimInstance()
{

}

void UCharacterBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (APawn* OwnerPawn = TryGetPawnOwner())
	{
		OwnerCharacter = Cast<ACharacter>(OwnerPawn);

		if (!OwnerCharacter)
		{
			COMMON_LOG(LogGameplay, Error, TEXT("OwnerCharacter is null."));
			return;
		}

		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UCharacterBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwnerCharacter) return;

	Velocity = OwnerCharacter->GetVelocity();
	bIsInAir = MovementComponent->IsFalling();
	Speed = Velocity.Length();
	
	const bool bHasInput = !MovementComponent->GetCurrentAcceleration().IsNearlyZero();
	const bool bIsMoving = Speed > MoveThreshold;
	FRotator Rotation = OwnerCharacter->GetActorRotation();

	bShouldMove = bHasInput && bIsMoving;
	Direction = CalculateDirection(Velocity, Rotation);

	const FRotator AimRotation = OwnerCharacter->GetBaseAimRotation();
	const FRotator AimDelta = (AimRotation - Rotation).GetNormalized();

	AimYaw = AimDelta.Yaw;
	AimPitch = AimDelta.Pitch;
}