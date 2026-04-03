// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "SessionSlot.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSessionSlot, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinButtonClickedEvent, int32 SessionIndex);


struct FSessionData
{
public:
	int32 SessionIndex;

	int32 MaxPlayers;
	
	int32 CurrentPlayers;
	
	FName SessionName;
	
	int32 Ping;

	FSessionData() : SessionIndex(-1), MaxPlayers(-1), CurrentPlayers(-1), SessionName(""), Ping(-1)
	{ }
};

/**
 * 
 */
UCLASS()
class RWG_API USessionSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	FOnJoinButtonClickedEvent OnJoinButtonClickedEvent;
	
	void InitializeSessionData(const FSessionData InSessionData);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnJoinButtonClicked();
	
private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* SessionNameText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* PlayerCountText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* PingText;

	UPROPERTY(meta=(BindWidget))
	class UButton* JoinButton;

	FSessionData SessionData;
};
