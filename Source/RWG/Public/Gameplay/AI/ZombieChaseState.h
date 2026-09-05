// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieChaseState.generated.h"

UENUM(BlueprintType)
enum class EZombieChaseState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	SlotChase	UMETA(DisplayName = "SlotChase"),
	CloseChase	UMETA(DisplayName = "CloseChase"),
	Attack		UMETA(DisplayName = "Attack"),
};
