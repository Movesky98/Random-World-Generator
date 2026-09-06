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

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	AZombieAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

/*********************************************************************
*                           구성 컴포넌트
*********************************************************************/
private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY()
	UBlackboardComponent* BlackboardComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

/*********************************************************************
*                          비헤이비어 트리
*********************************************************************/
public:
	static const FName TargetActorKey;
	static const FName SlotLocationKey;
	static const FName bIsInCloseRangeKey;
	static const FName ChaseStateKey;

	void StopBehaviorTree();

/*********************************************************************
*                             시야 감지
*********************************************************************/
protected:
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

/*********************************************************************
*                                 팀
*********************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GenericTeam")
	uint8 TeamId;

	FGenericTeamId GetGenericTeamId() const override;

	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
};
