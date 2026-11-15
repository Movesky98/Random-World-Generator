// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Convict/Animation/GunAnimInstance.h"
#include "Gameplay/Components/CombatComponent.h"
#include "Gameplay/Items/GunBase.h"
#include "Gameplay/DataAssets/WeaponData.h"
#include "CommonLogCategories.h"

#include "GameFramework/Character.h"

void UGunAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	if (!OwnerCharacter) return;

	UCombatComponent* CombatComp = OwnerCharacter->FindComponentByClass<UCombatComponent>();
	if (!CombatComp)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("%s have not CombatComponent."), *GetNameSafe(OwnerCharacter));
		return;
	}

	CombatComponent = CombatComp;
}

void UGunAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	bIsHandGuardValid = false;
	IdlePose = nullptr;

	if (!CombatComponent) return;

	AGunBase* Gun = Cast<AGunBase>(CombatComponent->GetCurrentWeapon());
	if (!Gun) return;

	HandGuardTransform = Gun->GetHandGuardTransform();
	bIsHandGuardValid = true;

	if (UWeaponData* WeaponData = Gun->GetItemData<UWeaponData>())
	{
		IdlePose = WeaponData->IdlePose;
	}
}
