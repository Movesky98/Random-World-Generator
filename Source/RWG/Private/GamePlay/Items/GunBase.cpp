// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Items/GunBase.h"
#include "GamePlay/Items/GunData.h"
#include "CommonLogCategories.h"
#include "Net/UnrealNetwork.h"

AGunBase::AGunBase()
{

}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();

}

void AGunBase::Equip(ACharacter* NewOwner)
{
	Super::Equip(NewOwner);

	// TODO : 厘馒 根鸥林 犁积 (WeaponData->EquipMontage)
	COMMON_LOG(LogGameplay, Log, TEXT("Gun equipped : %s"), *GetName());
}

void AGunBase::Unequip()
{
	Super::Unequip();

	// TODO : 秦力 根鸥林 犁积 (WeaponData->UnequipMontage)
	COMMON_LOG(LogGameplay, Log, TEXT("Gun unequipped : %s"), *GetName());
}

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGunBase, CurrentAmmo);
}

void AGunBase::SetCurrentAmmo(int32 NewAmmo)
{
	CurrentAmmo = NewAmmo;

	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, GetMagazineSize());
}

int32 AGunBase::GetMagazineSize() const
{
	UGunData* Data = GetItemData<UGunData>();

	return Data ? Data->MagazineSize : INDEX_NONE;
}

void AGunBase::OnRep_CurrentAmmo()
{
	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, GetMagazineSize());
}
