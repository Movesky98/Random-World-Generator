// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"
#include "LobbyMenu.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLobbyMenu, Log, All);

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
	
public:

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	void InitializeLobbyView();

	class ALobbyGameState* GetLobbyGameState();

	void RefreshPlayerList();

	void RefreshPlayerCount();
	
	UFUNCTION()
	void ExitSession();

	UFUNCTION()
	void OnClickedReadyButton();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<class UPlayerSlot> PlayerSlotClass;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SessionNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* PlayerListScrollBox;
	
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;
};
