// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/GunBase.h"
#include "Gameplay/Items/GunData.h"
#include "Gameplay/Actors/BulletProjectile.h"
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

	if (!CanStartAttack()) return;

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

int32 AGunBase::GetMagazineSize() const
{
	UGunData* Data = GetItemData<UGunData>();

	return Data ? Data->MagazineSize : INDEX_NONE;
}

void AGunBase::OnRep_CurrentAmmo()
{
	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, GetMagazineSize());
}
