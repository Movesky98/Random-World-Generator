// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerControllerBase, Log, All);

UENUM(BlueprintType)
enum class EUIType : uint8
{
	SessionMenu UMETA(DisplayName = "Session Menu"),
	LobbyMenu UMETA(DisplayName = "Lobby Menu"),
	PlayerHUD UMETA(DisplayName = "Player HUD")
};

/**
 * 
 */
UCLASS()
class RWG_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdateUIForCurrentLevel();

	EUIType ConvertLevelNameToEnum(FString LevelName);

	void ShowUI(EUIType UIType);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TMap<EUIType, TSubclassOf<UUserWidgetBase>> UIClassMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UUserWidgetBase* CurrentWidget;

public:
	void RequestSetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);
};
