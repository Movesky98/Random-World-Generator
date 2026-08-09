// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Components/BaseInputComponent.h"
#include "CombatComponent.generated.h"

class AWeaponBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponChanged, AWeaponBase* /* Weapon */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedDelegate, int32 /* CurrentAmmo */, int32 /* MaxAmmo */);

UENUM(BlueprintType)
enum class EWeaponActionState : uint8
{
	None,
	Equip,
	Unequip,
	Reload,
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

	FOnAmmoChangedDelegate OnAmmoChangedDelegate;

	void SetCurrentWeapon(AWeaponBase* NewWeapon);

	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

protected:
	void EquipWeapon(AWeaponBase* NewWeapon);

	void OnEquipEnded();

	void UnequipWeapon();

	void OnUnequipEnded();

	void Reload(class UAnimMontage* ReloadMontage);

	void OnReloadEnded();

	void OnAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);

private:
	UFUNCTION(Server, Reliable)
	void Server_RequestEquipWeapon(AWeaponBase* NewWeapon);

	UFUNCTION(Server, Reliable)
	void Server_RequestUnequipWeapon();

	UFUNCTION(Server, Reliable)
	void Server_RequestReload();

	UFUNCTION(Server, Reliable)
	void Server_RequestStartAttack();

	UFUNCTION(Server, Reliable)
	void Server_RequestStopAttack();

	UFUNCTION()
	void OnRep_CurrentWeapon(AWeaponBase* OldWeapon);
	
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentWeapon, Category = "Weapon")
	TObjectPtr<AWeaponBase> CurrentWeapon;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TObjectPtr<AWeaponBase> PendingWeapon;

	UPROPERTY(VisibleInstanceOnly, Replicated, Category = "Weapon")
	EWeaponActionState WeaponActionState;
	
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
	void PlayMontage(UAnimMontage* Montage, FOnMontageEnded EndDelegate = FOnMontageEnded());

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* Montage, EWeaponActionState ActionState);

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, EWeaponActionState EndedAction);

	// IWidgetBindable
protected:
	virtual TArray<TSubclassOf<UUserWidgetBase>> GetDefaultWidgetClasses() const override;

	virtual void BindComponent(UUserWidgetBase* Widget) override;

	virtual void UnbindComponent(UUserWidgetBase* Widget) override;
};
