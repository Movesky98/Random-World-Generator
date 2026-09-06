// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/UserWidgetBase.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "SessionMenu.generated.h"

UENUM()
enum class ESessionUIError : uint8
{
	InvalidState,
	SessionInterfaceNULL,
	CreateFailed,
	FindFailed,
	JoinFailed,
};

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

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	USessionMenu();

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

/*********************************************************************
*                          서브시스템 조회
*********************************************************************/
protected:
	class USessionSubsystem* GetSessionSubsystem() const;

/*********************************************************************
*                                상태
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	class UCircularThrobber* LoadingThrobber;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session|State", meta = (AllowPrivateAccess = "true"))
	ESessionState SessionState;

protected:
	void ResetUIToIdle();

	void HandleError(ESessionUIError Error);

/*********************************************************************
*                             세션 생성
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MaxPlayersText;

	UPROPERTY(meta = (BindWidget))
	class USlider* MaxPlayersSlider;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* SessionNameText;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* LANCheckBox;

	UPROPERTY(meta = (BindWidget))
	class UButton* CreateSessionButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Session|State", meta = (AllowPrivateAccess = "true"))
	bool bIsLAN;

protected:
	UFUNCTION()
	void OnSliderValueChanged(float Value);

	UFUNCTION()
	void OnCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnCreateSessionButtonClicked();

	void OnCreateSessionCompleted(bool bWasSuccessful);

/*********************************************************************
*                             세션 검색
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	class UButton* FindSessionsButton;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* SessionScrollBox;

	TArray<FOnlineSessionSearchResult> FindSessionsResults;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Session|UI")
	TSubclassOf<class USessionSlot> SessionSlotClass;

	UFUNCTION()
	void OnFindSessionButtonClicked();

	void OnFindSessionsCompleted(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);

	void DisplaySessionList(const TArray<FOnlineSessionSearchResult>& SessionResults);

/*********************************************************************
*                             세션 참가
*********************************************************************/
protected:
	UFUNCTION()
	void OnJoinButtonClicked(int32 SessionIndex);

	void OnJoinSessionCompleted(EOnJoinSessionCompleteResult::Type Result);
};
