// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Animation/Notifies/AnimNotify_ApplyWeaponAnimLayer.h"
#include "GamePlay/Components/CombatComponent.h"

void UAnimNotify_ApplyWeaponAnimLayer::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UCombatComponent* CombatComp = OwnerActor->FindComponentByClass<UCombatComponent>())
		{
			CombatComp->ApplyCurrentWeaponAnimLayer();
		}
	}
}