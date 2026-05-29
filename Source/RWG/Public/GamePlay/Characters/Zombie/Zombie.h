// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "Zombie.generated.h"

class UBehaviorTree;

/**
 * 
 */
UCLASS()
class RWG_API AZombie : public ACharacterBase, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
    AZombie();

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float CloseChaseDistance = 400.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float AttackDistance = 150.0f;

    FGenericTeamId GetGenericTeamId() const override;

    ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GenericTeam")
    uint8 TeamId;
};
