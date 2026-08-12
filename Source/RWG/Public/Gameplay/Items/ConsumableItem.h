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
public:
	AConsumableItem();
	
public:
	UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

};
