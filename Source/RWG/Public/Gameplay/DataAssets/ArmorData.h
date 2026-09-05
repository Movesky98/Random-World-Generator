// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DataAssets/ItemData.h"
#include "ArmorData.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UArmorData : public UItemData
{
	GENERATED_BODY()
	
public:
	// 데미지 감소율
	UPROPERTY(EditDefaultsOnly, Category = "Armor")
	float DamageReductionRate = 0.3f;

	// 내구도
	UPROPERTY(EditDefaultsOnly, Category = "Armor")
	float MaxDurability = 100.0f;
};
