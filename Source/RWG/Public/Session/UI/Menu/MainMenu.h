// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"
#include "MainMenu.generated.h"

class UButton;

/**
 *
 */
UCLASS()
class RWG_API UMainMenu : public UUserWidgetBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void NativeConstruct() override;

/*********************************************************************
*                                버튼
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

protected:
	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();
};
