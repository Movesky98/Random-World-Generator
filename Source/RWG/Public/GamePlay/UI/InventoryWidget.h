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
	UInventoryWidget();

	void InitInventory(UInventoryComponent* InInventoryComponent);

protected:
	void NativeConstruct() override;

	void NativeDestruct() override;

	void ToggleInventory();
	
	void RefreshInventory();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> ItemGrid;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Item")
	TSubclassOf<UItemSlot> ItemSlotClass;

private:
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent;
};
