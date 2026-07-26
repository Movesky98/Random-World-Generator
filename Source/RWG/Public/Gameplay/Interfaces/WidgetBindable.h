// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WidgetBindable.generated.h"

class UUserWidgetBase;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWidgetBindable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RWG_API IWidgetBindable
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UUserWidgetBase>> GetDefaultWidgetClasses() const = 0;

	virtual void BindComponent(UUserWidgetBase* Widget) = 0;

	virtual void UnbindComponent(UUserWidgetBase* Widget) = 0;
};
