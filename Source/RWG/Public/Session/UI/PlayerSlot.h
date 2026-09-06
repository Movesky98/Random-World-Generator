// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Session/GameFramework/LobbyPlayerState.h"
#include "PlayerSlot.generated.h"

class UTextBlock;
class UCheckBox;

/**
 * 
 */
UCLASS()
class RWG_API UPlayerSlot : public UUserWidget
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

/*********************************************************************
*                           플레이어 표시
*********************************************************************/
private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<ALobbyPlayerState> LinkedPlayerState;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* ReadyCheckBox;

	void HandleReadyChanged(bool IsReady);

public:
	void BindPlayerState(ALobbyPlayerState* LobbyPS);

	bool IsReady() const;
};
