// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseInputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS()
class RWG_API UBaseInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	UInputMappingContext* MappingContext;

	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere)
	UInputAction* LookAction;

protected:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
