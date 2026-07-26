// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/GameFramework/ZombieAIController.h"
#include "Gameplay/Characters/Zombie/Zombie.h"
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

void AZombieAIController::StopBehaviorTree()
{
    if (!HasAuthority()) return;

    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!HasAuthority()) return;

    AZombie* Zombie = Cast<AZombie>(InPawn);
    if (!Zombie || !Zombie->BehaviorTree) return;

    UseBlackboard(Zombie->BehaviorTree->BlackboardAsset, BlackboardComponent);
    RunBehaviorTree(Zombie->BehaviorTree);

    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnPerceptionUpdated);
}

void AZombieAIController::OnUnPossess()
{
    Super::OnUnPossess();

    if (!HasAuthority()) return;

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

FGenericTeamId AZombieAIController::GetGenericTeamId() const
{
    return TeamId;
}

ETeamAttitude::Type AZombieAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    const APawn* OtherPawn = Cast<APawn>(&Other);
    if (OtherPawn)
    {
        const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
        if (OtherTeamAgent && OtherTeamAgent->GetGenericTeamId() == FGenericTeamId(TeamId))
        {
            return ETeamAttitude::Friendly;
        }
    }
    return ETeamAttitude::Hostile;
}
