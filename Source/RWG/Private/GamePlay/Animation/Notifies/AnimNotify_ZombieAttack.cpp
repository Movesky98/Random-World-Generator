// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Animation/Notifies/AnimNotify_ZombieAttack.h"
#include "GamePlay/Characters/Zombie/Zombie.h"

void UAnimNotify_ZombieAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);

    AZombie* Zombie = Cast<AZombie>(MeshComp->GetOwner());
    if (!Zombie) return;

    Zombie->PerformAttack();
}