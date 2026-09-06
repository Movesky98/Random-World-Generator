// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Components/InputComponentBase.h"
#include "LocomotionComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UConvictAnimInstance;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API ULocomotionComponent : public UInputComponentBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this component's properties
	ULocomotionComponent();

protected:
	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	UPROPERTY()
	TObjectPtr<UConvictAnimInstance> AnimInstance;

/*********************************************************************
*                             입력 처리
*********************************************************************/
protected:
	TSubclassOf<UInputConfigBase> GetConfigClass() override;

	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

public:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Jump();

/*********************************************************************
*                            제자리 회전
*********************************************************************/
protected:
	/**
	 * 정지 상태에서 조준 방향과 몸의 각도 차가 TurnThreshold를 넘으면 몸을 그쪽으로 돌린다.
	 * 이동 중에는 CharacterMovement가 컨트롤 회전을 따라가게 맡긴다.
	 */
	void UpdateTurnInPlace(float DeltaTime);

	/** 제자리 회전 진입 임계값. AO_TR15_Idle의 Yaw 축 범위와 맞출 것 */
	UPROPERTY(EditDefaultsOnly, Category = "Turn In Place")
	float TurnThreshold = 90.0f;

	/** 제자리 회전 속도 (도/초) */
	UPROPERTY(EditDefaultsOnly, Category = "Turn In Place")
	float TurnRate = 360.0f;

	/** 회전 중인지. 회전 중에는 TargetYaw를 갱신하지 않는다 */
	bool bIsTurningInPlace = false;

	/** 진입 시점에 한 번 정하는 목표 Yaw */
	float TargetYaw = 0.0f;
};
