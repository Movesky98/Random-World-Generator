// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/LocomotionComponent.h"
#include "Gameplay/DataAssets/LocomotionInputConfig.h"
#include "Gameplay/Characters/Convict/Animation/ConvictAnimInstance.h"
#include "CommonLogCategories.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
	/**
	 * 제자리 회전 종료 판정 오차(도).
	 * RInterpConstantTo는 목표에 도달하지만 부동소수 오차로 정확히 같아지지 않을 수 있고,
	 * 그러면 bIsTurningInPlace가 안 풀려서 다음 회전이 영영 시작되지 않는다.
	 */
	constexpr float TurnCompleteTolerance = 0.01f;
}

// Sets default values for this component's properties
ULocomotionComponent::ULocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULocomotionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("OwnerCharacter is null. LocomotionComponent must be attached to an ACharacter."));
		return;
	}

	MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("MovementComponent is null. Check the character's movement component."));
	}

	if (USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh())
	{
		AnimInstance = Cast<UConvictAnimInstance>(MeshComponent->GetAnimInstance());
	}

	if (!AnimInstance)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("AnimInstance is null. Check the mesh's Anim Class and bEnableAnimation."));
	}
}

// Called when the game starts
void ULocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

}

void ULocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTurnInPlace(DeltaTime);
}

void ULocomotionComponent::UpdateTurnInPlace(float DeltaTime)
{
	if (!OwnerCharacter || !MovementComponent || !AnimInstance) return;

	// 시뮬레이티드 프록시는 복제된 회전을 그대로 쓴다. 여기서 계산하면 복제값과 싸운다.
	if (!OwnerCharacter->HasAuthority() && !OwnerCharacter->IsLocallyControlled()) return;

	// 이동 중에는 CharacterMovement가 RotationRate로 컨트롤 회전을 따라간다.
	if (AnimInstance->ShouldMove())
	{
		MovementComponent->bUseControllerDesiredRotation = true;
		bIsTurningInPlace = false;
		return;
	}

	// 정지 중에는 몸을 고정해서 AimOffset이 상체만 돌리도록 둔다.
	MovementComponent->bUseControllerDesiredRotation = false;

	// 목표는 진입할 때 한 번만 정하고, 회전이 끝날 때까지 갱신하지 않는다.
	if (!bIsTurningInPlace && FMath::Abs(AnimInstance->GetAimYaw()) > TurnThreshold)
	{
		TargetYaw = OwnerCharacter->GetBaseAimRotation().Yaw;
		bIsTurningInPlace = true;
	}

	if (!bIsTurningInPlace) return;

	const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
	const FRotator TargetRotation(CurrentRotation.Pitch, TargetYaw, CurrentRotation.Roll);
	const FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, TurnRate);

	OwnerCharacter->SetActorRotation(NewRotation);

	// -180~180으로 감아서 재야 179도와 -179도를 먼 각도로 오해하지 않는다.
	const float RemainingYaw = FRotator::NormalizeAxis(TargetYaw - NewRotation.Yaw);
	if (FMath::Abs(RemainingYaw) <= TurnCompleteTolerance)
	{
		bIsTurningInPlace = false;
	}
}

TSubclassOf<UInputConfigBase> ULocomotionComponent::GetConfigClass()
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
	if (OwnerCharacter)
	{
		OwnerCharacter->Jump();
	}
}
