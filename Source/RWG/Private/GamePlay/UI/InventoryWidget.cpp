// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/UI/InventoryWidget.h"
#include "GamePlay/UI/ItemSlot.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "Components/WrapBox.h"
#include "CommonLogCategories.h"

UInventoryWidget::UInventoryWidget()
{

}

void UInventoryWidget::InitInventory(UInventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent)
		return;

	InventoryComponent = InInventoryComponent;
	InventoryComponent->OnInventoryChanged.AddUObject(this, &ThisClass::RefreshInventory);
	InventoryComponent->OnInventoryToggled.AddUObject(this, &ThisClass::ToggleInventory);
	
	this->SetVisibility(ESlateVisibility::Collapsed);
	
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

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("GetOwningPlayerPawn is nullptr."));
		return;
	}
}

void UInventoryWidget::NativeDestruct()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.RemoveAll(this);
		InventoryComponent->OnInventoryToggled.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UInventoryWidget::ToggleInventory()
{
	IsVisible() ? SetVisibility(ESlateVisibility::Collapsed) : SetVisibility(ESlateVisibility::Visible);
}