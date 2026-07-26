// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/UI/GameplayWidget.h"
#include "GameOverWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class RWG_API UGameOverWidget : public UGameplayWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnReturnToLobbyButtonClicked();

private:
	UPROPERTY(meta = (BindWidget))	
	TObjectPtr<UTextBlock> ResultTitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToLobbyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmRewardButton;
};
