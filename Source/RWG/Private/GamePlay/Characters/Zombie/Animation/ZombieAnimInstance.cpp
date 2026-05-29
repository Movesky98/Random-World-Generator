// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Characters/Zombie/Animation/ZombieAnimInstance.h"

void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	bShouldMove = Speed > MoveThreshold;
}
