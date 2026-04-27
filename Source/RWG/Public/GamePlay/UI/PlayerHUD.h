// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UPlayerHUD : public UUserWidgetBase
{
	GENERATED_BODY()
public:
	UPlayerHUD();

protected:
	virtual void SetUp() override;
};
