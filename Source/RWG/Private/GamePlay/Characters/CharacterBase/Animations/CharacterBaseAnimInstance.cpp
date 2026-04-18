// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Characters/CharacterBase/Animations/CharacterBaseAnimInstance.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

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
			UE_LOG(LogCharacterAnimation, Error, TEXT("[%s] %s (%d) : OwnerCharacter is null"),
				TEXT(__FUNCTION__),
				TEXT(__FILE__),
				__LINE__
			);
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
}
