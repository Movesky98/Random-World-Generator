// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Common/CommonDelegates.h"
#include "CountdownWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class RWG_API UCountdownWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeCountdown(const FText& InTitle, const FGetRemainingSecondsDelegate& InDelegate);
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountdownText;

	FGetRemainingSecondsDelegate RemainingSecondsProvider;

	int32 LastSecond = INDEX_NONE;
};
