// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterBaseAnimInstance.generated.h"

class ACharacterBase;
class UCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class RWG_API UCharacterBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	UCharacterBaseAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

/*********************************************************************
*                           로코모션 상태
*********************************************************************/
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "State")
	bool bIsInAir;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "State")
	bool bShouldMove;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "State")
	float Direction;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "State")
	float Speed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "State")
	float MoveThreshold = 3.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FVector Velocity;

public:
	bool ShouldMove() const { return bShouldMove; }
};
