// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Animation/Notifies/AnimNotify_LoadOneRound.h"
#include "Gameplay/Components/CombatComponent.h"

void UAnimNotify_LoadOneRound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UCombatComponent* CombatComp = OwnerActor->FindComponentByClass<UCombatComponent>())
		{
			// 탄 수 변경은 서버에서만 일어난다 (UCombatComponent::LoadOneRound가 검사)
			CombatComp->LoadOneRound();
		}
	}
}
