// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/UI/GameplayWidget.h"
#include "PlayerHUD.generated.h"

class AWeaponBase;
class UTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS()
class RWG_API UPlayerHUD : public UGameplayWidget
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	UPlayerHUD();

protected:
	virtual void SetUp() override;

/*********************************************************************
*                                표시
*********************************************************************/
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

public:
	void SetAmmo(int32 CurrentAmmo, int32 MaxAmmo);

	void SetWeapon(AWeaponBase* Weapon);

	void SetHealth(float CurrentHealth, float MaxHealth);
};
