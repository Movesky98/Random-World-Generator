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

/** 무기가 들어가는 인벤토리 칸 종류 */
UENUM(BlueprintType)
enum class EWeaponSlotType : uint8
{
	Primary,	// 1·2번 칸
	Secondary,	// 3번 칸 (권총, 근접 무기)
	Throwable,	// 4번 칸
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Auto,
	Semi,
	Burst,
};
