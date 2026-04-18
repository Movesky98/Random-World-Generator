// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterBaseAnimInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCharacterAnimation, Log, All);

class ACharacterBase;
class UCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class RWG_API UCharacterBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UCharacterBaseAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
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

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
};
