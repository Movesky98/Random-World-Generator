// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/AmmoItem.h"
#include "Gameplay/DataAssets/AmmoData.h"

AAmmoItem::AAmmoItem()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetCollisionProfileName(TEXT("Item"));

	RootComponent = StaticMeshComponent;
}

void AAmmoItem::ApplyItemData()
{
	UAmmoData* AmmoData = GetItemData<UAmmoData>();
	if (!AmmoData || !StaticMeshComponent) return;

	StaticMeshComponent->SetStaticMesh(AmmoData->Mesh);

	if (AmmoData->MeshMaterial)
	{
		StaticMeshComponent->SetMaterial(0, AmmoData->MeshMaterial);
	}
}
