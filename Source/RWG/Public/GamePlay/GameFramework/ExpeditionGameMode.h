// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "ExpeditionGameMode.generated.h"

class USpawnDirectorComponent;
class UTimeManagementComponent;
/**
 * 
 */
UCLASS()
class RWG_API AExpeditionGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AExpeditionGameMode();

protected:
	virtual void BeginPlay() override;

	/* Pawn을 스폰하고 PlayerController에 Possess시키는 함수 */
	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpawnDirectorComponent> SpawnDirectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UTimeManagementComponent> TimeManagementComponent;
};
