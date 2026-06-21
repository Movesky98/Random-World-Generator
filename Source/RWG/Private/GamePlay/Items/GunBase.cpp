// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Items/GunBase.h"
#include "GamePlay/Items/GunData.h"
#include "GamePlay/Actors/BulletProjectile.h"
#include "CommonLogCategories.h"
#include "Net/UnrealNetwork.h"

AGunBase::AGunBase()
{
	
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();

}

void AGunBase::Equip(ACharacter* NewOwner)
{
	Super::Equip(NewOwner);

	// TODO : 장착 몽타주 재생 (WeaponData->EquipMontage)
	COMMON_LOG(LogGameplay, Log, TEXT("Gun equipped : %s"), *GetName());
}

void AGunBase::Unequip()
{
	Super::Unequip();

	// TODO : 해제 몽타주 재생 (WeaponData->UnequipMontage)
	COMMON_LOG(LogGameplay, Log, TEXT("Gun unequipped : %s"), *GetName());
}

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGunBase, CurrentAmmo);
}

void AGunBase::StartAttack()
{
	if (!HasAuthority()) return;

	if (!CanFire()) return;

	UGunData* Data = GetItemData<UGunData>();
	if (!Data) return;
	
	bIsFiring = true;

	Fire();
	
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ThisClass::Fire, Data->FireRate, true);
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

	if (!CanFire())
	{
		StopAttack();
		return;
	}

	COMMON_LOG(LogGameplay, Log, TEXT("Fire: %s"), *GetName());
	SpawnBulletProjectile();
	SetCurrentAmmo(CurrentAmmo - 1);
}

bool AGunBase::CanFire() const
{
	if (CurrentAmmo <= 0) return false;
	if (bIsFiring) return false;

	return true;
}

void AGunBase::SpawnBulletProjectile()
{
	UGunData* Data = GetItemData<UGunData>();
	if (!Data || !Data->BulletProjectileClass)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Please check GunData. Weapon : %s"), *GetName());
		StopAttack();
		return;
	}

	UStaticMeshComponent* MeshComp = GetStaticMeshComponent();

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
		return;
	}
	else
	{
		COMMON_LOG(LogGameplay, Log, TEXT("SpawnParam : Owner is %s, Instigator is %s"), *GetNameSafe(SpawnParams.Owner), *GetNameSafe(SpawnParams.Instigator));
	}

	Bullet->InitProjectile(Data->BulletDamage, Data->BulletSpeed, Data->BulletLifeTime);
}

void AGunBase::SetCurrentAmmo(int32 NewAmmo)
{
	CurrentAmmo = NewAmmo;

	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, GetMagazineSize());
}

int32 AGunBase::GetMagazineSize() const
{
	UGunData* Data = GetItemData<UGunData>();

	return Data ? Data->MagazineSize : INDEX_NONE;
}

void AGunBase::OnRep_CurrentAmmo()
{
	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, GetMagazineSize());
}
