// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/GameFramework/WorldGenSubsystem.h"
#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"
#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"
#include "RandomWorldGeneration/Actors/WorldGenerator.h"
#include "Common/GameFramework/GlobalUISubsystem.h"
#include "CommonLogCategories.h"

#include "Engine/AssetManager.h"
#include "HAL/IConsoleManager.h"

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
	{
		WorldGenerator->OnWorldGenerationCompleted.AddUObject(this, &ThisClass::OnWorldGenerationCompleted);
		WorldGenerator->GenerateWorld(LoadedConfigs);
	}
	else
		UE_LOG(LogWorldGenSubsystem, Error, TEXT("WorldGenerator is not found."));
}

void UWorldGenSubsystem::Deinitialize()
{
	if (!IsWorldReady())
	{
		const TCHAR* CVarStr = TEXT("pcg.FrameTime");
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarStr))
		{
			FString DefaultValueStr = CVar->GetDefaultValue();
			int32 DefaultValue = FCString::Atoi(*DefaultValueStr);
			CVar->Set(DefaultValue);

			COMMON_LOG(LogRandomWorldGen, Log, TEXT("Set CVar '%s' to %d"), CVarStr, DefaultValue);
		}
		else COMMON_LOG(LogRandomWorldGen, Warning, TEXT("Failed to find CVar '%s'"), CVarStr);
	}

	Super::Deinitialize();
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

	FString LevelName = World.GetMapName();
	LevelName.RemoveFromStart(World.StreamingLevelsPrefix);

	if (LevelName.Contains("LV_ExpeditionWorld"))
	{
		if (UGlobalUISubsystem* UISubsystem = GetGlobalUISubsystem())
		{
			UISubsystem->ShowLoadingScreen();
		}

		// Set pcg.Frametime
		const TCHAR* CVarStr = TEXT("pcg.FrameTime");
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarStr))
		{
			int SetValue = 100;
			CVar->Set(SetValue);

			COMMON_LOG(LogRandomWorldGen, Log, TEXT("Set CVar '%s' to %d"), CVarStr, SetValue);
		}
		else COMMON_LOG(LogRandomWorldGen, Warning, TEXT("Failed to find CVar '%s'"), CVarStr);

		// Start GenerateWorld
		InitializeWorldConfig();
	}
}

void UWorldGenSubsystem::OnWorldGenerationCompleted()
{
	bWorldReady = true;
	OnWorldReady.Broadcast();

	if (UGlobalUISubsystem* UISubsystem = GetGlobalUISubsystem())
	{
		UISubsystem->NotifyWorldGenReady();
	}

	const TCHAR* CVarStr = TEXT("pcg.FrameTime");
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarStr))
	{
		FString DefaultValueStr = CVar->GetDefaultValue();
		int32 DefaultValue = FCString::Atoi(*DefaultValueStr);
		CVar->Set(DefaultValue);

		COMMON_LOG(LogRandomWorldGen, Log, TEXT("Set CVar '%s' to %d"), CVarStr, DefaultValue);
	}
	else COMMON_LOG(LogRandomWorldGen, Warning, TEXT("Failed to find CVar '%s'"), CVarStr);
}

UGlobalUISubsystem* UWorldGenSubsystem::GetGlobalUISubsystem() const
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		return GI->GetSubsystem<UGlobalUISubsystem>();
	}

	return nullptr;
}
