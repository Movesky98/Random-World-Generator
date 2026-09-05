// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/Convict/Animation/ConvictAnimInstance.h"
#include "GunAnimInstance.generated.h"

class UCombatComponent;

/**
 * 
 */
UCLASS()
class RWG_API UGunAnimInstance : public UConvictAnimInstance
{
	GENERATED_BODY()
	
protected:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	FTransform HandGuardTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	bool bIsHandGuardValid = false;
};
