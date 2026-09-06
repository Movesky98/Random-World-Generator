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

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AExpeditionPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* aPawn) override;

	virtual void AcknowledgePossession(APawn* aPawn) override;

	virtual void OnUnPossess() override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UUIManagerComponent* UIManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UInputHandlerComponent* InputHandlerComponent;

/*********************************************************************
*                             게임 진행
*********************************************************************/
protected:
	UFUNCTION(Server, Reliable)
	void Server_RequestReturnToLobby();

	UFUNCTION(Server, Reliable)
	void Server_ReportWorldGenerationCompleted();

	void HandleGameOver();

	void OnGameplayStateChanged(EGameplayState GameplayState);

public:
	void RequestReturnToLobby();
};
