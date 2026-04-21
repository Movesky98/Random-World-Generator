// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Components/BaseInputComponent.h"
#include "LocomotionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API ULocomotionComponent : public UBaseInputComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULocomotionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TSubclassOf<UBaseInputConfig> GetConfigClass() override;

	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

public:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Jump();
};
