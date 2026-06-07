// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GamePlay/Interfaces/PlayerDetectable.h"
#include "BuildingActor.generated.h"

class UBoxComponent;
class AItem;

DECLARE_MULTICAST_DELEGATE(FOnPlayerEntered);
DECLARE_MULTICAST_DELEGATE(FOnPlayerExited);

UCLASS()
class RWG_API ABuildingActor : public AActor, public IPlayerDetectable
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

	// Item
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<TObjectPtr<UBoxComponent>> ItemSpawnVolumes;

	FOnPlayerEntered OnPlayerEntered;
	FOnPlayerExited OnPlayerExited;

private:
	int32 PlayersInside = 0;

public:
	virtual void OnPlayerApproach(APawn* Player) override;
	virtual void OnPlayerLeave(APawn* Player) override;
};
