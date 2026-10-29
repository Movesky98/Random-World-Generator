// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/GameFramework/WorldGenSubsystem.h"
#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"
#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"
#include "RandomWorldGeneration/Actors/WorldGenerator.h"
#include "Common/GameFramework/GlobalUISubsystem.h"
#include "Common/GameFramework/GameAssetManager.h"
#include "CommonLogCategories.h"

#include "HAL/IConsoleManager.h"

void UWorldGenSubsystem::InitiateWorldGeneration()
{
	if (bGenerationStarted) return;
	if (LoadedConfigs.Num() < ExpectedCount) return;

	// 서버: 생성기를 직접 스폰, 클라이언트: 복제 도착 시 RegisterWorldGenerator로 등록
	if (!WorldGenerator && GetWorld()->GetNetMode() != NM_Client)
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

		if (!WorldGenerator)
		{
			COMMON_LOG(LogRandomWorldGen, Error, TEXT("WorldGenerator is not found."));
			return;
		}
	}

	if (!WorldGenerator)
	{
		COMMON_LOG(LogRandomWorldGen, Log, TEXT("Waiting for replicated WorldGenerator."));
		return;
	}

	bGenerationStarted = true;
	WorldGenerator->OnWorldGenerationCompleted.AddUObject(this, &ThisClass::OnWorldGenerationCompleted);
	WorldGenerator->GenerateWorld(LoadedConfigs);
}

void UWorldGenSubsystem::RegisterWorldGenerator(AWorldGenerator* InWorldGenerator)
{
	WorldGenerator = InWorldGenerator;
	InitiateWorldGeneration();
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
	CacheConfigByType(FPrimaryAssetType(WorldConfigTags::GenConfigName));
	CacheConfigByType(FPrimaryAssetType(WorldConfigTags::ThemeConfigName));

	if (LoadedConfigs.Num() >= ExpectedCount)
	{
		COMMON_LOG(LogRandomWorldGen, Log, TEXT("All Configs loaded. Starting world generation..."));
		InitiateWorldGeneration();
	}
}

void UWorldGenSubsystem::CacheConfigByType(FPrimaryAssetType AssetType)
{
	if (UObject* Config = UGameAssetManager::Get().FindLoadedAsset(AssetType))
	{
		LoadedConfigs.Add(AssetType, Config);
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
