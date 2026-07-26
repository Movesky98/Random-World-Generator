// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerDetectable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerDetectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RWG_API IPlayerDetectable
{
	GENERATED_BODY()

public:
	virtual void OnPlayerApproach(APawn* Player) = 0;
	virtual void OnPlayerLeave(APawn* Player) = 0;
};
