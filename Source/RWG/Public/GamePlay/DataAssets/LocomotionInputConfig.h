// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/DataAssets/BaseInputConfig.h"
#include "LocomotionInputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS()
class RWG_API ULocomotionInputConfig : public UBaseInputConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere)
	UInputAction* LookAction;
};
