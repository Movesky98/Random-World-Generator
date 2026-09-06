// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gameplay/Interfaces/Damageable.h"
#include "CharacterBase.generated.h"

class ULocomotionComponent;
class UCombatComponent;
class UInventoryComponent;
class UInteractionComponent;
class UHealthComponent;

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class RWG_API ACharacterBase : public ACharacter, public IDamageable
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this character's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

/*********************************************************************
*                                피해
*********************************************************************/
protected:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void ProcessDamage(const FDamageInfo& DamageInfo) override;
};
