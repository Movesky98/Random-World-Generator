// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GenericTeamAgentInterface.h"
#include "ZombieAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class RWG_API AZombieAIController : public AAIController
{
	GENERATED_BODY()
	
public:
    AZombieAIController();

    void StopBehaviorTree();

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    FGenericTeamId GetGenericTeamId() const override;

    ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GenericTeam")
    uint8 TeamId;

private:
    UPROPERTY()
    TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

    UPROPERTY()
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

public:
    static const FName TargetActorKey;
    static const FName SlotLocationKey;
    static const FName bIsInCloseRangeKey;
    static const FName ChaseStateKey;
};
