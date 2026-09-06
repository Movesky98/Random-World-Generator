// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Interfaces/PlayerDetectable.h"
#include "BuildingActor.generated.h"

class UBoxComponent;
class AItem;

DECLARE_MULTICAST_DELEGATE(FOnPlayerEntered);
DECLARE_MULTICAST_DELEGATE(FOnPlayerExited);

UCLASS()
class RWG_API ABuildingActor : public AActor, public IPlayerDetectable
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this actor's properties
	ABuildingActor();

protected:
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ExteriorMesh;

/*********************************************************************
*                          아이템 스폰 볼륨
*********************************************************************/
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<TObjectPtr<UBoxComponent>> ItemSpawnVolumes;

	const TArray<UBoxComponent*>& GetItemSpawnVolumes() const { return ItemSpawnVolumes; }

/*********************************************************************
*                         플레이어 접근 감지
*********************************************************************/
private:
	int32 PlayersInside = 0;

public:
	FOnPlayerEntered OnPlayerEntered;

	FOnPlayerExited OnPlayerExited;

	virtual void OnPlayerApproach(APawn* Player) override;

	virtual void OnPlayerLeave(APawn* Player) override;
};
