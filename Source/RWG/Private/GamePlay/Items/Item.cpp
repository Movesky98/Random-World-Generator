// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Items/Item.h"
#include "GamePlay/Components/InventoryComponent.h"


// Sets default values
AItem::AItem()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionProfileName(TEXT("Item"));
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItem::Interact(APawn* Interactor)
{
	if (!ItemData) return;

	UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp) return;

	FItemInstance NewInstance;
	NewInstance.ItemData = ItemData;
	NewInstance.Quantity = DropQuantity;

	InventoryComp->AddItem(NewInstance);
	Destroy();
}
