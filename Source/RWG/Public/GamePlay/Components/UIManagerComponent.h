// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIManagerComponent.generated.h"

enum class EWidgetType : uint8;
class UUserWidgetBase;

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
	TArray<TSubclassOf<UUserWidgetBase>> WidgetClasses;

	/* Phase에서 기본적으로 보여줄 위젯 */
	UPROPERTY(EditDefaultsOnly)
	EWidgetType DefaultWidget;
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

protected:
	void DisplayWidgetsForPhase();

	void InitializeLevelNameMap();

	void CreateWidgetsForPhase(EGamePhase Phase);

	EGamePhase ConvertLevelNameToPhase(FName LevelName);

	void ShowDefaultWidget(EWidgetType DefaultType);

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
	TMap<EWidgetType, TObjectPtr<UUserWidgetBase>> AvailableWidgets;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<UUserWidgetBase> CurrentWidget;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	EGamePhase CurrentPhase;
};
