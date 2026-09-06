// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Item.h"
#include "Gameplay/Enums/WeaponTypes.h"
#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class RWG_API AWeaponBase : public AItem
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AWeaponBase();

protected:
	virtual void BeginPlay() override;

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                              상호작용
*********************************************************************/
protected:
	virtual void Interact(AActor* Interactor) override;

/*********************************************************************
*                                장착
*********************************************************************/
private:
	UPROPERTY(Replicated)
	TObjectPtr<ACharacter> OwnerCharacter;

public:
	virtual void AttachToHolster(ACharacter* NewOwner);

	virtual void Equip(ACharacter* NewOwner);

	virtual void Unequip();

/*********************************************************************
*                            몽타주 재생
*********************************************************************/
public:
	// 이 액션에서 캐릭터 메시가 재생할 몽타주. 무기 종류마다 자기 데이터에서 고른다
	virtual class UAnimMontage* GetCharacterMontage(EWeaponActionState ActionState) const;

	// 이 액션에서 무기 메시가 재생할 몽타주를 직접 재생한다. AWeaponBase는 메시 컴포넌트가 없어 아무것도 하지 않는다
	virtual void PlayActionMontage(EWeaponActionState ActionState);

/*********************************************************************
*                                공격
*********************************************************************/
public:
	virtual void StartAttack();

	virtual void StopAttack();
};
