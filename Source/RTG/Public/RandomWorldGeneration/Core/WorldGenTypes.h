// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.generated.h"

UENUM(BlueprintType)
enum class EWorldTheme : uint8
{
	None			UMETA(DisplayName = "None"),
	IsolatedCity	UMETA(DisplayName = "Isolated City"),	// 고립된 도시
	RuralVillage	UMETA(DisplayName = "Rural Village"),	// 촌락
	Joseon			UMETA(DisplayName = "Joseon"),			// 조선
};

namespace WorldConfigTags
{
	const FName GenConfigName = FName("WorldGenConfig");
	const FName ThemeConfigName = FName("WorldThemeConfig");
}

