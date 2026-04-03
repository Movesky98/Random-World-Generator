// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "SessionMenu.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSessionMenu, Log, All);

UENUM(BlueprintType)
enum class ESessionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Creating UMETA(DisplayName = "Creating"),
	Finding UMETA(DisplayName = "Finding"),
	Joining UMETA(DisplayName = "Joining")
};

/**
 * 
 */
UCLASS()
class RWG_API USessionMenu : public UUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	class USessionSubsystem* GetSessionSubsystem() const;

	/* Create Session */
	UFUNCTION()
	void OnCreateSessionButtonClicked();

	void OnCreateSessionCompleted(bool bWasSuccessful);

	/* Find Sessions */
	UFUNCTION()
	void OnFindSessionButtonClicked();

	void OnFindSessionsCompleted(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);

	void DisplaySessionList(const TArray<FOnlineSessionSearchResult>& SessionResults);
	
	/* Join Session */
	UFUNCTION()
	void OnJoinButtonClicked(int32 SessionIndex);
	
	void OnJoinSessionCompleted(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnSliderValueChanged(float Value);

	UFUNCTION()
	void OnCheckStateChanged(bool bIsChecked);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Session|UI")
	TSubclassOf<class USessionSlot> SessionSlotClass;

private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* MaxPlayersText;

	UPROPERTY(meta=(BindWidget))
	class USlider* MaxPlayersSlider;

	UPROPERTY(meta=(BindWidget))
	class UEditableTextBox* SessionNameText;

	UPROPERTY(meta=(BindWidget))
	class UCheckBox* LANCheckBox;

	UPROPERTY(meta=(BindWidget))
	class UButton* CreateSessionButton;

	UPROPERTY(meta=(BindWidget))
	class UButton* FindSessionsButton;

	UPROPERTY(meta=(BindWidget))
	class UScrollBox* SessionScrollBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session|State", meta = (AllowPrivateAccess = "true"))
	bool bIsLAN;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session|State", meta = (AllowPrivateAccess = "true"))
	ESessionState SessionState;

	TArray<FOnlineSessionSearchResult> FindSessionsResults;
};
