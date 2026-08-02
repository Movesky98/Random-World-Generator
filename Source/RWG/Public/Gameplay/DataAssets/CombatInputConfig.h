// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DataAssets/BaseInputConfig.h"
#include "CombatInputConfig.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UCombatInputConfig : public UBaseInputConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<UInputAction*> EquipWeaponActions;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> UnequipWeaponAction;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> AttackAction;
};
