// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RecoilComponent.generated.h"

class ACharacter;
class AWeaponBase;
class AGunBase;
struct FRecoilSettings;

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

	/** 구독 중인 총의 반동 튜닝값. 총이나 데이터가 없으면 nullptr */
	const FRecoilSettings* GetRecoilSettings() const;

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
