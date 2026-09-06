// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "ExpeditionGameMode.generated.h"

class USpawnDirectorComponent;
class UTimeManagementComponent;
class AConvict;
class UItemData;
/**
 * 
 */
UCLASS()
class RWG_API AExpeditionGameMode : public AGameModeBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AExpeditionGameMode();

protected:
	virtual void BeginPlay() override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpawnDirectorComponent> SpawnDirectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UTimeManagementComponent> TimeManagementComponent;

/*********************************************************************
*                          플레이어 입퇴장
*********************************************************************/
protected:
	/* Pawn을 스폰하고 PlayerController에 Possess시키는 함수 */
	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

/*********************************************************************
*                             게임 시작
*********************************************************************/
protected:
	TSet<TWeakObjectPtr<APlayerState>> ReportedPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	float PrepareGameTime = 5.0f;

	void OnWorldReady();

	void TryGameStart(APlayerState* IgnorePS = nullptr);

	void PrepareGameStart();

	void GameStart();

public:
	void ReportWorldGenerationCompleted(APlayerController* Player);

/*********************************************************************
*                             탈출 조건
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Extraction")
	TObjectPtr<UDataTable> ExtractionConditionTable;

	// 탈출 조건 등록 임시 방편
	void SubscribeInventoryComponent(AConvict* Player);

	void UnsubscribeInventoryComponent(AConvict* Player);

	void OnInventoryItemAdded(UItemData* ItemData, int32 Quantity);

public:
	void InitializeExtractionConditions();

/*********************************************************************
*                             게임 종료
*********************************************************************/
protected:
	FTimerHandle ReturnToLobbyTimerHandle;

	void GameOver();

	void ReturnToLobby();
};
