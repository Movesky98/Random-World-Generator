// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"
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
	
public:
    AZombie();

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float CloseChaseDistance = 400.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float AttackDistance = 150.0f;

protected:
    virtual void BeginPlay() override;

public:
    FOnAttackFinished OnAttackFinished;

    void RequestAttack();

    void PerformAttack();

protected:
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttackMontage(int32 MontageIndex);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDeathMontage();

    void EnableRagdoll();

    UFUNCTION()
    void OnDeath(const FDamageInfo& DamageInfo);

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float AttackDamage = 20.f;

public:
    FOnZombieDeath OnZombieDeath;

    void InitializeZombie(ACharacterBase* InAssignedCharacter);

    ACharacterBase* GetAssignedCharacter() const;

protected:
    TObjectPtr<ACharacterBase> AssignedCharacter;
};
