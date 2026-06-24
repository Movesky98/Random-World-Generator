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
	
public:
	UItemData* FindItemDataByID(FName ID) const;

	FOnRegistryReady OnRegistryReady;

	bool IsReady() const;
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void OnItemAssetsLoaded();

private:
	TMap<FName /*ItemID*/, TObjectPtr<UItemData>> CachedItems;

	const FPrimaryAssetType AssetType = FPrimaryAssetType(FName("Item"));

	bool bIsReady = false;
};
