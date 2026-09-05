// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/WeaponData.h"
#include "GunData.generated.h"

class ABulletProjectile;
class UNiagaraSystem;
class USoundBase;

/**
 * 
 */
UCLASS()
class RWG_API UGunData : public UWeaponData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	int32 MagazineSize = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float FireRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float Range = 5000.0f;

	// 소모 탄약 타입
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	TObjectPtr<UItemData> AmmoType = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	TSubclassOf<ABulletProjectile> BulletProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	float BulletDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	float BulletSpeed = 30000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	float BulletLifeTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Socket")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Socket")
	FName EjectionPortSocketName = TEXT("EjectionPort");

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Socket")
	FName HandGuardSocketName = TEXT("HandGuard");

	UPROPERTY(EditDefaultsOnly, Category = "Gun|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|FX")
	TObjectPtr<UNiagaraSystem> ShellEjectFX;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Sound")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Sound")
	TObjectPtr<USoundBase> DryFireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterReloadEmptyMontage;

	// 아래는 무기 메시가 재생한다. 캐릭터 몽타주와 1:1 짝으로 authoring된 것을 물릴 것
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshReloadEmptyMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshEquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshUnequipMontage;
};
