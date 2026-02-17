// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/GameFramework/WorldGenSubsystem.h"

#include "Engine/AssetManager.h"

#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"
#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"
#include "RandomWorldGeneration/Actors/WorldGenerator.h"

DEFINE_LOG_CATEGORY(LogWorldGenSubsystem);

void UWorldGenSubsystem::GenerateWorld(int32 Seed)
{
	if (WorldGenConfig && WorldGenConfig->WorldGeneratorClass)
	{
		WorldGenerator = GetWorld()->SpawnActor<AWorldGenerator>(WorldGenConfig->WorldGeneratorClass);
	}
	else
	{
		WorldGenerator = GetWorld()->SpawnActor<AWorldGenerator>(AWorldGenerator::StaticClass());
	}

	// WorldGenConfig를 넘겨줘야 할 듯.
	WorldGenerator->UpdateMesh(Seed, WorldGenConfig);
}

void UWorldGenSubsystem::InitializeWorldConfig()
{
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("WorldGenConfig"), AssetIds);

	if(AssetIds.IsEmpty())
	{
		UE_LOG(LogWorldGenSubsystem, Error, TEXT("World generation config PrimaryAssetId is not found."));
		return;
	}

	// World Config가 이미 캐싱되어 있는지 확인
	if (UObject* Object = AssetManager.GetPrimaryAssetObject(AssetIds[0]))
	{
		WorldGenConfig = Cast<UWorldGenConfig>(Object);

		OnConfigInitialized();
	}
	else
	{
		// 캐시에 올라와있지 않은 경우 비동기 작업 시작
		// 로드가 완료되면 실행될 '델리게이트'를 등록함.
		AssetManager.LoadPrimaryAssets(AssetIds, TArray<FName>(), FStreamableDelegate::CreateUObject(this, &UWorldGenSubsystem::OnConfigInitialized));

		UE_LOG(LogWorldGenSubsystem, Warning, TEXT("Starting async load of World Generation config..."));
	}
}

void UWorldGenSubsystem::OnConfigInitialized()
{
	if (!WorldGenConfig)
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		TArray<FPrimaryAssetId> AssetIds;
		AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("WorldGenConfig"), AssetIds);

		WorldGenConfig = Cast<UWorldGenConfig>(AssetManager.GetPrimaryAssetObject(AssetIds[0]));
	}

	UE_LOG(LogWorldGenSubsystem, Warning, TEXT("World Generation config loaded successfully.\n Starting world generation..."));

	GenerateWorld(0);
}

void UWorldGenSubsystem::OnWorldBeginPlay(UWorld& World)
{
	Super::OnWorldBeginPlay(World);

	InitializeWorldConfig();
}
