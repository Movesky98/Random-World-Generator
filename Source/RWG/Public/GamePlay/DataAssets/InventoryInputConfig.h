// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/DataAssets/BaseInputConfig.h"
#include "InventoryInputConfig.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UInventoryInputConfig : public UBaseInputConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	UInputAction* ToggleInventoryAction;
};
