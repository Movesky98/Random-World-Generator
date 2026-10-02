// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RecoilComponent.generated.h"

class ACharacter;

/**
 * 사격 반동을 소유 클라의 컨트롤 로테이션에 적용한다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API URecoilComponent : public UActorComponent
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	URecoilComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
private:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

/*********************************************************************
*                             반동 적용
*********************************************************************/
private:
	/** 반동을 계산하는 머신인지. 자기 폰을 조종하는 쪽에서만 참이다 */
	bool IsRecoilOwner() const;

protected:
	/** 발당 컨트롤 로테이션을 올리는 각도(도) */
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float PitchKick = 1.0f;

	/** 누적된 반동을 되돌리는 속도(도/초) */
	UPROPERTY(EditAnywhere, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoverySpeed = 2.0f;

	float AccumulatedPitch = 0.f;
	float LastKnownPitch = 0.f;

public:
	/** 한 발분 반동을 넣는다 */
	void ApplyRecoilShot();
};
