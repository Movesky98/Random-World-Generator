// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Characters/Zombie/Zombie.h"
#include "Components/CapsuleComponent.h"

AZombie::AZombie()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->SetCollisionProfileName(FName("Zombie"));
}

FGenericTeamId AZombie::GetGenericTeamId() const
{
	return TeamId;
}

ETeamAttitude::Type AZombie::GetTeamAttitudeTowards(const AActor& Other) const
{
    const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
    if (OtherTeamAgent && OtherTeamAgent->GetGenericTeamId() == FGenericTeamId(1))
    {
        return ETeamAttitude::Friendly;
    }
    return ETeamAttitude::Hostile;
}
