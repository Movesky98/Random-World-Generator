// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/RecoilComponent.h"
#include "CommonLogCategories.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"

URecoilComponent::URecoilComponent()
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void URecoilComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("OwnerCharacter is null. RecoilComponent must be attached to an ACharacter."));
	}
}

void URecoilComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsRecoilOwner()) return;
	if (AccumulatedPitch <= 0.f) return;

	APlayerController* OwnerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!OwnerController) return;

	FRotator ControlRotation = OwnerController->GetControlRotation();
	const float CurrentPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
	const float ViewDeltaPitch = FRotator::NormalizeAxis(CurrentPitch - LastKnownPitch);

	// 플레이어가 조준을 내린 만큼 누적치에서 빼되 0 밑으로는 내리지 않는다.
	if (ViewDeltaPitch < 0.f)
	{
		AccumulatedPitch = FMath::Max(0.f, AccumulatedPitch + ViewDeltaPitch);
	}

	// 남은 누적치를 넘지 않는 만큼 조준과 누적치를 같이 내린다.
	const float RecoveryStep = FMath::Min(AccumulatedPitch, RecoverySpeed * DeltaTime);

	ControlRotation.Pitch = CurrentPitch - RecoveryStep;
	OwnerController->SetControlRotation(ControlRotation);

	AccumulatedPitch -= RecoveryStep;
	LastKnownPitch = ControlRotation.Pitch;
}

bool URecoilComponent::IsRecoilOwner() const
{
	return OwnerCharacter && OwnerCharacter->IsLocallyControlled();
}

void URecoilComponent::ApplyRecoilShot()
{
	if (!IsRecoilOwner()) return;

	APlayerController* OwnerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!OwnerController) return;
	
	auto PlayerCameraManager = OwnerController->PlayerCameraManager;
	if (!PlayerCameraManager) return;

	// 현재 컨트롤 로테이션의 Pitch를 가져와 (-180, 180]으로 변환
	FRotator ControlRotation = OwnerController->GetControlRotation();
	const float CurrentPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);

	// 더 올릴 수 있는지 확인 (PitchKick 만큼)
	const float TargetPitch = FMath::ClampAngle(CurrentPitch + PitchKick, PlayerCameraManager->ViewPitchMin, PlayerCameraManager->ViewPitchMax);

	LastKnownPitch = TargetPitch;
	AccumulatedPitch += TargetPitch - CurrentPitch;

	ControlRotation.Pitch = TargetPitch;
	OwnerController->SetControlRotation(ControlRotation);
}
