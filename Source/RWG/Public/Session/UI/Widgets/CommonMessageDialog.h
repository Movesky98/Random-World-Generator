// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonMessageDialog.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UCommonMessageDialog : public UUserWidget
{
	GENERATED_BODY()
	
	
private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text;

	UPROPERTY(meta=(BindWidget))
	class UButton* Button;
};
