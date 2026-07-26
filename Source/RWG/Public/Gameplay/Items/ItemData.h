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

	/*
	아이템 식별자 - 매칭 / 로직용
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FName ItemID;

	/* 표시 텍스트 (로컬라이제이션 대상) - UI 전용, 식별자로 쓰지 말 것 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	/* 표시 텍스트 (로컬라이제이션 대상) - UI 전용, 식별자로 쓰지 말 것 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	EItemType ItemType = EItemType::None;

	// 슬롯에 저장 가능한 최대 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stack")
	int32 MaxStackSize = 1;
};
