// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/UI/GameplayWidget.h"
#include "PlayerHUD.generated.h"

class AWeaponBase;
class UTextBlock;

/**
 * 
 */
UCLASS()
class RWG_API UPlayerHUD : public UGameplayWidget
{
	GENERATED_BODY()
public:
	UPlayerHUD();

protected:
	virtual void SetUp() override;

	/* Display Functions */
public:
	void SetAmmo(int32 CurrentAmmo, int32 MaxAmmo);

	void SetWeapon(AWeaponBase* Weapon);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoTextBlock;
};
