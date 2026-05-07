// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Items/WeaponData.h"
#include "GunData.generated.h"

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
};
