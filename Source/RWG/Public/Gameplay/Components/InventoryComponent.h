// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Components/InputComponentBase.h"
#include "Gameplay/Data/ItemInstance.h"
#include "Gameplay/Enums/WeaponTypes.h"
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
private:
	/** 무기 칸 수 (1·2번 주무기, 3번 권총·근접 무기, 4번 투척 무기) */
	static constexpr int32 WeaponSlotCount = 4;

	/** 무기를 버릴 때 카메라 시선 방향으로 떨어뜨리는 거리(cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DropDistance = 80.0f;

	/** 무기를 버리는 높이 (캐릭터 중심 기준, cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DropHeight = 0.0f;

	/** 버릴 자리가 막혔을 때 막힌 지점에서 캐릭터 쪽으로 당기는 거리(cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DropWallOffset = 30.0f;

	// 칸 종류가 쓰는 칸 번호 범위를 구한다
	void GetSlotRange(EWeaponSlotType SlotType, int32& OutFirstSlot, int32& OutLastSlot) const;

	// 무기를 버릴 위치와 방향을 구한다
	FVector GetDropLocation(FRotator& OutDropRotation) const;

protected:
	/** 칸 번호별 무기 (빈 칸은 nullptr) */
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AWeaponBase>> Weapons;

public:
	// 무기를 칸 종류에 맞는 빈 칸에 넣는다. 빈 칸이 없으면 들고 있는 무기의 칸(없으면 첫 칸)의 무기를 버리고 넣는다. 들어간 칸 번호를 돌려준다
	int32 AddWeapon(AWeaponBase* NewWeapon, AWeaponBase* HeldWeapon, AWeaponBase*& OutDroppedWeapon);

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
