// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/InputComponentBase.h"
#include "Gameplay/DataAssets/InputConfigBase.h"
#include "Common/UI/UserWidgetBase.h"
#include "CommonLogCategories.h"
#include "Engine/AssetManager.h"

// Sets default values for this component's properties
UInputComponentBase::UInputComponentBase()
{
	bWantsInitializeComponent = true;
}


void UInputComponentBase::InitializeComponent()
{
	Super::InitializeComponent();

	if(!IsValid(LoadedConfig))
		LoadInputConfig();
}

// Called when the game starts
void UInputComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

bool UInputComponentBase::TryLoadConfigFromAssetManager()
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
				LoadedConfig = Cast<UInputConfigBase>(LoadedObject);
				OnConfigLoaded.Broadcast(this);
				return true;
			}
		}
	}

	return false;
}

void UInputComponentBase::LoadInputConfig()
{
	if (TryLoadConfigFromAssetManager()) return;
	
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType(InputConfigName), AssetIds);

	AssetManager.LoadPrimaryAssets(
		AssetIds,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &UInputComponentBase::OnLoadedInputConfig)
		);
}

void UInputComponentBase::OnLoadedInputConfig()
{
	TryLoadConfigFromAssetManager();
}

UInputConfigBase* UInputComponentBase::GetInputConfig() const
{
	return IsValid(LoadedConfig) ? LoadedConfig : nullptr;
}

int32 UInputComponentBase::GetIMCPriority() const
{
	return IMCPriority;
}

UInputMappingContext* UInputComponentBase::GetMappingContext() const
{
	return IsValid(LoadedConfig) ? LoadedConfig->MappingContext : nullptr;
}

bool UInputComponentBase::IsConfigLoaded() const
{
	return IsValid(LoadedConfig);
}

void UInputComponentBase::BindOnConfigLoaded(TFunction<void()> Callback)
{
	OnConfigLoaded.AddLambda([Callback](UInputComponentBase*) {Callback(); });
}

TArray<TSubclassOf<UUserWidgetBase>> UInputComponentBase::GetDefaultWidgetClasses() const
{
	return TArray<TSubclassOf<UUserWidgetBase>>();
}

void UInputComponentBase::BindComponent(UUserWidgetBase* Widget)
{

}

void UInputComponentBase::UnbindComponent(UUserWidgetBase* Widget)
{

}
