// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GlobalUISubsystem.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS()
class RWG_API UGlobalUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	void ShowLoadingScreen();
	void HideLoadingScreen();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingScreen = nullptr;
};
