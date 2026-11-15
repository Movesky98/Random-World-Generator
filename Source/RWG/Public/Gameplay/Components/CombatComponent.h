// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Components/InputComponentBase.h"
#include "Gameplay/Enums/WeaponTypes.h"
#include "CombatComponent.generated.h"

class AWeaponBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponChanged, AWeaponBase* /* Weapon */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedDelegate, int32 /* CurrentAmmo */, int32 /* MaxAmmo */);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UCombatComponent : public UInputComponentBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

/*********************************************************************
*                                복제
*********************************************************************/
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
private:
	UPROPERTY()
	TObjectPtr<class UInventoryComponent> InventoryComponent;

	UPROPERTY()
	TObjectPtr<class URecoilComponent> RecoilComponent;

/*********************************************************************
*                             입력 처리
*********************************************************************/
protected:
	TSubclassOf<UInputConfigBase> GetConfigClass() override;

	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

/*********************************************************************
*                                상태
*********************************************************************/
private:
	UPROPERTY(VisibleInstanceOnly, Replicated, Category = "Weapon")
	EWeaponActionState WeaponActionState;

/*********************************************************************
*                             무기 교체
*********************************************************************/
private:
	UFUNCTION(Server, Reliable)
	void Server_RequestEquipWeapon(AWeaponBase* NewWeapon);

	UFUNCTION(Server, Reliable)
	void Server_RequestUnequipWeapon();

	UFUNCTION()
	void OnRep_CurrentWeapon(AWeaponBase* OldWeapon);

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentWeapon, Category = "Weapon")
	TObjectPtr<AWeaponBase> CurrentWeapon;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TObjectPtr<AWeaponBase> PendingWeapon;

protected:
	void SelectWeaponSlot(int32 SlotIndex);

	void EquipWeapon(AWeaponBase* NewWeapon);

	void OnEquipEnded();

	void UnequipWeapon();

	void OnUnequipEnded();

public:
	FOnCurrentWeaponChanged OnCurrentWeaponChanged;

	void SetCurrentWeapon(AWeaponBase* NewWeapon);

	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

/*********************************************************************
*                            재장전·사격
*********************************************************************/
private:
	UFUNCTION(Server, Reliable)
	void Server_RequestReload();

	UFUNCTION(Server, Reliable)
	void Server_RequestStartAttack();

	UFUNCTION(Server, Reliable)
	void Server_RequestStopAttack();

	UFUNCTION(Server, Reliable)
	void Server_RequestSwitchFireMode();

	/** 한 발씩 재장전 중에 사격 입력이 들어와 다음 단계를 끝 몽타주로 보내야 하는지 여부 (서버에서만 쓴다) */
	bool bReloadStopRequested = false;

protected:
	void Reload(EWeaponActionState ActionState);

	void OnReloadEnded();

	bool IsOneRoundReloadStep(EWeaponActionState ActionState) const;

	void AdvanceOneRoundReload();

	void OnAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);

public:
	FOnAmmoChangedDelegate OnAmmoChangedDelegate;

	void LoadOneRound();

/*********************************************************************
*                             애니메이션
*********************************************************************/
private:
	void PlayMontage(UAnimMontage* Montage, FOnMontageEnded EndDelegate = FOnMontageEnded(), FOnMontageBlendingOutStarted BlendingOutDelegate = FOnMontageBlendingOutStarted());

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(AWeaponBase* Weapon, EWeaponActionState ActionState);

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, EWeaponActionState EndedAction);

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted, EWeaponActionState BlendingOutAction);

protected:
	void SetAnimLayer(TSubclassOf<UAnimInstance> AnimLayerClass);

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimLayerClass;	// ABP_Unarmed_Layers

public:
	void ApplyCurrentWeaponAnimLayer();

/*********************************************************************
*                             위젯 연결
*********************************************************************/
protected:
	virtual TArray<TSubclassOf<UUserWidgetBase>> GetDefaultWidgetClasses() const override;

	virtual void BindComponent(UUserWidgetBase* Widget) override;

	virtual void UnbindComponent(UUserWidgetBase* Widget) override;
};
