// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/Convict/Animation/ConvictAnimInstance.h"
#include "GunAnimInstance.generated.h"

class UCombatComponent;
class UAnimSequence;

/**
 * 
 */
UCLASS()
class RWG_API UGunAnimInstance : public UConvictAnimInstance
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TObjectPtr<UCombatComponent> CombatComponent;

/*********************************************************************
*                              왼손 IK
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	FTransform HandGuardTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	bool bIsHandGuardValid = false;

/*********************************************************************
*                             무기 포즈
*********************************************************************/
protected:
	/** 현재 무기를 든 상체 정지 포즈 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pose")
	TObjectPtr<UAnimSequence> IdlePose;
};
