// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputHandlerComponent.generated.h"

class UBaseInputConfig;
class ULocomotionComponent;

namespace InputConfgTags
{
	const FName BaseName = FName("InputConfig");
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UInputHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInputHandlerComponent();

	void BindInputs(class UEnhancedInputComponent* EnhancedInputComponent);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UBaseInputConfig* BaseInputConfig;

	UPROPERTY()
	APlayerController* OwnerController;

	ULocomotionComponent* GetLocomotionComponent();

	void HandleMove(const FInputActionValue& Value);

	void HandleLook(const FInputActionValue& Value);

	void HandleJump();
	
	void HandleAttack();
	
	void HandleInteract();
};
