// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,
	Weapon,
	Consumable,
	Material,
};

/**
 * 
 */
UCLASS(BlueprintType)
class RWG_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Item", GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	EItemType ItemType = EItemType::None;

	/* 인벤토리에서 같은 아이템을 한 슬롯에 쌓을 최대 개수의 수*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack")
	int32 MaxStackSize = 1;
};
