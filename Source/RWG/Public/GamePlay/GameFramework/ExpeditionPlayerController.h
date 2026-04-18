// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/GameFramework/PlayerControllerBase.h"
#include "ExpeditionPlayerController.generated.h"

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

	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UInputHandlerComponent* InputHandlerComponent;
};
