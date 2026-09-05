// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DataAssets/WeaponData.h"
#include "GrenadeData.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UGrenadeData : public UWeaponData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionRadius = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float FuseTime = 3.0f;
};
