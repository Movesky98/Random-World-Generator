// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/BaseInputComponent.h"
#include "GamePlay/DataAssets/BaseInputConfig.h"
#include "CommonLogCategories.h"
#include "Engine/AssetManager.h"

// Sets default values for this component's properties
UBaseInputComponent::UBaseInputComponent()
{
	bWantsInitializeComponent = true;
}


void UBaseInputComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if(!IsValid(LoadedConfig))
		LoadInputConfig();
}

// Called when the game starts
void UBaseInputComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

bool UBaseInputComponent::TryLoadConfigFromAssetManager()
{
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType(InputConfigName), AssetIds);

	if (AssetIds.IsEmpty())
	{
		COMMON_LOG(LogGameplay, Error, TEXT("PrimaryAssetId : %s, InputConfigs are not found."), *InputConfigName.ToString());
		return false;
	}

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (UObject* LoadedObject = AssetManager.GetPrimaryAssetObject(AssetId))
		{
			if (LoadedObject->GetClass() == GetConfigClass())
			{
				LoadedConfig = Cast<UBaseInputConfig>(LoadedObject);
				OnConfigLoaded.Broadcast(this);
				return true;
			}
		}
	}

	return false;
}

void UBaseInputComponent::LoadInputConfig()
{
	if (TryLoadConfigFromAssetManager()) return;
	
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType(InputConfigName), AssetIds);

	AssetManager.LoadPrimaryAssets(
		AssetIds,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &UBaseInputComponent::OnLoadedInputConfig)
		);
}

void UBaseInputComponent::OnLoadedInputConfig()
{
	TryLoadConfigFromAssetManager();
}

UBaseInputConfig* UBaseInputComponent::GetInputConfig() const
{
	return IsValid(LoadedConfig) ? LoadedConfig : nullptr;
}

int32 UBaseInputComponent::GetIMCPriority() const
{
	return IMCPriority;
}

UInputMappingContext* UBaseInputComponent::GetMappingContext() const
{
	return IsValid(LoadedConfig) ? LoadedConfig->MappingContext : nullptr;
}

bool UBaseInputComponent::IsConfigLoaded() const
{
	return IsValid(LoadedConfig);
}

void UBaseInputComponent::BindOnConfigLoaded(TFunction<void()> Callback)
{
	OnConfigLoaded.AddLambda([Callback](UBaseInputComponent*) {Callback(); });
}
