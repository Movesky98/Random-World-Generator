// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/UI/GameplayWidget.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UWrapBox;
class UItemSlot;

/**
 * 
 */
UCLASS()
class RWG_API UInventoryWidget : public UGameplayWidget
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	UInventoryWidget();

protected:
	void NativeConstruct() override;

	void NativeDestruct() override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
private:
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent;

/*********************************************************************
*                           인벤토리 표시
*********************************************************************/
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> ItemGrid;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Item")
	TSubclassOf<UItemSlot> ItemSlotClass;

	void ToggleInventory();

	void RefreshInventory();

public:
	void InitInventory(UInventoryComponent* InInventoryComponent);
};
