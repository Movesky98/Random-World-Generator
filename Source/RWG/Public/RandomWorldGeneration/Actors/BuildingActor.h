// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingActor.generated.h"

class UBoxComponent;

UCLASS()
class RWG_API ABuildingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildingActor();

	const TArray<UBoxComponent*>& GetItemSpawnVolumes() const { return ItemSpawnVolumes; }

protected:
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ExteriorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TArray<TObjectPtr<UBoxComponent>> ItemSpawnVolumes;
};
