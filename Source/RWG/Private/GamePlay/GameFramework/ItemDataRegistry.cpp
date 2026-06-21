// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ItemDataRegistry.h"
#include "GamePlay/Items/ItemData.h"
#include "CommonLogCategories.h"

#include "Engine/AssetManager.h"

void UItemDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAssetManager& Manager = UAssetManager::Get();


}

UItemData* UItemDataRegistry::FindItemDataByID(FName ID) const
{


	return nullptr;
}
