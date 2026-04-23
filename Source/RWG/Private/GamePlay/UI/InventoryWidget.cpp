// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/UI/InventoryWidget.h"
#include "GamePlay/UI/ItemSlot.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "Components/WrapBox.h"

void UInventoryWidget::InitInventory(UInventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent)
		return;

	InventoryComponent = InInventoryComponent;
	InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);

	RefreshInventory();
}

void UInventoryWidget::RefreshInventory()
{
	if (!InventoryComponent || !ItemSlotClass) return;

	ItemGrid->ClearChildren();

	for (const FItemInstance& Instance : InventoryComponent->GetSlots())
	{
		UItemSlot* ItemSlot = CreateWidget<UItemSlot>(GetOwningPlayer(), ItemSlotClass);
		if (!ItemSlot) continue;

		ItemSlot->UpdateSlot(Instance);
		ItemGrid->AddChild(ItemSlot);
	}
}

void UInventoryWidget::NativeDestruct()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryChanged);
	}

	Super::NativeDestruct();
}

void UInventoryWidget::OnInventoryChanged()
{
	RefreshInventory();
}
