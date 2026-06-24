// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ItemDataRegistry.h"
#include "GamePlay/Items/ItemData.h"
#include "CommonLogCategories.h"

#include "Engine/AssetManager.h"

bool UItemDataRegistry::IsReady() const
{
	return bIsReady;
}

void UItemDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAssetManager& AssetManager = UAssetManager::Get();
	
	// 아이템 에셋 비동기 로드 및 콜백함수 등록
	AssetManager.LoadPrimaryAssetsWithType(
		AssetType,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnItemAssetsLoaded)
	);
}

void UItemDataRegistry::OnItemAssetsLoaded()
{
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<UObject*> LoadedAssets;
	AssetManager.GetPrimaryAssetObjectList(AssetType, LoadedAssets);

	if (!LoadedAssets.Num())
	{
		COMMON_LOG(LogGameplay, Error, TEXT("%s PrimaryAssetId is not found."), *AssetType.ToString());
		return;
	}

	for (UObject* Asset : LoadedAssets)
	{
		if (UItemData* ItemData = Cast<UItemData>(Asset))
		{
			CachedItems.Add(ItemData->ItemID, ItemData);
		}
	}

	bIsReady = true;
	OnRegistryReady.Broadcast();

	COMMON_LOG(LogGameplay, Log, TEXT("%s PrimaryAssetId is found. CachedItem num is %d"), *AssetType.ToString(), CachedItems.Num());
}

UItemData* UItemDataRegistry::FindItemDataByID(FName ID) const
{
	if (UItemData* FoundItem = CachedItems.FindRef(ID))
	{
		return FoundItem;
	}

	return nullptr;
}
