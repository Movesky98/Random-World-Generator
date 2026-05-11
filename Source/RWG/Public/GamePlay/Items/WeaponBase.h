// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Items/Item.h"
#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API AWeaponBase : public AItem
{
	GENERATED_BODY()
public:
	AWeaponBase();

	virtual void AttachToHolster(ACharacter* NewOwner);

	virtual void Equip(ACharacter* NewOwner);

	virtual void Unequip();


protected:
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Interact(AActor* Interactor) override;
	
private:
	UPROPERTY(Replicated)
	TObjectPtr<ACharacter> OwnerCharacter;

public:
	virtual void StartAttack();

	virtual void StopAttack();
};
