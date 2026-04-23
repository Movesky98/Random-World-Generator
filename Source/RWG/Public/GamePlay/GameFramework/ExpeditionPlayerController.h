// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/GameFramework/PlayerControllerBase.h"
#include "ExpeditionPlayerController.generated.h"

class UInventoryWidget;

/**
 * 
 */
UCLASS()
class RWG_API AExpeditionPlayerController : public APlayerControllerBase
{
	GENERATED_BODY()
public:
	AExpeditionPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* aPawn) override;
	
	virtual void OnUnPossess() override;

	// Inventory Component Section - Test //
	void HandleInventoryToggled();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UInputHandlerComponent* InputHandlerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UInventoryWidget* InventoryWidget;

	bool bIsInventoryOpen = false;
};
