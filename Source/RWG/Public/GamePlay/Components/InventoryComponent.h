// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Components/BaseInputComponent.h"
#include "GamePlay/Items/ItemInstance.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_MULTICAST_DELEGATE(FOnInventoryToggled);

/**
 * 
 */
UCLASS()
class RWG_API UInventoryComponent : public UBaseInputComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(const FItemInstance& NewItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UItemData* ItemData, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	FOnInventoryChanged	OnInventoryChanged;

	FOnInventoryToggled OnInventoryToggled;

	const TArray<FItemInstance> GetSlots();

protected:
	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

	TSubclassOf<UBaseInputConfig> GetConfigClass() override;

	void ToggleInventory();

private:
	UPROPERTY()
	TArray<FItemInstance> Slots;

	int32 FindExistingSlot(UItemData* ItemData) const;
};
