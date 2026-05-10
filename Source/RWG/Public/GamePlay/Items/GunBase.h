// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Items/WeaponBase.h"
#include "GunBase.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedDelegate, int32 /* CurrentAmmo */, int32 /* MaxAmmo */);

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

	/* Ammo */
public:

	FOnAmmoChangedDelegate OnAmmoChangedDelegate;

	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	void SetCurrentAmmo(int32 NewAmmo);

	int32 GetMagazineSize() const;

private:
	UFUNCTION()
	void OnRep_CurrentAmmo();

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo = 0;
};
