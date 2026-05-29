// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
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

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

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
