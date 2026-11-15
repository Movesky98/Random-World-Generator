// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Chaos/ChaosEngineInterface.h"
#include "BulletImpactData.generated.h"

class UParticleSystem;
class UMaterialInterface;
class USoundBase;

/** 탄이 맞은 자리 하나에서 재생할 연출 묶음 */
USTRUCT(BlueprintType)
struct FBulletImpactEffect
{
	GENERATED_BODY()

	/** 맞은 자리에서 재생할 파티클 */
	UPROPERTY(EditDefaultsOnly, Category = "Impact")
	TObjectPtr<UParticleSystem> Particle = nullptr;

	/** 맞은 자리에 남길 탄흔 머티리얼 (비워 두면 탄흔을 남기지 않는다) */
	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	TObjectPtr<UMaterialInterface> DecalMaterial = nullptr;

	/** 탄흔 크기 (X: 투영 깊이, Y·Z: 표면 위 크기)(cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Decal")
	FVector DecalSize = FVector(4.0f, 6.0f, 6.0f);

	/** 탄흔이 사라지기 시작할 때까지의 시간(초) */
	UPROPERTY(EditDefaultsOnly, Category = "Decal", meta = (ClampMin = "0.0"))
	float DecalLifeSpan = 10.0f;

	/** 탄흔이 서서히 사라지는 시간(초) */
	UPROPERTY(EditDefaultsOnly, Category = "Decal", meta = (ClampMin = "0.0"))
	float DecalFadeDuration = 1.0f;

	/** 탄흔이 화면에서 이 비율보다 작아지면 흐려지는 기준 (0이면 거리와 관계없이 보인다) */
	UPROPERTY(EditDefaultsOnly, Category = "Decal", meta = (ClampMin = "0.0"))
	float DecalFadeScreenSize = 0.001f;

	/** 맞은 자리에서 재생할 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> Sound = nullptr;
};

/**
 * 탄이 맞은 대상에 따라 재생할 Bullet Impact·Bullet Decal 목록
 */
UCLASS()
class RWG_API UBulletImpactData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 표면 종류별 목록에 없는 표면에 맞았을 때의 연출 */
	UPROPERTY(EditDefaultsOnly, Category = "Impact")
	FBulletImpactEffect DefaultEffect;

	/** 캐릭터에 맞았을 때의 연출 */
	UPROPERTY(EditDefaultsOnly, Category = "Impact")
	FBulletImpactEffect CharacterEffect;

	/** 표면 종류(Physical Material의 Surface Type)별 연출 */
	UPROPERTY(EditDefaultsOnly, Category = "Impact")
	TMap<TEnumAsByte<EPhysicalSurface>, FBulletImpactEffect> SurfaceEffects;

	const FBulletImpactEffect& GetEffect(bool bHitCharacter, EPhysicalSurface SurfaceType) const;
};
