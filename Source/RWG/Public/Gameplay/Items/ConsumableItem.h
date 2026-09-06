// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Item.h"
#include "ConsumableItem.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API AConsumableItem : public AItem
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AConsumableItem();

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

public:
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }
};
