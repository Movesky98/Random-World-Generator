// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIManagerComponent.generated.h"

class UUserWidgetBase;
class IWidgetBindable;

UENUM(BlueprintType)
enum class EUIType : uint8
{
	None UMETA(DisplayName = "None"),
	SessionMenu UMETA(DisplayName = "Session Menu"),
	LobbyMenu UMETA(DisplayName = "Lobby Menu"),
	PlayerHUD UMETA(DisplayName = "Player HUD"),
	Inventory UMETA(DisplayName = "Inventory"),
	GameOver UMETA(DisplayName = "GameOver"),
};

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	None,		// 테스트용
	Session,
	Lobby,
	Gameplay
};

USTRUCT(BlueprintType)
struct FWidgetClassList
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TMap<EUIType, TSubclassOf<UUserWidgetBase>> WidgetClasses;

	/* Phase에서 기본적으로 보여줄 위젯 */
	UPROPERTY(EditDefaultsOnly)
	EUIType DefaultWidgetType;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UUIManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UUIManagerComponent();

protected:
	void InitializeComponent() override;

	virtual void BeginPlay() override;

	/* Initialize Settings */
public:
	void InitializePawnWidgets(APawn* aPawn);

	void BindDelegatesFromGameplayWidgets(const TArray<TScriptInterface<IWidgetBindable>>& BindableComponents);

	void UnbindDelegatesFromGameplayWidgets(const TArray<TScriptInterface<IWidgetBindable>>& BindableComponents);

protected:
	void DisplayWidgetsForPhase();

	void InitializeLevelNameMap();

	void CreateWidgetsForPhase(EGamePhase Phase);

	EGamePhase ConvertLevelNameToPhase(FName LevelName);

	void ShowWidget(EUIType WidgetType);

	/* 에디터 설정용 TMap */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TMap<TSoftObjectPtr<UWorld>, EGamePhase> LevelToPhaseMap;

	/* 런타임 조회용 TMap */
	TMap<FName, EGamePhase> LevelNameToPhaseMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TMap<EGamePhase, FWidgetClassList> WidgetClassMap;

private:
	APlayerController* OwnerController;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	TMap<EUIType, TObjectPtr<UUserWidgetBase>> AvailableWidgets;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<UUserWidgetBase> CurrentWidget;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	EGamePhase CurrentPhase;

	////////////////////////////// Game Over 관련 //////////////////////////////
public:
	void ShowGameOverWidget();
};
