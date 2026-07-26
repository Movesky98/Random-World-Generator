// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/AI/BTService_UpdateChaseMode.h"
#include "Gameplay/GameFramework/ZombieAIController.h"
#include "Gameplay/Characters/Zombie/Zombie.h"
#include "Gameplay/AI/ZombieChaseState.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateChaseMode::UBTService_UpdateChaseMode()
{
	NodeName = TEXT("Update Chase Mode");
	Interval = 0.2f;
}

void UBTService_UpdateChaseMode::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AZombieAIController* AIController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
    if (!AIController) return;

    AZombie* Zombie = Cast<AZombie>(AIController->GetPawn());
    if (!Zombie) return;

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return;

    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(AZombieAIController::TargetActorKey));
    if (!Target)
    {
        Blackboard->SetValueAsEnum(AZombieAIController::ChaseStateKey, static_cast<uint8>(EZombieChaseState::Idle));
        Blackboard->ClearValue(AZombieAIController::SlotLocationKey);
        return;
    }

    FVector ZombieLocation = Zombie->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();

    float DistanceToTarget = FVector::Dist2D(ZombieLocation, TargetLocation);
    
    EZombieChaseState NewChaseState = EZombieChaseState::Idle;

    if (DistanceToTarget <= Zombie->AttackDistance)
    {
        NewChaseState = EZombieChaseState::Attack;
    }
    else if (DistanceToTarget <= Zombie->CloseChaseDistance)
    {
        NewChaseState = EZombieChaseState::CloseChase;
    }
    else
    {
        NewChaseState = EZombieChaseState::SlotChase;

        FVector SlotLocation = CalculateSlotLocation(Target, Zombie, Zombie->CloseChaseDistance);
        
        Blackboard->SetValueAsVector(AZombieAIController::SlotLocationKey, SlotLocation);
    }

    Blackboard->SetValueAsEnum(AZombieAIController::ChaseStateKey, static_cast<uint8>(NewChaseState));
}

FVector UBTService_UpdateChaseMode::CalculateSlotLocation(AActor* Target, AActor* Owner, float MaxSlotDistanceFromTarget) const
{
    if (!Owner || !Target)
    {
        return FVector::ZeroVector;
    }

    FVector Direction = Owner->GetActorLocation() - Target->GetActorLocation();
    Direction.Z = 0.f;

    if (Direction.IsNearlyZero())
    {
        Direction = -Target->GetActorForwardVector();
        Direction.Z = 0.f;
    }

    Direction.Normalize();

    float SlotDistanceFromTarget = FMath::RandRange(MaxSlotDistanceFromTarget * 0.2f, MaxSlotDistanceFromTarget);
    float SlotAngleOffset = FMath::RandRange(-MaxSlotAngleOffset, MaxSlotAngleOffset);

    FVector RotationDirection = Direction.RotateAngleAxis(SlotAngleOffset, FVector::UpVector);
    FVector SlotLocation = Target->GetActorLocation() + RotationDirection * SlotDistanceFromTarget;

    return SlotLocation;
}
