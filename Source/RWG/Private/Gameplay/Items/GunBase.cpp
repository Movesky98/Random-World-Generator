// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/GunBase.h"
#include "Gameplay/DataAssets/GunData.h"
#include "Gameplay/Actors/BulletProjectile.h"
#include "CommonLogCategories.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AGunBase::AGunBase()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetCollisionProfileName(TEXT("Item"));

	RootComponent = SkeletalMeshComponent;

	MagazineMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MagazineMeshComponent"));
	MagazineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MagazineMeshComponent->SetupAttachment(SkeletalMeshComponent);
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();

}

void AGunBase::ApplyItemData()
{
	UGunData* GunData = GetItemData<UGunData>();
	if (!GunData || !SkeletalMeshComponent) return;

	SkeletalMeshComponent->SetSkeletalMeshAsset(GunData->GunMesh);
	SkeletalMeshComponent->SetAnimInstanceClass(GunData->GunAnimClass);

	// 발사 모드는 서버가 기본 모드로 정하고 클라이언트는 복제된 값을 쓴다
	if (HasAuthority() && GunData->AvailableFireModes.Num() > 0)
	{
		CurrentFireMode = GunData->AvailableFireModes[0];
	}

	// 탄창이 총 메시에 포함되지 않은 총만 탄창 메시를 탄창 본에 붙인다
	if (!MagazineMeshComponent || !GunData->MagazineMesh) return;

	MagazineMeshComponent->SetSkeletalMeshAsset(GunData->MagazineMesh);
	MagazineMeshComponent->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, GunData->MagazineBoneName);
}

void AGunBase::Equip(ACharacter* NewOwner)
{
	Super::Equip(NewOwner);

	COMMON_LOG(LogGameplay, Log, TEXT("Gun equipped : %s"), *GetName());
}

void AGunBase::Drop(const FVector& DropLocation, const FRotator& DropRotation)
{
	// 재생 중이던 총 메시 몽타주(재장전 등)를 멈춘다
	if (SkeletalMeshComponent)
	{
		if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.0f);
		}
	}

	Super::Drop(DropLocation, DropRotation);

	COMMON_LOG(LogGameplay, Log, TEXT("Gun dropped : %s"), *GetName());
}

UAnimMontage* AGunBase::GetCharacterMontage(EWeaponActionState ActionState) const
{
	UGunData* GunData = GetItemData<UGunData>();
	if (!GunData) return nullptr;

	switch (ActionState)
	{
	case EWeaponActionState::Reload:		return GunData->CharacterReloadMontage;
	case EWeaponActionState::ReloadEmpty:	return GunData->CharacterReloadEmptyMontage;
	case EWeaponActionState::ReloadLoop:	return GunData->CharacterReloadLoopMontage;
	case EWeaponActionState::ReloadEnd:		return GunData->CharacterReloadEndMontage;
	default:								return Super::GetCharacterMontage(ActionState);
	}
}

void AGunBase::PlayActionMontage(EWeaponActionState ActionState)
{
	UGunData* GunData = GetItemData<UGunData>();
	if (!GunData || !SkeletalMeshComponent) return;

	UAnimMontage* Montage = nullptr;
	switch (ActionState)
	{
	case EWeaponActionState::Equip:			Montage = GunData->GunMeshEquipMontage;			break;
	case EWeaponActionState::Unequip:		Montage = GunData->GunMeshUnequipMontage;		break;
	case EWeaponActionState::Reload:		Montage = GunData->GunMeshReloadMontage;			break;
	case EWeaponActionState::ReloadEmpty:	Montage = GunData->GunMeshReloadEmptyMontage;	break;
	case EWeaponActionState::ReloadLoop:	Montage = GunData->GunMeshReloadLoopMontage;	break;
	case EWeaponActionState::ReloadEnd:		Montage = GunData->GunMeshReloadEndMontage;		break;
	default:								break;
	}
	if (!Montage) return;

	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (!AnimInstance) return;

	// 연출 전용이므로 종료 델리게이트를 걸지 않는다 (상태 머신은 캐릭터 몽타주가 굴린다)
	AnimInstance->Montage_Play(Montage);
}

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGunBase, CurrentAmmo);
	DOREPLIFETIME(AGunBase, CurrentFireMode);
}

void AGunBase::StartAttack()
{
	if (!HasAuthority()) return;
	
	if (!CanStartAttack())
	{
		if (!HasAmmo())
		{
			Multicast_PlayDryFireSound();
		}

		return;
	}

	UGunData* Data = GetItemData<UGunData>();
	if (!Data) return;
	
	bIsFiring = true;

	switch (CurrentFireMode)
	{
	case EFireMode::Auto:
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ThisClass::Fire, Data->FireRate, true);
		break;
	case EFireMode::Burst:
		BurstShotsRemaining = Data->BurstCount;
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ThisClass::Fire, Data->FireRate, true);
		break;
	case EFireMode::Semi:
		break;
	}

	Fire();
}

void AGunBase::StopAttack()
{
	if (!HasAuthority()) return;

	bIsFiring = false;
	BurstShotsRemaining = 0;

	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void AGunBase::ReleaseAttack()
{
	if (!HasAuthority()) return;

	// 점사는 입력을 떼도 남은 발수를 다 쏜 뒤 Fire()에서 멈춘다
	if (CurrentFireMode == EFireMode::Burst && BurstShotsRemaining > 0) return;

	StopAttack();
}

void AGunBase::SwitchFireMode()
{
	if (!HasAuthority() || bIsFiring) return;

	UGunData* Data = GetItemData<UGunData>();
	if (!Data || Data->AvailableFireModes.Num() < 2) return;

	// 현재 모드의 위치를 찾아 그다음 모드로 바꾸고, 마지막이면 처음으로 돌아간다
	int32 CurrentIndex = INDEX_NONE;
	for (int32 i = 0; i < Data->AvailableFireModes.Num(); i++)
	{
		if (Data->AvailableFireModes[i] == CurrentFireMode)
		{
			CurrentIndex = i;
			break;
		}
	}

	const int32 NextIndex = (CurrentIndex + 1) % Data->AvailableFireModes.Num();
	CurrentFireMode = Data->AvailableFireModes[NextIndex];

	COMMON_LOG(LogGameplay, Log, TEXT("Fire mode switched : %s"), *UEnum::GetValueAsString(CurrentFireMode));
}

void AGunBase::Fire()
{
	if (!HasAuthority()) return;

	if (!HasAmmo() || !SpawnBulletProjectile())
	{
		StopAttack();
		return;
	}

	UGunData* Data = GetItemData<UGunData>();

	SetCurrentAmmo(CurrentAmmo - 1);
	LastFireTime = GetWorld()->GetTimeSeconds();

	// 수동 장전 총은 캐릭터 발사 몽타주가 끝날 때까지 다음 발을 막는다
	bIsCycling = Data && Data->bRequiresManualCycling && Data->CharacterFireMontage;

	Multicast_PlayFireFX();
	COMMON_LOG(LogGameplay, Log, TEXT("Fire: %s"), *GetName());

	// 점사는 정해진 발수를 다 쏘면 멈춘다
	if (CurrentFireMode == EFireMode::Burst)
	{
		BurstShotsRemaining--;
		if (BurstShotsRemaining <= 0)
		{
			StopAttack();
		}
	}
}

bool AGunBase::CanStartAttack() const
{
	if (!HasAmmo() || bIsFiring || bIsCycling) return false;

	// 마지막 발사 뒤 FireRate가 지나기 전에는 새로 쏘지 않는다
	UGunData* Data = GetItemData<UGunData>();
	if (Data && LastFireTime >= 0.0 && GetWorld()->GetTimeSeconds() - LastFireTime < Data->FireRate) return false;

	return true;
}

bool AGunBase::SpawnBulletProjectile()
{
	UGunData* Data = GetItemData<UGunData>();
	if (!Data || !Data->BulletProjectileClass)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Please check GunData. Weapon : %s"), *GetName());
		return false;
	}

	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!MeshComp) return false;
	
	const FVector MuzzleLocation = MeshComp->GetSocketLocation(Data->MuzzleSocketName);
	const FVector MuzzleDirection = MeshComp->GetSocketRotation(Data->MuzzleSocketName).Vector();

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	// 한 번 쏠 때의 총 피해를 투사체 수만큼 나눈다
	const int32 PelletCount = FMath::Max(Data->PelletCount, 1);
	const float PelletDamage = Data->BulletDamage / PelletCount;
	const float SpreadHalfAngleRad = FMath::DegreesToRadians(Data->PelletSpreadAngle);

	int32 SpawnedCount = 0;
	for (int32 PelletIndex = 0; PelletIndex < PelletCount; ++PelletIndex)
	{
		// 총구 방향을 중심으로 한 원뿔 안에서 무작위 방향을 고른다
		const FVector PelletDirection = SpreadHalfAngleRad > 0.0f
			? FMath::VRandCone(MuzzleDirection, SpreadHalfAngleRad)
			: MuzzleDirection;
		const FTransform PelletTransform(PelletDirection.Rotation(), MuzzleLocation);

		ABulletProjectile* Bullet = GetWorld()->SpawnActorDeferred<ABulletProjectile>(
			Data->BulletProjectileClass,
			PelletTransform,
			this,
			InstigatorPawn,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);

		if (!Bullet)
		{
			COMMON_LOG(LogGameplay, Error, TEXT("Spawn bullet failed."));
			continue;
		}

		// 피해량·속도는 BeginPlay 전에 넣고, 스폰 완료 중 바로 맞아 사라지는 경우도 발사로 친다
		Bullet->InitProjectile(PelletDamage, Data->BulletSpeed, Data->BulletLifeTime, Data->BulletImpactData);
		Bullet->FinishSpawning(PelletTransform);
		++SpawnedCount;
	}

	COMMON_LOG(LogGameplay, Log, TEXT("SpawnParam : Owner is %s, Instigator is %s, Count : %d"), *GetNameSafe(this), *GetNameSafe(InstigatorPawn), SpawnedCount);

	return SpawnedCount > 0;
}

void AGunBase::PlayCharacterFireMontage(UAnimMontage* Montage)
{
	ACharacter* GunOwner = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInstance = GunOwner ? GunOwner->GetMesh()->GetAnimInstance() : nullptr;

	if (!AnimInstance || AnimInstance->Montage_Play(Montage) <= 0.0f)
	{
		// 몽타주를 재생하지 못하면 끝 콜백이 오지 않으므로 볼트 조작을 바로 끝낸다
		if (HasAuthority())
		{
			bIsCycling = false;
		}
		return;
	}

	if (!HasAuthority()) return;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ThisClass::OnCharacterFireMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
}

void AGunBase::OnCharacterFireMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 끝까지 재생됐든 재장전·무기 해제 몽타주에 끊겼든 볼트 조작을 끝낸 것으로 본다
	bIsCycling = false;
}

void AGunBase::SetCurrentAmmo(int32 NewAmmo)
{
	CurrentAmmo = NewAmmo;

	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, GetMagazineSize());
}

int32 AGunBase::GetLoadableAmmo(const int32 AvailableAmmo) const
{
	UGunData* GunData = GetItemData<UGunData>();
	if (!GunData) return 0;

	int32 NeededAmmo = GunData->MagazineSize - CurrentAmmo;
	return FMath::Min(NeededAmmo, AvailableAmmo);
}

int32 AGunBase::GetMagazineSize() const
{
	UGunData* Data = GetItemData<UGunData>();

	return Data ? Data->MagazineSize : INDEX_NONE;
}

EReloadCondition AGunBase::CheckReloadCondition(int32 AvailableAmmo, EWeaponActionState& OutActionState) const
{
	OutActionState = EWeaponActionState::None;

	UGunData* GunData = GetItemData<UGunData>();
	if (!GunData || !GunData->AmmoType) return EReloadCondition::InvalidData;

	int32 NeededAmmo = GunData->MagazineSize - CurrentAmmo;
	if (NeededAmmo <= 0) return EReloadCondition::MagazineFull;

	int32 ToLoad = FMath::Min(NeededAmmo, AvailableAmmo);
	if (ToLoad <= 0) return EReloadCondition::NoSpareAmmo;

	OutActionState = HasAmmo() ? EWeaponActionState::Reload : EWeaponActionState::ReloadEmpty;
	if (!GetCharacterMontage(OutActionState))
	{
		OutActionState = EWeaponActionState::None;
		return EReloadCondition::MontageMissing;
	}

	return EReloadCondition::Ready;
}

UItemData* AGunBase::GetAmmoType() const
{
	UGunData* GunData = GetItemData<UGunData>();
	return GunData ? GunData->AmmoType : nullptr;
}

bool AGunBase::LoadsOneRoundAtATime() const
{
	UGunData* GunData = GetItemData<UGunData>();
	return GunData && GunData->bLoadsOneRoundAtATime;
}

FTransform AGunBase::GetHandGuardTransform() const
{
	UGunData* GunData = GetItemData<UGunData>();
	if (!GunData) return FTransform();

	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!MeshComp) return FTransform();

	return MeshComp->GetSocketTransform(GunData->HandGuardSocketName);
}

void AGunBase::OnRep_CurrentAmmo()
{
	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, GetMagazineSize());
}

void AGunBase::Multicast_PlayDryFireSound_Implementation()
{
	UGunData* Data = GetItemData<UGunData>();
	if (!Data) return;

	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!MeshComp) return;

	if (Data->DryFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Data->DryFireSound,
			MeshComp->GetSocketLocation(Data->MuzzleSocketName)
		);
	}
}

void AGunBase::Multicast_PlayFireFX_Implementation()
{
	OnFired.Broadcast();

	UGunData* Data = GetItemData<UGunData>();
	if (!Data) return;

	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!MeshComp) return;

	if (Data->GunMeshFireMontage)
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Play(Data->GunMeshFireMontage);
		}
	}

	if (Data->CharacterFireMontage)
	{
		PlayCharacterFireMontage(Data->CharacterFireMontage);
	}

	if (Data->MuzzleFlashFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			Data->MuzzleFlashFX,
			MeshComp,
			Data->MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	if (Data->ShellEjectFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			Data->ShellEjectFX,
			MeshComp,
			Data->EjectionPortSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	if (Data->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Data->FireSound,
			MeshComp->GetSocketLocation(Data->MuzzleSocketName)
		);
	}
}
