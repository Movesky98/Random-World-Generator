// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Components/BaseInputComponent.h"
#include "GamePlay/Items/ItemInstance.h"
#include "InventoryComponent.generated.h"

class AWeaponBase;

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
	UInventoryComponent();

protected:
	void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UFUNCTION(Category = "Inventory")
	bool AddItem(const FItemInstance& NewItem);

	UFUNCTION(Category = "Inventory")
	bool RemoveItem(UItemData* ItemData, int32 Quantity);

	UFUNCTION(Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	FOnInventoryChanged	OnInventoryChanged;

	FOnInventoryToggled OnInventoryToggled;

	const TArray<FItemInstance> GetSlots();

protected:
	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

	TSubclassOf<UBaseInputConfig> GetConfigClass() override;

	void ToggleInventory();

	UFUNCTION()
	void OnRep_Slots();

private:
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
	TArray<FItemInstance> Slots;

	int32 FindExistingSlot(UItemData* ItemData) const;

	/* Weapon */
public:
	void TryAddWeapon(AWeaponBase* Weapon);

	AWeaponBase* GetWeaponAtSlot(int32 SlotIndex) const;

	int32 GetAmmoCount(UItemData* AmmoType) const;

	bool ConsumeAmmo(UItemData* AmmoType, int32 Amount);

	void Debug_AddAmmo(UItemData* AmmoType, int32 Amount);

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TObjectPtr<UItemData> Debug_DefaultAmmo;

protected:
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AWeaponBase>> Weapons;
};
