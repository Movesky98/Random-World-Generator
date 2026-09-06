// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Interfaces/Interactable.h"
#include "Gameplay/DataAssets/ItemData.h"
#include "Item.generated.h"

UCLASS(Abstract)
class RWG_API AItem : public AActor, public IInteractable
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

/*********************************************************************
*                           아이템 데이터
*********************************************************************/
protected:
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UItemData> ItemData;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	int32 DropQuantity = 1;

public:
	template<typename T>
	T* GetItemData() const { return Cast<T>(ItemData); }

/*********************************************************************
*                              상호작용
*********************************************************************/
protected:
	virtual void Interact(AActor* Interactor) override;
};
