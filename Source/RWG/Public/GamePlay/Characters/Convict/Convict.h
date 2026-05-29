// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Convict.generated.h"

class ULocomotionComponent;
class UCombatComponent;
class UInventoryComponent;
class UInteractionComponent;
class UHealthComponent;

class USpringArmComponent;
class UCameraComponent;

/**
 * 
 */
UCLASS()
class RWG_API AConvict : public ACharacterBase
{
	GENERATED_BODY()
public:
	AConvict();

protected:
	virtual void ProcessDamage(const FDamageInfo& DamageInfo) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	ULocomotionComponent* LocomotionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UCombatComponent* CombatComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UInteractionComponent* InteractionComponent;

	UPROPERTY()
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;
};
