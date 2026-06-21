// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemDataRegistry.generated.h"

class UItemData;

/**
 * 
 */
UCLASS()
class RWG_API UItemDataRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UItemData* FindItemDataByID(FName ID) const;
	
private:
	TMap<FName /*ItemID*/, TObjectPtr<UItemData>> CachedItems;
};
