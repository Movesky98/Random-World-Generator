// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CharacterBase/CharacterBase.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Convict.generated.h"

class ULocomotionComponent;
class UCombatComponent;
class UInventoryComponent;
class UInteractionComponent;
class UHealthComponent;

class USpringArmComponent;
class UCameraComponent;
class USphereComponent;

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
	virtual void PostInitializeComponents() override;

	virtual void ProcessDamage(const FDamageInfo& DamageInfo) override;
	
protected:
	UFUNCTION()
	void OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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

	UPROPERTY(VisibleAnywhere, Category = "Detection")
	TObjectPtr<USphereComponent> ProximityDetector;
};
