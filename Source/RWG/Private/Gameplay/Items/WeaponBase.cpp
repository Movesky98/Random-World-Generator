// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/WeaponBase.h"
#include "Gameplay/Items/WeaponData.h"
#include "Gameplay/Components/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AWeaponBase::AWeaponBase()
{

}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, OwnerCharacter);
}

void AWeaponBase::Interact(AActor* Interactor)
{
	if (!HasAuthority()) return;

	UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp) return;

	InventoryComp->TryAddWeapon(this);
}

void AWeaponBase::StartAttack()
{

}

void AWeaponBase::StopAttack()
{

}

void AWeaponBase::Equip(ACharacter* NewOwner)
{
	if (!HasAuthority()) return;

	if (!NewOwner) return;

	OwnerCharacter = NewOwner;

	UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
	if (!WeaponData) return;

	AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		WeaponData->EquipSocketName
	);
}

void AWeaponBase::Unequip()
{
	if (!HasAuthority()) return;

	UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
	if (!WeaponData || !OwnerCharacter) return;

	AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		WeaponData->HolsterSocketName
	);
}

UAnimMontage* AWeaponBase::GetCharacterMontage(EWeaponActionState ActionState) const
{
	UWeaponData* WeaponData = GetItemData<UWeaponData>();
	if (!WeaponData) return nullptr;

	switch (ActionState)
	{
	case EWeaponActionState::Equip:		return WeaponData->CharacterEquipMontage;
	case EWeaponActionState::Unequip:	return WeaponData->CharacterUnequipMontage;
	default:							return nullptr;
	}
}

void AWeaponBase::PlayActionMontage(EWeaponActionState ActionState)
{
	// AWeaponBase에는 메시 컴포넌트가 없다. 메시를 가진 파생 클래스가 구현한다
}

void AWeaponBase::AttachToHolster(ACharacter* NewOwner)
{
	if (!NewOwner) return;

	OwnerCharacter = NewOwner;

	UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
	if (!WeaponData) return;

	AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		WeaponData->HolsterSocketName
	);
}
