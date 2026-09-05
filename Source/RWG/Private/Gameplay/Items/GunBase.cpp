// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/GunBase.h"
#include "Gameplay/DataAssets/GunData.h"
#include "Gameplay/Actors/BulletProjectile.h"
#include "CommonLogCategories.h"

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

}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();

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
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ThisClass::Fire, Data->FireRate, true);

	Fire();
}

void AGunBase::StopAttack()
{
	if (!HasAuthority()) return;

	bIsFiring = false;

	GetWorldTimerManager().ClearTimer(FireTimerHandle);
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
	
	const FVector MuzzleLocation = MeshComp->GetSocketLocation(Data->MuzzleSocketName);
	const FRotator MuzzleRotation = MeshComp->GetSocketRotation(Data->MuzzleSocketName);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABulletProjectile* Bullet = GetWorld()->SpawnActor<ABulletProjectile>(
		Data->BulletProjectileClass,
		MuzzleLocation,
		MuzzleRotation,
		SpawnParams
		);

	if (!Bullet)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("Spawn bullet failed."));
		return false;
	}


	COMMON_LOG(LogGameplay, Log, TEXT("SpawnParam : Owner is %s, Instigator is %s"), *GetNameSafe(SpawnParams.Owner), *GetNameSafe(SpawnParams.Instigator));
	Bullet->InitProjectile(Data->BulletDamage, Data->BulletSpeed, Data->BulletLifeTime);

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
	UGunData* Data = GetItemData<UGunData>();
	if (!Data) return;

	USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
	if (!MeshComp) return;

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
