// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/AI/BTTask_ZombieAttack.h"
#include "GamePlay/GameFramework/ZombieAIController.h"
#include "GamePlay/Characters/Zombie/Zombie.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_ZombieAttack::UBTTask_ZombieAttack()
{
	NodeName = TEXT("Zombie Attack");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_ZombieAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

    AZombieAIController* AIController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
    if (!AIController) return EBTNodeResult::Failed;

    AZombie* Zombie = Cast<AZombie>(AIController->GetPawn());
    if (!Zombie) return EBTNodeResult::Failed;

    // 몽타주 종료 델리게이트 바인딩
    Zombie->OnAttackFinished.BindLambda([this, &OwnerComp]()
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    });

    // 공격 요청
    Zombie->RequestAttack();

    return EBTNodeResult::InProgress;
}
