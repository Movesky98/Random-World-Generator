// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/DataAssets/InventoryInputConfig.h"
#include "GamePlay/Items/WeaponBase.h"
#include "GamePlay/Items/ArmorData.h"
#include "GamePlay/UI/InventoryWidget.h"
#include "CommonLogCategories.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Debug_DefaultAmmo)
		Debug_AddAmmo(Debug_DefaultAmmo, 90);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryComponent, Slots, COND_OwnerOnly);
	DOREPLIFETIME(UInventoryComponent, Weapons);
}


const TArray<FItemInstance> UInventoryComponent::GetSlots()
{
	return Slots.Num() ? Slots : TArray<FItemInstance>();
}

void UInventoryComponent::BindInputActions(UEnhancedInputComponent* InputComponent)
{
	UInventoryInputConfig* InventoryConfig = Cast<UInventoryInputConfig>(LoadedConfig);

	if (!InputComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InputComponent is null. SetupInputComponent may not have been called."));
		return;
	}

	if (!InventoryConfig)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InventoryConfig is null. Check Asset Manager or config assignment."));
		return;
	}

	InputComponent->BindAction(InventoryConfig->ToggleInventoryAction, ETriggerEvent::Triggered, this, &ThisClass::ToggleInventory);
}

TSubclassOf<UBaseInputConfig> UInventoryComponent::GetConfigClass()
{
	return UInventoryInputConfig::StaticClass();
}

void UInventoryComponent::ToggleInventory()
{
	OnInventoryToggled.Broadcast();

	UE_LOG(LogGameplay, Warning, TEXT("--------------------- Inventory State ---------------------"));

	if (Slots.Num())
	{
		for (int32 i = 0; i < Slots.Num(); i++)
		{
			FItemInstance& ItemInstance = Slots[i];

			UE_LOG(LogGameplay, Warning, TEXT("%d\t%s\t%d"),
				i,
				*ItemInstance.ItemData->DisplayName.ToString(),
				ItemInstance.Quantity
			);
		}
	}
	else
	{
		UE_LOG(LogGameplay, Warning, TEXT("No Item Slots."));
	}
	
}

void UInventoryComponent::OnRep_Slots()
{
	COMMON_LOG(LogGameplay, Warning, TEXT("Slots is changed."));
	OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::AddItem(const FItemInstance& NewItem)
{
	if (!NewItem.IsValid()) return false;
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;

	int32 RemainingQuantity = NewItem.Quantity;

	while (RemainingQuantity > 0)
	{
		int32 ExistingIndex = FindExistingSlot(NewItem.ItemData);

		if (ExistingIndex != INDEX_NONE)
		{
			FItemInstance& Existing = Slots[ExistingIndex];
			int32 CanAdd = NewItem.ItemData->MaxStackSize - Existing.Quantity;

			if (CanAdd > 0)
			{
				int32 Added = FMath::Min(CanAdd, RemainingQuantity);
				Existing.Quantity += Added;
				RemainingQuantity -= Added;
			}
			else
			{
				// 货 浇吩 积己
				FItemInstance NewSlot;
				NewSlot.ItemData = NewItem.ItemData;
				NewSlot.Quantity = FMath::Min(RemainingQuantity, NewItem.ItemData->MaxStackSize);
				RemainingQuantity -= NewSlot.Quantity;
				Slots.Add(NewSlot);
			}
		}
		else
		{
			// 货 浇吩 积己
			FItemInstance NewSlot;
			NewSlot.ItemData = NewItem.ItemData;
			NewSlot.Quantity = FMath::Min(RemainingQuantity, NewItem.ItemData->MaxStackSize);
			RemainingQuantity -= NewSlot.Quantity;
			Slots.Add(NewSlot);
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(UItemData* ItemData, int32 Quantity)
{
	if (!ItemData || Quantity <= 0) return false;

	int32 ExistingIndex = FindExistingSlot(ItemData);
	if (ExistingIndex == INDEX_NONE) return false;

	FItemInstance& Existing = Slots[ExistingIndex];
	Existing.Quantity -= Quantity;

	if (Existing.Quantity <= 0)
	{
		Slots.RemoveAt(ExistingIndex);
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex)) return false;

	FItemInstance& instance = Slots[SlotIndex];
	UItemData* Data = instance.ItemData;
	if (!Data) return false;

	switch (Data->ItemType)
	{
	case EItemType::Weapon:
		break;
	case EItemType::Consumable:
		break;
	default:
		return false;
	}

	return true;
}

int32 UInventoryComponent::FindExistingSlot(UItemData* ItemData) const
{
	int32 Index = 0;

	for (auto& Slot : Slots)
	{
		if (Slot.ItemData == ItemData && Slot.Quantity < ItemData->MaxStackSize)
		{
			return Index;
		}

		Index++;
	}

	return INDEX_NONE;
}

void UInventoryComponent::TryAddWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return;

	// 老窜 林公扁何磐
	Weapons.Add(Weapon);
	Weapon->AttachToHolster(Cast<ACharacter>(GetOwner()));

	COMMON_LOG(LogGameplay, Log, TEXT("Primary Weapon acquired: %s"), *Weapon->GetName());
}

AWeaponBase* UInventoryComponent::GetWeaponAtSlot(int32 SlotIndex) const
{
	return Weapons.IsValidIndex(SlotIndex) ? Weapons[SlotIndex] : nullptr;
}

int32 UInventoryComponent::GetAmmoCount(UItemData* AmmoType) const
{
	if (!AmmoType) return 0;
	
	int32 Total = 0;
	for (const FItemInstance& Item : Slots)
	{
		if (Item.ItemData == AmmoType)
		{
			Total += Item.Quantity;
		}
	}

	return Total;
}

bool UInventoryComponent::ConsumeAmmo(UItemData* AmmoType, int32 Amount)
{
	if (!AmmoType || Amount <= 0) return false;

	for (FItemInstance& Item : Slots)
	{
		if (Item.ItemData == AmmoType)
		{
			if (Item.Quantity < Amount)
			{
				COMMON_LOG(LogGameplay, Warning, TEXT("Not enough ammo."));
				return false;
			}

			Item.Quantity -= Amount;
			return true;
		}
	}

	COMMON_LOG(LogGameplay, Warning, TEXT("Ammo type not found in bag."));
	return false;
}

void UInventoryComponent::Debug_AddAmmo(UItemData* AmmoType, int32 Amount)
{
	if (!AmmoType || Amount <= 0) return;

	FItemInstance AmmoInstance;

	AmmoInstance.ItemData = AmmoType;
	AmmoInstance.Quantity = Amount;
	Slots.Add(AmmoInstance);

	COMMON_LOG(LogGameplay, Log, TEXT("Debug ammo added : %d"), Amount);
}

float UInventoryComponent::ProcessArmorHit(FName BoneName)
{
	// 赣府
	if (BoneName == "spine_05")
	{
		if (HelmetSlot.ItemData && HelmetDurability > 0.0f)
		{
			if (UArmorData* ArmorData = Cast<UArmorData>(HelmetSlot.ItemData))
			{
				HelmetDurability = FMath::Max(HelmetDurability - 1.0f, 0.0f);
				return 1.0f - ArmorData->DamageReductionRate;
			}
		}
	}
	else if (BoneName == "spine_02" || BoneName == "spine_04")
	{
		if (VestSlot.ItemData && VestDurability > 0.0f)
		{
			if (UArmorData* ArmorData = Cast<UArmorData>(VestSlot.ItemData))
			{
				VestDurability = FMath::Max(VestDurability - 1.0f, 0.0f);
				return 1.0f - ArmorData->DamageReductionRate;
			}
		}
	}

	return 1.0f;
}

TSubclassOf<UUserWidgetBase> UInventoryComponent::GetDefaultWidgetClass() const
{
	return UInventoryWidget::StaticClass();
}

void UInventoryComponent::BindComponent(UUserWidgetBase* Widget)
{
	if (UInventoryWidget* InvWidget = Cast<UInventoryWidget>(Widget))
	{
		InvWidget->InitInventory(this);
	}
}

void UInventoryComponent::UnbindComponent(UUserWidgetBase* Widget)
{
	if (UInventoryWidget* InvWidget = Cast<UInventoryWidget>(Widget))
	{
		OnInventoryChanged.RemoveAll(InvWidget);
		OnInventoryToggled.RemoveAll(InvWidget);
	}
}

