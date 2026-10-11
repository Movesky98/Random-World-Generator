// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/RecoilComponent.h"
#include "Gameplay/Items/GunBase.h"
#include "Gameplay/DataAssets/GunData.h"
#include "CommonLogCategories.h"

#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Curves/CurveFloat.h"
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

	const FRecoilSettings* RecoilSettings = GetRecoilSettings();
	if (!RecoilSettings) return;

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

	// 마지막 발 뒤 지연 시간이 지나기 전에는 복원하지 않는다.
	if (GetWorld()->GetTimeSeconds() - LastShotTime < RecoilSettings->RecoveryDelay)
	{
		LastKnownPitch = CurrentPitch;
		return;
	}

	// 남은 누적치를 넘지 않는 만큼 조준과 누적치를 같이 내린다.
	const float RecoveryStep = FMath::Min(AccumulatedPitch, RecoilSettings->RecoverySpeed * DeltaTime);

	ControlRotation.Pitch = CurrentPitch - RecoveryStep;
	OwnerController->SetControlRotation(ControlRotation);

	AccumulatedPitch -= RecoveryStep;
	LastKnownPitch = ControlRotation.Pitch;
}

void URecoilComponent::SetWeapon(AWeaponBase* NewWeapon)
{
	if (Gun)
	{
		Gun->OnFired.RemoveAll(this);
	}

	Gun = Cast<AGunBase>(NewWeapon);

	if (Gun)
	{
		Gun->OnFired.AddUObject(this, &ThisClass::OnGunFired);
	}

	// 이전 총의 반동은 복원하지 않고 버린다.
	AccumulatedPitch = 0.f;
	ShotCount = 0;
}

void URecoilComponent::OnGunFired()
{
	ApplyRecoilShot();
}

const FRecoilSettings* URecoilComponent::GetRecoilSettings() const
{
	if (!Gun) return nullptr;

	const UGunData* GunData = Gun->GetItemData<UGunData>();
	return GunData ? &GunData->Recoil : nullptr;
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

	const FRecoilSettings* RecoilSettings = GetRecoilSettings();
	if (!RecoilSettings) return;

	if (!RecoilSettings->PitchKickCurve)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("PitchKickCurve is null. Recoil is not applied."));
		return;
	}

	// 마지막 발 뒤 오래 쉬었으면 새 연사로 보고 처음부터 센다.
	const double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastShotTime >= RecoilSettings->ShotCountResetDelay)
	{
		ShotCount = 0;
	}
	++ShotCount;

	const float PitchKick = RecoilSettings->PitchKickCurve->GetFloatValue(ShotCount);

	// 현재 컨트롤 로테이션의 Pitch를 가져와 (-180, 180]으로 변환
	FRotator ControlRotation = OwnerController->GetControlRotation();
	const float CurrentPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);

	// 더 올릴 수 있는지 확인 (PitchKick 만큼)
	const float TargetPitch = FMath::ClampAngle(CurrentPitch + PitchKick, PlayerCameraManager->ViewPitchMin, PlayerCameraManager->ViewPitchMax);

	LastKnownPitch = TargetPitch;
	LastShotTime = CurrentTime;

	// 이번 발의 킥만 복원 대상으로 남긴다. 복원이 끝나면 마지막 발이 나간 조준으로 돌아온다.
	AccumulatedPitch = TargetPitch - CurrentPitch;

	ControlRotation.Pitch = TargetPitch;
	OwnerController->SetControlRotation(ControlRotation);

	if (RecoilSettings->FireCameraShake)
	{
		PlayerCameraManager->StartCameraShake(RecoilSettings->FireCameraShake);
	}
}
