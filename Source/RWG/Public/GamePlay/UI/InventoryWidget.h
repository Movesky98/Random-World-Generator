// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UWrapBox;
class UItemSlot;

/**
 * 
 */
UCLASS()
class RWG_API UInventoryWidget : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	void InitInventory(UInventoryComponent* InInventoryComponent);
	void RefreshInventory();

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> ItemGrid;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Item")
	TSubclassOf<UItemSlot> ItemSlotClass;

private:
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UFUNCTION()
	void OnInventoryChanged();
};
