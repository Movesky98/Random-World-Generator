// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/UIManagerComponent.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/Components/CombatComponent.h"
#include "Common/UI/UserWidgetBase.h"
#include "GamePlay/UI/InventoryWidget.h"
#include "GamePlay/UI/PlayerHUD.h"
#include "GamePlay/Interfaces/WidgetBindable.h"
#include "CommonLogCategories.h"

#include "Kismet/GameplayStatics.h"

UUIManagerComponent::UUIManagerComponent()
{
	bWantsInitializeComponent = true;

	CurrentPhase = EGamePhase::None;
}

void UUIManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerController = Cast<APlayerController>(GetOwner());
	checkf(OwnerController, TEXT("UIManagerComponent must be attached to PlayerController"));

	InitializeLevelNameMap();
}

void UUIManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUIManagerComponent::InitializePawnWidgets(APawn* aPawn)
{
	if (!OwnerController->IsLocalController())
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("NetMode: %s / OwnerController: %s / IsLocal: %d / Pawn : %s"),
			*ToString(GetWorld()->GetNetMode()),
			*GetNameSafe(OwnerController),
			OwnerController ? OwnerController->IsLocalController() : false,
			*GetNameSafe(aPawn));
		return;
	}

	DisplayWidgetsForPhase();
}

void UUIManagerComponent::BindDelegatesFromGameplayWidgets(const TArray<TScriptInterface<IWidgetBindable>>& BindableComponents)
{
	for (auto& Component : BindableComponents)
	{
		TArray<TSubclassOf<UUserWidgetBase>> WidgetClasses = Component->GetDefaultWidgetClasses();

		for (TSubclassOf<UUserWidgetBase>& WidgetClass : WidgetClasses)
		{
			if (!WidgetClass) continue;

			for (UUserWidgetBase* Widget : AvailableWidgets)
			{
				if (Widget->IsA(WidgetClass))
				{
					Component->BindComponent(Widget);
					break;
				}
			}
		}
	}
}

void UUIManagerComponent::UnbindDelegatesFromGameplayWidgets(const TArray<TScriptInterface<IWidgetBindable>>& BindableComponents)
{
	for (auto& Component : BindableComponents)
	{
		TArray<TSubclassOf<UUserWidgetBase>> WidgetClasses = Component->GetDefaultWidgetClasses();

		for (TSubclassOf<UUserWidgetBase>& WidgetClass : WidgetClasses)
		{
			if (!WidgetClass) continue;

			for (UUserWidgetBase* Widget : AvailableWidgets)
			{
				if (Widget->IsA(WidgetClass))
				{
					Component->UnbindComponent(Widget);
					break;
				}
			}
		}
	}
}

void UUIManagerComponent::DisplayWidgetsForPhase()
{
	FString CurrentLevelStr = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	COMMON_LOG(LogGameplay, Log, TEXT("CurrentLevelName : %s"), *CurrentLevelStr);

	CurrentPhase = ConvertLevelNameToPhase(FName(CurrentLevelStr));
	COMMON_LOG(LogGameplay, Warning, TEXT("CurrentPhase: %s"), *UEnum::GetValueAsString(CurrentPhase));

	CreateWidgetsForPhase(CurrentPhase);
}

void UUIManagerComponent::InitializeLevelNameMap()
{
	for (auto& [SoftObjectPath, Phase] : LevelToPhaseMap)
	{
		if (Phase == EGamePhase::None) return;

		FName LevelName = FName(*SoftObjectPath.GetAssetName());
		LevelNameToPhaseMap.Add(LevelName, Phase);
	}
}

void UUIManagerComponent::CreateWidgetsForPhase(EGamePhase Phase)
{
	COMMON_LOG(LogGameplay, Log, TEXT("Current Phase : %s"), *UEnum::GetValueAsString(Phase))
	FWidgetClassList* CurrentPhaseWidgetClasses = WidgetClassMap.Find(Phase);
	if (!CurrentPhaseWidgetClasses) return;

	for (TSubclassOf<UUserWidgetBase> WidgetClass : CurrentPhaseWidgetClasses->WidgetClasses)
	{
		if (UUserWidgetBase* WidgetBase = CreateWidget<UUserWidgetBase>(OwnerController, WidgetClass))
		{
			AvailableWidgets.Add(WidgetBase);
			COMMON_LOG(LogGameplay, Log, TEXT("Saved widget name : %s"), *WidgetBase->GetClass()->GetName());
		}
	}

	for (auto& Widget : AvailableWidgets)
	{
		Widget->AddToViewport();
	}

	ShowDefaultWidget(CurrentPhaseWidgetClasses->DefaultWidgetClass);
}

EGamePhase UUIManagerComponent::ConvertLevelNameToPhase(FName LevelName)
{
	if (!LevelNameToPhaseMap.Num())
		return EGamePhase::None;

	EGamePhase* PhasePtr = LevelNameToPhaseMap.Find(LevelName);

	if (!PhasePtr)
	{
		return EGamePhase::None;
	}

	return *PhasePtr;
}

void UUIManagerComponent::ShowDefaultWidget(TSubclassOf<UUserWidgetBase> WidgetClass)
{
	if (!WidgetClass) return;

	UUserWidgetBase* FoundWidget = nullptr;
	for (UUserWidgetBase* Widget : AvailableWidgets)
	{
		if (Widget->IsA(WidgetClass))
		{
			FoundWidget = Widget;
			break;
		}
	}

	if (!FoundWidget)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("No widget found for DefaultWidgetClass. CurrentPhase is %s"), *UEnum::GetValueAsString(CurrentPhase));
		return;
	}

	if (CurrentWidget)
	{
		CurrentWidget->TearDown();
		CurrentWidget = nullptr;
	}

	CurrentWidget = FoundWidget;
	CurrentWidget->SetUp();
	COMMON_LOG(LogGameplay, Log, TEXT("Set up widget name : %s"), *GetNameSafe(CurrentWidget->GetClass()));
}