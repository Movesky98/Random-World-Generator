// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "Gameplay/DataAssets/ItemData.h"
#include "ExtractionConditionRow.generated.h"

USTRUCT()
struct FExtractionConditionRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemData> ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredQuantity;
};

USTRUCT()
struct FExtractionCondition
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName ItemID;

	UPROPERTY()
	int32 RequiredQuantity = 0;

	UPROPERTY()
	int32 CurrentQuantity = 0;

	bool IsSatisfied() const { return CurrentQuantity >= RequiredQuantity; }
};
