// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RecoilComponent.generated.h"

class ACharacter;
class AWeaponBase;
class AGunBase;
class UCurveFloat;

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
*                             발사 신호
*********************************************************************/
private:
	/** 발사 신호를 구독 중인 총 */
	UPROPERTY()
	TObjectPtr<AGunBase> Gun;

	void OnGunFired();

public:
	/** 발사 신호를 받을 무기를 바꾼다. 이전 총의 구독은 해제하고, 총일 때만 새로 구독한다 */
	void SetWeapon(AWeaponBase* NewWeapon);

/*********************************************************************
*                             반동 적용
*********************************************************************/
private:
	/** 반동을 계산하는 머신인지. 자기 폰을 조종하는 쪽에서만 참이다 */
	bool IsRecoilOwner() const;

protected:
	/** 연사 중 발마다 조준이 위로 튀는 각도 커브 (X: 몇 번째 발, Y: 각도(도)) */
	UPROPERTY(EditAnywhere, Category = "Recoil")
	TObjectPtr<UCurveFloat> PitchKickCurve;

	/** 연사 카운트를 리셋하는 마지막 발 뒤 대기 시간(초) */
	UPROPERTY(EditAnywhere, Category = "Recoil", meta = (ClampMin = "0.0"))
	float ShotCountResetDelay = 0.3f;

	/** 누적된 반동을 되돌리는 속도(도/초) */
	UPROPERTY(EditAnywhere, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoverySpeed = 2.0f;

	/** 마지막 발 뒤 복원을 시작하기까지 기다리는 시간(초) */
	UPROPERTY(EditAnywhere, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoveryDelay = 0.15f;

	float AccumulatedPitch = 0.f;
	float LastKnownPitch = 0.f;

	/** 이번 연사의 발 번호 */
	int32 ShotCount = 0;

	/** 마지막 발의 월드 시간 */
	double LastShotTime = 0.0;

public:
	/** 한 발분 반동을 넣는다 */
	void ApplyRecoilShot();
};
