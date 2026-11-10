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

void AGunBase::Unequip()
{
	Super::Unequip();

	COMMON_LOG(LogGameplay, Log, TEXT("Gun unequipped : %s"), *GetName());
}

UAnimMontage* AGunBase::GetCharacterMontage(EWeaponActionState ActionState) const
{
	UGunData* GunData = GetItemData<UGunData>();
	if (!GunData) return nullptr;

	switch (ActionState)
	{
	case EWeaponActionState::Reload:		return GunData->CharacterReloadMontage;
	case EWeaponActionState::ReloadEmpty:	return GunData->CharacterReloadEmptyMontage;
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

	SetCurrentAmmo(CurrentAmmo - 1);
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
	return HasAmmo() && !bIsFiring;
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
	
	const FTransform MuzzleTransform(
		MeshComp->GetSocketRotation(Data->MuzzleSocketName),
		MeshComp->GetSocketLocation(Data->MuzzleSocketName)
	);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	ABulletProjectile* Bullet = GetWorld()->SpawnActorDeferred<ABulletProjectile>(
		Data->BulletProjectileClass,
		MuzzleTransform,
		this,
		InstigatorPawn,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

	if (!Bullet)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("Spawn bullet failed."));
		return false;
	}

	COMMON_LOG(LogGameplay, Log, TEXT("SpawnParam : Owner is %s, Instigator is %s"), *GetNameSafe(this), *GetNameSafe(InstigatorPawn));

	// 피해량·속도는 BeginPlay 전에 넣고, 스폰 완료 중 바로 맞아 사라지는 경우도 발사로 친다
	Bullet->InitProjectile(Data->BulletDamage, Data->BulletSpeed, Data->BulletLifeTime);
	Bullet->FinishSpawning(MuzzleTransform);

	return true;
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
