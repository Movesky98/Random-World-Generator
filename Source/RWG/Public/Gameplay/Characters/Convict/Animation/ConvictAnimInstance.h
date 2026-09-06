// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CharacterBase/Animation/CharacterBaseAnimInstance.h"
#include "ConvictAnimInstance.generated.h"

/**
 * 조준 각도를 다루는 계층. 좀비는 AimOffset을 쓰지 않으므로 CharacterBase가 아니라 여기에 둔다.
 */
UCLASS()
class RWG_API UConvictAnimInstance : public UCharacterBaseAnimInstance
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

/*********************************************************************
*                             상체 조준
*********************************************************************/
protected:
	/** 몸 기준으로 얼마나 더 좌우로 돌려 보는가 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float AimYaw;

	/** 몸 기준으로 얼마나 더 위아래로 돌려 보는가 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float AimPitch;

public:
	float GetAimYaw() const { return AimYaw; }
};
