// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/UIManagerComponent.h"
#include "Common/UI/UserWidgetBase.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/Components/CombatComponent.h"
#include "GamePlay/UI/InventoryWidget.h"
#include "GamePlay/UI/PlayerHUD.h"
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

	if (AvailableWidgets.Contains(EWidgetType::PlayerHUD))
	{
		BindPlayerHUD(aPawn);

		InitializePlayerHUD();
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
			AvailableWidgets.Add(WidgetBase->GetWidgetType(), WidgetBase);
			COMMON_LOG(LogGameplay, Log, TEXT("Saved widget name : %s"), *WidgetBase->GetClass()->GetName());
		}
	}

	for (auto& [WidgetType, Widget] : AvailableWidgets)
	{
		Widget->AddToViewport();
	}

	ShowDefaultWidget(CurrentPhaseWidgetClasses->DefaultWidget);
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

void UUIManagerComponent::BindPlayerHUD(APawn* Pawn)
{
	UnbindPlayerHUD(Pawn);

	if (!Pawn)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("BindToPawn failed. Pawn is nullptr."));
		return;
	}

	TObjectPtr<UPlayerHUD> PlayerHUD = Cast<UPlayerHUD>(AvailableWidgets.FindRef(EWidgetType::PlayerHUD));
	if (!PlayerHUD)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("PlayerHUD is nullptr."));
		return;
	}

	if (UCombatComponent* CombatComp = Pawn->FindComponentByClass<UCombatComponent>())
	{
		CombatComp->OnAmmoChanged.AddUObject(PlayerHUD, &UPlayerHUD::SetAmmo);
		CombatComp->OnCurrentWeaponChanged.AddUObject(PlayerHUD, &UPlayerHUD::SetWeapon);
	}

	if (UInventoryComponent* InventoryComp = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		// QuickSlot 변경 델리게이트에 PlayerHUD 함수 등록.
	}

	// 나중에 StatComponent. 상태 관리 컴포넌트 같은 것도 필요함.
}

void UUIManagerComponent::UnbindPlayerHUD(APawn* Pawn)
{
	if (!Pawn)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("BindToPawn failed. Pawn is nullptr."));
		return;
	}

	TObjectPtr<UPlayerHUD> PlayerHUD = Cast<UPlayerHUD>(AvailableWidgets.FindRef(EWidgetType::PlayerHUD));
	if (!PlayerHUD)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("PlayerHUD is nullptr."));
		return;
	}

	if (UCombatComponent* CombatComp = Pawn->FindComponentByClass<UCombatComponent>())
	{
		CombatComp->OnAmmoChanged.RemoveAll(PlayerHUD);
		CombatComp->OnCurrentWeaponChanged.RemoveAll(PlayerHUD);
	}

	if (UInventoryComponent* InventoryComp = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		// QuickSlot 변경 델리게이트에 등록된 PlayerHUD 함수 제거
	}
}

void UUIManagerComponent::InitializePlayerHUD()
{
	APawn* Pawn = OwnerController->GetPawn();
	if (!Pawn) return;

	if (UCombatComponent* CombatComp = Pawn->FindComponentByClass<UCombatComponent>())
	{
		CombatComp->NotifyCurrentWeaponState();
	}
}

