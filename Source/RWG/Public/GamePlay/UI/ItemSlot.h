// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GamePlay/Items/ItemInstance.h"
#include "ItemSlot.generated.h"

class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class RWG_API UItemSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateSlot(const FItemInstance& InItemInstance);
	
	void ClearSlot();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Frame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText;

private:
	FItemInstance ItemInstance;
};
