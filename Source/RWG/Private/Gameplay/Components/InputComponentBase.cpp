// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/InputComponentBase.h"
#include "Gameplay/DataAssets/InputConfigBase.h"
#include "Common/UI/UserWidgetBase.h"
#include "Common/GameFramework/GameAssetManager.h"
#include "CommonLogCategories.h"

// Sets default values for this component's properties
UInputComponentBase::UInputComponentBase()
{
	bWantsInitializeComponent = true;
}


void UInputComponentBase::InitializeComponent()
{
	Super::InitializeComponent();

	if(!IsValid(LoadedConfig))
		TryLoadConfigFromAssetManager();
}

// Called when the game starts
void UInputComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

bool UInputComponentBase::TryLoadConfigFromAssetManager()
{
	TArray<UObject*> LoadedObjects;
	UGameAssetManager::Get().GetLoadedAssets(FPrimaryAssetType(InputConfigName), LoadedObjects);

	for (UObject* LoadedObject : LoadedObjects)
	{
		if (LoadedObject->GetClass() == GetConfigClass())
		{
			LoadedConfig = Cast<UInputConfigBase>(LoadedObject);
			OnConfigLoaded.Broadcast(this);
			return true;
		}
	}

	return false;
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
