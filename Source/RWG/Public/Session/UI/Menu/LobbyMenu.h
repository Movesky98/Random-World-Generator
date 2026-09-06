// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"
#include "LobbyMenu.generated.h"

class UTextBlock;
class UScrollBox;
class UButton;

/**
 * 
 */
UCLASS()
class RWG_API ULobbyMenu : public UUserWidgetBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	ULobbyMenu();

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

/*********************************************************************
*                         게임 스테이트 조회
*********************************************************************/
protected:
	class ALobbyGameState* GetLobbyGameState();

/*********************************************************************
*                           로비 정보 표시
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SessionNameText;

protected:
	void InitializeLobbyView();

/*********************************************************************
*                           플레이어 목록
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerListScrollBox;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<class UPlayerSlot> PlayerSlotClass;

	void RefreshPlayerList();

	void RefreshPlayerCount();

/*********************************************************************
*                                버튼
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;

protected:
	UFUNCTION()
	void ExitSession();

	UFUNCTION()
	void OnReadyButtonClicked();
};
