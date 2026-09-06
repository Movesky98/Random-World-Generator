// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CharacterBase/CharacterBase.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Convict.generated.h"

class ULocomotionComponent;
class UCombatComponent;
class UInventoryComponent;
class UInteractionComponent;
class UHealthComponent;

class USpringArmComponent;
class UCameraComponent;
class USphereComponent;

/**
 * 
 */
UCLASS()
class RWG_API AConvict : public ACharacterBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AConvict();

protected:
	virtual void PostInitializeComponents() override;

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	ULocomotionComponent* LocomotionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UCombatComponent* CombatComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UInteractionComponent* InteractionComponent;

	UPROPERTY()
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;

	UPROPERTY(VisibleAnywhere, Category = "Detection")
	TObjectPtr<USphereComponent> ProximityDetector;

/*********************************************************************
*                           조준 방향 복제
*********************************************************************/
public:
	/** 시뮬레이티드 프록시에서 쓸 조준 Yaw(도, 월드 기준). 압축을 풀고 -180~180으로 감아서 돌려준다. */
	float GetRemoteViewYaw() const { return FRotator::NormalizeAxis(FRotator::DecompressAxisFromShort(RemoteViewYaw16)); }

protected:
	/**
	 * 이 캐릭터가 바라보는 방향(월드 기준 Yaw)을 압축해서 담는다. 엔진의 RemoteViewPitch16과 짝이다.
	 * 주인은 자기 컨트롤러로 직접 구하므로 COND_SkipOwner로 제외한다.
	 */
	UPROPERTY(Replicated)
	uint16 RemoteViewYaw16 = 0;

/*********************************************************************
*                         플레이어 접근 감지
*********************************************************************/
protected:
	UFUNCTION()
	void OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

/*********************************************************************
*                                피해
*********************************************************************/
protected:
	virtual void ProcessDamage(const FDamageInfo& DamageInfo) override;
};
