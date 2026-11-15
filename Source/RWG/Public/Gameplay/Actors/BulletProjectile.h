// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/NetSerialization.h"
#include "Chaos/ChaosEngineInterface.h"
#include "BulletProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UBulletImpactData;

/** 탄이 맞은 자리의 정보 */
USTRUCT()
struct FBulletImpactInfo
{
	GENERATED_BODY()

	/** 무언가에 맞았는지 여부 */
	UPROPERTY()
	bool bHasImpacted = false;

	/** 캐릭터에 맞았는지 여부 */
	UPROPERTY()
	bool bHitCharacter = false;

	/** 맞은 위치 */
	UPROPERTY()
	FVector_NetQuantize Location = FVector::ZeroVector;

	/** 맞은 표면의 바깥 방향 */
	UPROPERTY()
	FVector_NetQuantizeNormal Normal = FVector::ZeroVector;

	/** 맞은 표면의 종류 (Physical Material의 Surface Type) */
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType = SurfaceType_Default;
};

UCLASS()
class RWG_API ABulletProjectile : public AActor
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this actor's properties
	ABulletProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

/*********************************************************************
*                            발사와 소멸
*********************************************************************/
private:
	float Damage = 1.0f;

	FTimerHandle LifeTimerHandle;

	void DestroyProjectile();

public:
	void InitProjectile(float InDamage, float InInitialSpeed, float LifeTime, UBulletImpactData* InImpactData);

/*********************************************************************
*                             피격 처리
*********************************************************************/
protected:
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

/*********************************************************************
*                         Bullet Impact 연출
*********************************************************************/
private:
	/** 맞은 대상별 Bullet Impact·Bullet Decal 목록 */
	UPROPERTY(Replicated)
	TObjectPtr<UBulletImpactData> ImpactData;

	/** 맞은 자리의 정보 */
	UPROPERTY(ReplicatedUsing = OnRep_ImpactInfo)
	FBulletImpactInfo ImpactInfo;

	/** 맞은 뒤 숨긴 채 남아 있다가 사라질 때까지의 시간(초) */
	float ImpactLingerTime = 1.0f;

	UFUNCTION()
	void OnRep_ImpactInfo();

	void HandleImpact(AActor* HitActor, const FHitResult& Hit, bool bHasHitLocation);

	void StopAfterImpact();

	void PlayImpactEffects() const;
};
