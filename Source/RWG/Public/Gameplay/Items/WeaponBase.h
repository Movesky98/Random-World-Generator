// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Item.h"
#include "Gameplay/Enums/WeaponTypes.h"
#include "WeaponBase.generated.h"

/**
 * 무기가 캐릭터에 붙어 있는지와 붙은 소켓
 */
USTRUCT()
struct FWeaponAttachState
{
	GENERATED_BODY()

	/** 무기가 붙은 캐릭터 (없으면 바닥에 놓인 상태) */
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	/** 무기를 붙인 캐릭터 메시 소켓 이름 */
	UPROPERTY()
	FName SocketName;
};

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
	/** 무기가 붙은 캐릭터와 소켓 */
	UPROPERTY(ReplicatedUsing = OnRep_AttachState)
	FWeaponAttachState AttachState;

	UFUNCTION()
	void OnRep_AttachState();

	// 서버에서 붙음 상태를 바꾸고 적용한다. NewOwner가 없으면 바닥에 놓인 상태가 된다
	void SetAttachState(ACharacter* NewOwner, FName SocketName);

	// 붙음 상태에 맞춰 물리·충돌·부착·이동 복제를 정한다. 서버와 클라이언트가 같이 쓴다
	void ApplyAttachState();

public:
	virtual void AttachToHolster(ACharacter* NewOwner);

	virtual void Equip(ACharacter* NewOwner);

	// 캐릭터에서 떼어 지정한 위치에 놓고 물리 시뮬레이션으로 떨어뜨린다
	virtual void Drop(const FVector& DropLocation, const FRotator& DropRotation);

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

	// 공격 입력을 뗐을 때 호출된다. 기본 동작은 공격 중지
	virtual void ReleaseAttack();
};
