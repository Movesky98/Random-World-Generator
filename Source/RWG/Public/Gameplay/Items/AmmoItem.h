// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Item.h"
#include "AmmoItem.generated.h"

/**
 *
 */
UCLASS()
class RWG_API AAmmoItem : public AItem
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AAmmoItem();

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

public:
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

/*********************************************************************
*                           아이템 데이터
*********************************************************************/
protected:
	virtual void ApplyItemData() override;
};
