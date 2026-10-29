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
*                                복제
*********************************************************************/
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                           아이템 데이터
*********************************************************************/
protected:
	UPROPERTY(EditAnywhere, Replicated, Category = "Item")
	TObjectPtr<UItemData> ItemData;

	// 아이템 데이터의 외형을 메시 컴포넌트에 적용한다
	virtual void ApplyItemData();

public:
	template<typename T>
	T* GetItemData() const { return Cast<T>(ItemData); }

	// FinishSpawning 전에 호출해야 BeginPlay에서 외형이 적용된다
	void SetItemData(UItemData* NewItemData);

/*********************************************************************
*                              상호작용
*********************************************************************/
protected:
	virtual void Interact(AActor* Interactor) override;
};
