// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemDataRegistry.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnRegistryReady);

class UItemData;

/**
 * 
 */
UCLASS()
class RWG_API UItemDataRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

/*********************************************************************
*                             애셋 로딩
*********************************************************************/
private:
	const FPrimaryAssetType AssetType = FPrimaryAssetType(FName("Item"));

	bool bIsReady = false;

protected:
	void OnItemAssetsLoaded();

public:
	FOnRegistryReady OnRegistryReady;

	bool IsReady() const;

/*********************************************************************
*                            아이템 조회
*********************************************************************/
private:
	TMap<FName /*ItemID*/, TObjectPtr<UItemData>> CachedItems;

public:
	UItemData* FindItemDataByID(FName ID) const;
};
