// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/DataAssets/InventoryInputConfig.h"
#include "CommonLogCategories.h"

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

bool UInventoryComponent::AddItem(const FItemInstance& NewItem)
{
	if (!NewItem.IsValid()) return false;

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
	for (auto& Slot : Slots)
	{
		if (Slot.ItemData == ItemData && Slot.Quantity < ItemData->MaxStackSize)
		{
			return 1;
		}
	}

	return INDEX_NONE;
}
