// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingActor.generated.h"

class UBoxComponent;
class AItem;

DECLARE_MULTICAST_DELEGATE(FOnPlayerEntered);
DECLARE_MULTICAST_DELEGATE(FOnPlayerExited);

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

	// Item
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<TObjectPtr<UBoxComponent>> ItemSpawnVolumes;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TObjectPtr<UBoxComponent> ProximityTrigger;

	FOnPlayerEntered OnPlayerEntered;
	FOnPlayerExited OnPlayerExited;

private:
	int32 PlayersInside = 0;

	UFUNCTION()
	void OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
