// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Items/WeaponBase.h"
#include "GamePlay/Items/WeaponData.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AWeaponBase::AWeaponBase()
{

}

void AWeaponBase::BeginPlay()
{
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
