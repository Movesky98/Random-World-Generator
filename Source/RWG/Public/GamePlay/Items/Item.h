// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GamePlay/Interfaces/Interactable.h"
#include "GamePlay/Items/ItemData.h"
#include "Item.generated.h"

UCLASS()
class RWG_API AItem : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Interact(APawn* Interactor) override;

	UPROPERTY(EditInstanceOnly, Category = "Item")
	TObjectPtr<UItemData> ItemData;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	int32 DropQuantity = 1;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
