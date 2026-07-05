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

	void NotifyWorldGenReady();
	void NotifyHUDReady();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void TryHideLoadingScreen();

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingScreen = nullptr;

	bool bWorldGenReady = false;
	bool bHUDReady = false;
};
