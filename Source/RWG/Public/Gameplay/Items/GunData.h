// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/WeaponData.h"
#include "GunData.generated.h"

class ABulletProjectile;

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
};
