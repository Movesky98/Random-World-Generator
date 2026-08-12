// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/ConsumableItem.h"

AConsumableItem::AConsumableItem()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetCollisionProfileName(TEXT("Item"));

	RootComponent = StaticMeshComponent;
}
