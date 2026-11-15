// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponActionState : uint8
{
	None,
	Equip,
	Unequip,
	Reload,
	ReloadEmpty,
	ReloadLoop,
	ReloadEnd,
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Auto,
	Semi,
	Burst,
};
