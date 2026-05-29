// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ZombieAIController.h"
#include "GamePlay/Characters/Zombie/Zombie.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

const FName AZombieAIController::TargetActorKey = TEXT("TargetActor");
const FName AZombieAIController::SlotLocationKey = TEXT("SlotLocation");
const FName AZombieAIController::bIsInCloseRangeKey = TEXT("bIsInCloseRange");
const FName AZombieAIController::ChaseStateKey = TEXT("ChaseState");

AZombieAIController::AZombieAIController()
{
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    UAIPerceptionComponent* AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*AIPerception);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 2000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    GetPerceptionComponent()->ConfigureSense(*SightConfig);
    GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AZombie* Zombie = Cast<AZombie>(InPawn);
    if (!Zombie || !Zombie->BehaviorTree) return;

    UseBlackboard(Zombie->BehaviorTree->BlackboardAsset, BlackboardComponent);
    RunBehaviorTree(Zombie->BehaviorTree);

    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnPerceptionUpdated);
}

void AZombieAIController::OnUnPossess()
{
    Super::OnUnPossess();
    BehaviorTreeComponent->StopTree();
}

void AZombieAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        GetBlackboardComponent()->SetValueAsObject(TargetActorKey, Actor);
    }
    else
    {
        GetBlackboardComponent()->SetValueAsObject(TargetActorKey, nullptr);
    }
}