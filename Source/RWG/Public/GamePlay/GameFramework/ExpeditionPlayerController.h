// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/GameFramework/PlayerControllerBase.h"
#include "ExpeditionPlayerController.generated.h"

enum class EGameplayState : uint8;

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
	virtual void AcknowledgePossession(APawn* aPawn) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UUIManagerComponent* UIManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UInputHandlerComponent* InputHandlerComponent;

	////////////////////////////// GameplayState 관련 //////////////////////////////
public:
	void RequestReturnToLobby();

protected:
	UFUNCTION(Server, Reliable)
	void Server_RequestReturnToLobby();

	UFUNCTION(Server, Reliable)
	void Server_ReportWorldGenerationCompleted();

	void HandleGameOver();

	void OnGameplayStateChanged(EGameplayState GameplayState);
};
