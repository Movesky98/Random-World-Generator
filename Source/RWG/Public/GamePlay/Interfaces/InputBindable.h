// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnhancedInputComponent.h"
#include "GamePlay/DataAssets/BaseInputConfig.h"
#include "InputBindable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInputBindable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RWG_API IInputBindable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void BindInputActions(UEnhancedInputComponent* InputComponent) = 0;

	virtual UBaseInputConfig* GetInputConfig() const = 0;

	virtual int32 GetIMCPriority() const = 0;

	virtual UInputMappingContext* GetMappingContext() const = 0;

	virtual bool IsConfigLoaded() const = 0;

	virtual void BindOnConfigLoaded(TFunction<void()> Callback) = 0;
};
