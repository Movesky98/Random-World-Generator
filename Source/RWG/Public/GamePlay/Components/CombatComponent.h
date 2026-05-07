// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Components/BaseInputComponent.h"
#include "CombatComponent.generated.h"

class AWeaponBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponChanged, AWeaponBase* /* Weapon */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32 /* CurrentAmmo */, int32 /* MaxAmmo */);

UENUM(BlueprintType)
enum class EWeaponTransitionState : uint8
{
	None,
	Equipping,
	Unequipping
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UCombatComponent : public UBaseInputComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

	/* Component Lifecycle */
protected:
	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Input */
protected:
	TSubclassOf<UBaseInputConfig> GetConfigClass() override;

	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

	void SelectWeaponSlot(int32 SlotIndex);

	/* Weapon Flow */
public:
	FOnCurrentWeaponChanged OnCurrentWeaponChanged;

	FOnAmmoChanged OnAmmoChanged;

	void NotifyCurrentWeaponState();

protected:
	void EquipWeapon(AWeaponBase* NewWeapon);

	void OnEquipEnded();

	void UnequipWeapon();

	void OnUnequipEnded();

	void Reload();

	void onReloadEnded();

private:
	void RequestEquipWeapon(AWeaponBase* NewWeapon);
	
	UPROPERTY(VisibleInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<AWeaponBase> CurrentWeapon;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TObjectPtr<AWeaponBase> PendingWeapon;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	EWeaponTransitionState WeaponTransitionState;
	
	UPROPERTY()
	TObjectPtr<class UInventoryComponent> InventoryComponent;

	/* Animation */
public:
	void ApplyCurrentWeaponAnimLayer();

protected:
	void SetAnimLayer(TSubclassOf<UAnimInstance> AnimLayerClass);

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimLayerClass;	// ABP_Unarmed_Layers

private:
	void PlayMontage(UAnimMontage* Montage, FOnMontageEnded EndDelegate);

	void OnUnequipMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
