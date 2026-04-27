// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/UIManagerComponent.h"
#include "Common/UI/UserWidgetBase.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/UI/InventoryWidget.h"
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
	if (OwnerController->IsLocalController())
	{
		DisplayWidgetsForPhase();
	}
}

void UUIManagerComponent::DisplayWidgetsForPhase()
{
	FString CurrentLevelStr = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	COMMON_LOG(LogGameplay, Log, TEXT("CurrentLevelName : %s"), *CurrentLevelStr);

	CurrentPhase = ConvertLevelNameToPhase(FName(CurrentLevelStr));

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
	FWidgetClassList* WidgetClassesList = WidgetClassMap.Find(Phase);
	if (!WidgetClassesList) return;

	for (TSubclassOf<UUserWidgetBase> WidgetClass : WidgetClassesList->WidgetClasses)
	{
		if (UUserWidgetBase* WidgetBase = CreateWidget<UUserWidgetBase>(OwnerController, WidgetClass))
		{
			AvailableWidgets.Add(WidgetBase->GetWidgetType(), WidgetBase);
			WidgetBase->AddToViewport();
			COMMON_LOG(LogGameplay, Log, TEXT("Saved widget name : %s"), *WidgetBase->GetClass()->GetName());
		}
	}

	ShowDefaultWidget(WidgetClassesList->DefaultWidget);
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

void UUIManagerComponent::ShowDefaultWidget(EWidgetType DefaultType)
{
	TObjectPtr<UUserWidgetBase>* WidgetBasePtr = AvailableWidgets.Find(DefaultType);
	if (!WidgetBasePtr)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("No widget found for DefaultWidget. WidgetType : %s"), *UEnum::GetValueAsString(DefaultType));
		return;
	}

	if (CurrentWidget)
	{
		CurrentWidget->TearDown();
		CurrentWidget = nullptr;
	}

	CurrentWidget = *WidgetBasePtr;
	CurrentWidget->SetUp();
	COMMON_LOG(LogGameplay, Log, TEXT("Set up widget name : %s"), *WidgetBasePtr->GetClass()->GetName());
}

