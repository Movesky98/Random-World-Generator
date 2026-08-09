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
};

/**
 * 
 */
UCLASS()
class RWG_API AGunBase : public AWeaponBase
{
	GENERATED_BODY()
public:
	AGunBase();

protected:
	virtual void BeginPlay() override;

	virtual void Equip(ACharacter* NewOwner) override;

	virtual void Unequip() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void StartAttack() override;

	virtual void StopAttack() override;

	void Fire();

	bool CanStartAttack() const;

	FTimerHandle FireTimerHandle;

	bool bIsFiring = false;

	bool SpawnBulletProjectile();

	/* Ammo */
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

	EReloadCondition CheckReloadCondition(int32 AvailableAmmo, class UAnimMontage*& OutReloadMontage) const;

	class UItemData* GetAmmoType() const;

	FTransform GetHandGuardTransform() const;

private:
	UFUNCTION()
	void OnRep_CurrentAmmo();

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo = 0;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayDryFireSound();
};
