// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DataAssets/ItemData.h"
#include "Gameplay/Enums/WeaponTypes.h"
#include "WeaponData.generated.h"

class UAnimSequence;

/**
 * 
 */
UCLASS()
class RWG_API UWeaponData : public UItemData
{
	GENERATED_BODY()
public:
	/** 이 무기가 들어가는 인벤토리 칸 종류 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Slot")
	EWeaponSlotType SlotType = EWeaponSlotType::Primary;

	// 등 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName HolsterSocketName;

	// 장착 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Socket")
	FName EquipSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Properties")
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterEquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterUnequipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TSubclassOf<UAnimInstance> AnimLayerClass;

	// 무기를 든 상체 정지 포즈
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimSequence> IdlePose;
};
