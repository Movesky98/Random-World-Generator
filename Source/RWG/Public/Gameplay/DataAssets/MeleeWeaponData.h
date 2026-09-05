// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DataAssets/WeaponData.h"
#include "MeleeWeaponData.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UMeleeWeaponData : public UWeaponData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float SwingSpeed = 1.0f;
};
