// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Components/InputComponentBase.h"
#include "Gameplay/Data/ItemInstance.h"
#include "InventoryComponent.generated.h"

class AWeaponBase;

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_MULTICAST_DELEGATE(FOnInventoryToggled);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, UItemData* /* ItemData */, int32 /* Quantity */);

/**
 * 
 */
UCLASS()
class RWG_API UInventoryComponent : public UInputComponentBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	UInventoryComponent();

protected:
	void BeginPlay() override;

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                             입력 처리
*********************************************************************/
protected:
	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

	TSubclassOf<UInputConfigBase> GetConfigClass() override;

/*********************************************************************
*                            아이템 슬롯
*********************************************************************/
private:
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
	TArray<FItemInstance> Slots;

	int32 FindExistingSlot(UItemData* ItemData) const;

	int32 FindSlot(UItemData* ItemData) const;

protected:
	UFUNCTION()
	void OnRep_Slots();

public:
	UFUNCTION(Category = "Inventory")
	bool AddItem(const FItemInstance& NewItem);

	UFUNCTION(Category = "Inventory")
	bool RemoveItem(UItemData* ItemData, int32 Quantity);

	UFUNCTION(Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	const TArray<FItemInstance> GetSlots();

	FOnInventoryChanged	OnInventoryChanged;

	FOnItemAdded OnItemAdded;

/*********************************************************************
*                                무기
*********************************************************************/
protected:
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AWeaponBase>> Weapons;

public:
	void TryAddWeapon(AWeaponBase* Weapon);

	AWeaponBase* GetWeaponAtSlot(int32 SlotIndex) const;

	int32 GetAmmoCount(UItemData* AmmoType) const;

/*********************************************************************
*                               방어구
*********************************************************************/
private:
	UPROPERTY(Replicated)
	FItemInstance HelmetSlot;

	UPROPERTY(Replicated)
	FItemInstance VestSlot;

	// 현재 내구도
	UPROPERTY(Replicated)
	float HelmetDurability = 0.0f;

	UPROPERTY(Replicated)
	float VestDurability = 0.0f;

public:
	float ProcessArmorHit(FName BoneName);

/*********************************************************************
*                            인벤토리 UI
*********************************************************************/
public:
	FOnInventoryToggled OnInventoryToggled;

protected:
	void ToggleInventory();

	virtual TArray<TSubclassOf<UUserWidgetBase>> GetDefaultWidgetClasses() const override;

	virtual void BindComponent(UUserWidgetBase* Widget) override;

	virtual void UnbindComponent(UUserWidgetBase* Widget) override;
};
