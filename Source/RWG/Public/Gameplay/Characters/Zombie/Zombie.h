// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CharacterBase/CharacterBase.h"
#include "Zombie.generated.h"

class UBehaviorTree;

DECLARE_DELEGATE(FOnAttackFinished)

DECLARE_MULTICAST_DELEGATE_OneParam(FOnZombieDeath, AZombie* /* self */);

/**
 * 
 */
UCLASS()
class RWG_API AZombie : public ACharacterBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AZombie();

protected:
	virtual void BeginPlay() override;

/*********************************************************************
*                                 AI
*********************************************************************/
public:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

/*********************************************************************
*                                추격
*********************************************************************/
public:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float CloseChaseDistance = 400.0f;

/*********************************************************************
*                                공격
*********************************************************************/
protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage(int32 MontageIndex);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackDamage = 20.f;

public:
	FOnAttackFinished OnAttackFinished;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackDistance = 150.0f;

	void RequestAttack();

	void PerformAttack();

/*********************************************************************
*                                사망
*********************************************************************/
protected:
	UFUNCTION()
	void OnDeath(const FDamageInfo& DamageInfo);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathMontage();

	void EnableRagdoll();

public:
	FOnZombieDeath OnZombieDeath;

/*********************************************************************
*                           담당 플레이어
*********************************************************************/
protected:
	TObjectPtr<ACharacterBase> AssignedCharacter;

public:
	void InitializeZombie(ACharacterBase* InAssignedCharacter);

	ACharacterBase* GetAssignedCharacter() const;
};
