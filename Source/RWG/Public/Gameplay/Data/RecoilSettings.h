// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "RecoilSettings.generated.h"

class UCurveFloat;
class UCameraShakeBase;

/**
 * 총 한 자루의 사격 반동 튜닝값 묶음
 */
USTRUCT()
struct FRecoilSettings
{
	GENERATED_BODY()

	/** 연사 중 발마다 조준이 위로 튀는 각도 커브 (X: 몇 번째 발, Y: 각도(도)) */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCurveFloat> PitchKickCurve;

	/** 쏠 때마다 쏜 사람의 화면만 흔드는 카메라 셰이크 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	/** 연사 카운트를 리셋하는 마지막 발 뒤 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float ShotCountResetDelay = 0.3f;

	/** 누적된 반동을 되돌리는 속도(도/초) */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float RecoverySpeed = 2.0f;

	/** 마지막 발 뒤 복원을 시작하기까지 기다리는 시간(초) */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float RecoveryDelay = 0.15f;
};
