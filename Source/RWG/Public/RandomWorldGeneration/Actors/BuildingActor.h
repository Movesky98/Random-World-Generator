// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingActor.generated.h"

UCLASS()
class RWG_API ABuildingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
	TObjectPtr<UStaticMesh> BuildingMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ExteriorMesh;

	virtual void PostActorCreated() override;
};
