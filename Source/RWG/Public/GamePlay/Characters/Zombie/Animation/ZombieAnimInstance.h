// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Characters/CharacterBase/Animations/CharacterBaseAnimInstance.h"
#include "ZombieAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UZombieAnimInstance : public UCharacterBaseAnimInstance
{
	GENERATED_BODY()
	
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
