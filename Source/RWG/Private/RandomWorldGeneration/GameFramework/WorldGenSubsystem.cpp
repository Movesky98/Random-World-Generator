// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/GameFramework/WorldGenSubsystem.h"

#include "Engine/AssetManager.h"

#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"
#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"
#include "RandomWorldGeneration/Actors/WorldGenerator.h"

DEFINE_LOG_CATEGORY(LogWorldGenSubsystem);

void UWorldGenSubsystem::InitiateWorldGeneration()
{
	for (auto& Config : LoadedConfigs)
	{
		if (Config.Key == FPrimaryAssetType(WorldConfigTags::GenConfigName))
		{
			UWorldGenConfig* GenConfig = Cast<UWorldGenConfig>(Config.Value);

			WorldGenerator = GetWorld()->SpawnActor<AWorldGenerator>(GenConfig->WorldGeneratorClass);
			break;
		}
		else
			continue;
	}
	if (WorldGenerator)
		WorldGenerator->GenerateWorld(LoadedConfigs);
	else
		UE_LOG(LogWorldGenSubsystem, Error, TEXT("WorldGenerator is not found."));
}

void UWorldGenSubsystem::InitializeWorldConfig()
{
	LoadConfigByType(FPrimaryAssetType(WorldConfigTags::GenConfigName));
	LoadConfigByType(FPrimaryAssetType(WorldConfigTags::ThemeConfigName));
}

void UWorldGenSubsystem::LoadConfigByType(FPrimaryAssetType AssetType)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(AssetType, AssetIds);

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogWorldGenSubsystem, Error, TEXT("%s PrimaryAssetId is not found."), *AssetType.ToString());
		return;
	}

	if (UObject* LoadObject = AssetManager.GetPrimaryAssetObject(AssetIds[0]))
	{
		// 이미 있다면 즉시 콜백 호출
		OnConfigInitialized(AssetType);
	}
	else
	{
		// 비동기 로드 시작
		// 여기서 AssetType을 바인딩하여 OnConfigInitialized의 인자를 넘겨줌
		AssetManager.LoadPrimaryAssets(
			AssetIds,
			TArray<FName>(),
			FStreamableDelegate::CreateUObject(this, &UWorldGenSubsystem::OnConfigInitialized, AssetType)
		);

		UE_LOG(LogWorldGenSubsystem, Log, TEXT("Starting async load for: %s"), *AssetType.ToString());
	}
}

void UWorldGenSubsystem::OnConfigInitialized(FPrimaryAssetType AssetType)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(AssetType, AssetIds);

	UObject* LoadObject = AssetManager.GetPrimaryAssetObject(AssetIds[0]);
	if (!LoadObject) return;

	LoadedConfigs.Add(AssetType, LoadObject);
	if (LoadedConfigs.Num() >= ExpectedCount)
	{
		UE_LOG(LogWorldGenSubsystem, Log, TEXT("All Configs loaded. Starting world generation..."));
		InitiateWorldGeneration();
	}
}

void UWorldGenSubsystem::OnWorldBeginPlay(UWorld& World)
{
	Super::OnWorldBeginPlay(World);

	InitializeWorldConfig();
}
