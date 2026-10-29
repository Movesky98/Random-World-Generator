// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/GameFramework/GameAssetManager.h"
#include "CommonLogCategories.h"

UGameAssetManager& UGameAssetManager::Get()
{
	UGameAssetManager* AssetManager = Cast<UGameAssetManager>(GEngine->AssetManager);
	check(AssetManager);

	return *AssetManager;
}

void UGameAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// 에셋 스캔이 끝난 뒤에 로드한다 (에디터에서는 스캔이 늦게 끝날 수 있음)
	CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::LoadInitialAssets));
}

void UGameAssetManager::LoadInitialAssets()
{
	TArray<FPrimaryAssetTypeInfo> TypeInfos;
	GetPrimaryAssetTypeInfoList(TypeInfos);

	// 에디터 전용이 아닌 타입의 에셋 ID를 모은다
	TArray<FPrimaryAssetId> AssetIds;
	for (const FPrimaryAssetTypeInfo& TypeInfo : TypeInfos)
	{
		if (TypeInfo.bIsEditorOnly) continue;

		TArray<FPrimaryAssetId> TypeAssetIds;
		GetPrimaryAssetIdList(TypeInfo.PrimaryAssetType, TypeAssetIds);
		AssetIds.Append(TypeAssetIds);
	}

	if (AssetIds.IsEmpty())
	{
		COMMON_LOG(LogGameplay, Error, TEXT("No primary assets to load."));
		return;
	}

	LoadPrimaryAssets(
		AssetIds,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnInitialAssetsLoaded)
	);

	COMMON_LOG(LogGameplay, Log, TEXT("Starting async load for %d primary assets."), AssetIds.Num());
}

void UGameAssetManager::OnInitialAssetsLoaded()
{
	COMMON_LOG(LogGameplay, Log, TEXT("Initial primary assets loaded."));
}

UObject* UGameAssetManager::FindLoadedAsset(FPrimaryAssetType AssetType) const
{
	TArray<UObject*> Assets;
	GetPrimaryAssetObjectList(AssetType, Assets);

	if (Assets.IsEmpty())
	{
		COMMON_LOG(LogGameplay, Error, TEXT("%s asset is not loaded."), *AssetType.ToString());
		return nullptr;
	}

	return Assets[0];
}

void UGameAssetManager::GetLoadedAssets(FPrimaryAssetType AssetType, TArray<UObject*>& OutAssets) const
{
	GetPrimaryAssetObjectList(AssetType, OutAssets);

	if (OutAssets.IsEmpty())
	{
		COMMON_LOG(LogGameplay, Error, TEXT("%s assets are not loaded."), *AssetType.ToString());
	}
}
