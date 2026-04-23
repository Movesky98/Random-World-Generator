// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/UI/ItemSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UItemSlot::UpdateSlot(const FItemInstance& InItemInstance)
{
	ItemInstance = InItemInstance;

	UItemData* Data = ItemInstance.ItemData;
	if (!Data) return;

	Frame->SetVisibility(ESlateVisibility::Visible);

	if (Data->Icon)
	{
		ItemIcon->SetBrushFromTexture(Data->Icon);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}

	if (Data->MaxStackSize > 1)
	{
		QuantityText->SetText(FText::AsNumber(ItemInstance.Quantity));
		QuantityText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		QuantityText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UItemSlot::ClearSlot()
{
	ItemInstance = FItemInstance();
	ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	QuantityText->SetVisibility(ESlateVisibility::Collapsed);
	Frame->SetVisibility(ESlateVisibility::Collapsed);
}
