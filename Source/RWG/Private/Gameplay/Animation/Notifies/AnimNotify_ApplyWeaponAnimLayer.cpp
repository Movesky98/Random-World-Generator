// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Animation/Notifies/AnimNotify_ApplyWeaponAnimLayer.h"
#include "Gameplay/Components/CombatComponent.h"

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