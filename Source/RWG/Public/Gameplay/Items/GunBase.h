// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/WeaponBase.h"
#include "GunBase.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedDelegate, int32 /* CurrentAmmo */, int32 /* MaxAmmo */);

UENUM()
enum class EReloadCondition : uint8
{
	Ready,
	MagazineFull,
	NoSpareAmmo,
	InvalidData,
	MontageMissing,
};

/**
 * 
 */
UCLASS()
class RWG_API AGunBase : public AWeaponBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AGunBase();

protected:
	virtual void BeginPlay() override;

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USkeletalMeshComponent> SkeletalMeshComponent;

public:
	class USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }

/*********************************************************************
*                                장착
*********************************************************************/
protected:
	virtual void Equip(ACharacter* NewOwner) override;

	virtual void Unequip() override;

/*********************************************************************
*                            몽타주 재생
*********************************************************************/
protected:
	virtual UAnimMontage* GetCharacterMontage(EWeaponActionState ActionState) const override;

	virtual void PlayActionMontage(EWeaponActionState ActionState) override;

/*********************************************************************
*                                사격
*********************************************************************/
private:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayDryFireSound();

protected:
	FTimerHandle FireTimerHandle;

	bool bIsFiring = false;

	virtual void StartAttack() override;

	virtual void StopAttack() override;

	bool CanStartAttack() const;

	void Fire();

	bool SpawnBulletProjectile();

/*********************************************************************
*                                탄약
*********************************************************************/
private:
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo = 0;

	UFUNCTION()
	void OnRep_CurrentAmmo();

public:
	FOnAmmoChangedDelegate OnAmmoChangedDelegate;

	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	bool HasAmmo() const
	{
		return CurrentAmmo > 0;
	}

	void SetCurrentAmmo(int32 NewAmmo);

	int32 GetLoadableAmmo(const int32 AvailableAmmo) const;

	int32 GetMagazineSize() const;

	EReloadCondition CheckReloadCondition(int32 AvailableAmmo, EWeaponActionState& OutActionState) const;

	class UItemData* GetAmmoType() const;

/*********************************************************************
*                              왼손 IK
*********************************************************************/
public:
	FTransform GetHandGuardTransform() const;
};
