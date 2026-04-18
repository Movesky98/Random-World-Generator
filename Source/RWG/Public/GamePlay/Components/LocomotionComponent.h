// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LocomotionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API ULocomotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULocomotionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void Move(const FVector2D& InputValue);

	void Look(const FVector2D& InputValue);

	void Jump();
};
