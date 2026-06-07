// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ExpeditionGameMode.generated.h"

class USpawnDirectorComponent;
class UDayNightComponent;

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpawnDirectorComponent> SpawnDirectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UDayNightComponent> DayNightComponent;
};
