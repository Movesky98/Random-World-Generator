// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/CommonDelegates.h"

#include "GlobalUISubsystem.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS()
class RWG_API UGlobalUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

/*********************************************************************
*                            로딩 스크린
*********************************************************************/
protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingScreen = nullptr;

	bool bWorldGenReady = false;
	bool bHUDReady = false;

	void TryHideLoadingScreen();

public:
	void ShowLoadingScreen();
	void HideLoadingScreen();

	void NotifyWorldGenReady();
	void NotifyHUDReady();

/*********************************************************************
*                             카운트다운
*********************************************************************/
protected:
	UPROPERTY()
	TObjectPtr<class UCountdownWidget> CountdownWidget = nullptr;

public:
	void ShowCountdownWidget(const FText& InTitle, const FGetRemainingSecondsDelegate& InDelegate);
	void HideCountdownWidget();
};
