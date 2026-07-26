// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Zombie/Animation/ZombieAnimInstance.h"

void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	bShouldMove = Speed > MoveThreshold;
}

void UZombieAnimInstance::PlayAttackMontage(int32 MontageIndex)
{
	if (!AttackMontages.IsValidIndex(MontageIndex)) return;
	Montage_Play(AttackMontages[MontageIndex]);
}

void UZombieAnimInstance::PlayDeathMontage()
{
	if (!DeathMontage) return;
	Montage_Play(DeathMontage);
}
