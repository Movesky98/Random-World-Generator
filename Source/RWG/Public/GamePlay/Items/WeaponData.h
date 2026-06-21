// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Items/ItemData.h"
#include "WeaponData.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UWeaponData : public UItemData
{
	GENERATED_BODY()
public:
	// 등 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName HolsterSocketName;

	// 장착 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName EquipSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> UnequipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimLayerClass;
};
