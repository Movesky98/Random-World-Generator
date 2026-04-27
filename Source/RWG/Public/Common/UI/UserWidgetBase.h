// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserWidgetBase.generated.h"

UENUM(BlueprintType)
enum class EWidgetType : uint8
{
	None,
	SessionMenu,
	LobbyMenu,
	PlayerHUD,
	Inventory
};

/**
 * 
 */
UCLASS()
class RWG_API UUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void SetUp();

	virtual void TearDown();

	EWidgetType GetWidgetType() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	EWidgetType WidgetType;
};
