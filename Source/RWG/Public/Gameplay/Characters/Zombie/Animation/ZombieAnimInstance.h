// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CharacterBase/Animation/CharacterBaseAnimInstance.h"
#include "ZombieAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UZombieAnimInstance : public UCharacterBaseAnimInstance
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

/*********************************************************************
*                            몽타주 재생
*********************************************************************/
public:
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DeathMontage;

	void PlayAttackMontage(int32 MontageIndex);

	void PlayDeathMontage();
};
