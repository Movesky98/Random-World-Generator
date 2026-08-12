// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Item.h"
#include "Gameplay/Components/InventoryComponent.h"
#include "CommonLogCategories.h"


// Sets default values
AItem::AItem()
{
	bReplicates = true;
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItem::Interact(AActor* Interactor)
{
	if (!ItemData)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Item %s's Data is nullptr"), *GetNameSafe(this));
		return;
	}

	if (UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>())
	{
		FItemInstance NewInstance;
		NewInstance.ItemData = ItemData;
		NewInstance.Quantity = DropQuantity;

		InventoryComp->AddItem(NewInstance);
		Destroy();
	}
	else
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Can't find InventoryCompoennt in %s"), *GetNameSafe(Interactor));
	}
}
