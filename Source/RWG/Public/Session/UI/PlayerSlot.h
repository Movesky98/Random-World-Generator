// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Session/GameFramework/LobbyPlayerState.h"
#include "PlayerSlot.generated.h"

class UTextBlock;
class UCheckBox;

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerSlot, Log, All);

/**
 * 
 */
UCLASS()
class RWG_API UPlayerSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	void BindPlayerState(ALobbyPlayerState* LobbyPS);

	bool IsReady() const;
	
protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	void HandleReadyChanged(bool IsReady);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* ReadyCheckBox;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<ALobbyPlayerState> LinkedPlayerState;
};
