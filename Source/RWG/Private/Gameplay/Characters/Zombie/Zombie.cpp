// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Zombie/Zombie.h"
#include "Gameplay/Components/HealthComponent.h"
#include "Gameplay/Characters/Zombie/Animation/ZombieAnimInstance.h"
#include "Gameplay/GameFramework/ZombieAIController.h"
#include "CommonLogCategories.h"

#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

AZombie::AZombie()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->SetCollisionProfileName(FName("Zombie"));
}

void AZombie::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        HealthComponent->OnDeathByDamage.AddUObject(this, &AZombie::OnDeath);
    }
}

void AZombie::RequestAttack()
{
    if (!HasAuthority()) return;

    UZombieAnimInstance* ZombieAnim = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
    if (!ZombieAnim || ZombieAnim->AttackMontages.IsEmpty()) return;

    int32 Index = FMath::RandRange(0, ZombieAnim->AttackMontages.Num() - 1);
    Multicast_PlayAttackMontage(Index);

    FOnMontageEnded MontageEndedDelegate;
    MontageEndedDelegate.BindLambda([this](UAnimMontage*, bool bInterrupted)
    {
        OnAttackFinished.ExecuteIfBound();
    });

    ZombieAnim->Montage_SetEndDelegate(MontageEndedDelegate, ZombieAnim->AttackMontages[Index]);
}

void AZombie::PerformAttack()
{
    if (!HasAuthority()) return;

    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * AttackDistance;

    FHitResult HitResult;
    FCollisionShape Box = FCollisionShape::MakeBox(FVector(20.f, 20.f, 100.f));
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ECC_GameTraceChannel3,  // PlayerHitable
        Box,
        Params
    );

    if (bHit && HitResult.GetActor())
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformAttack Hit: %s"), *HitResult.GetActor()->GetName());

        APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
        if (HitPawn)
        {
            IGenericTeamAgentInterface* HitTeamAgent = Cast<IGenericTeamAgentInterface>(HitPawn->GetController());
            IGenericTeamAgentInterface* SelfTeamAgent = Cast<IGenericTeamAgentInterface>(GetController());
            if (HitTeamAgent && HitTeamAgent->GetGenericTeamId() == SelfTeamAgent->GetGenericTeamId())
            {
                return;
            }
        }

        UGameplayStatics::ApplyDamage(
            HitResult.GetActor(),
            AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
    }

#if ENABLE_DRAW_DEBUG
    DrawDebugBox(GetWorld(), (Start + End) * 0.5f, Box.GetExtent(), FColor::Red, false, 1.f);
#endif
}

void AZombie::Multicast_PlayAttackMontage_Implementation(int32 MontageIndex)
{
    UZombieAnimInstance* ZombieAnim = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
    if (!ZombieAnim) return;

    ZombieAnim->PlayAttackMontage(MontageIndex);
}


void AZombie::Multicast_PlayDeathMontage_Implementation()
{
    UZombieAnimInstance* ZombieAnim = Cast<UZombieAnimInstance>(GetMesh()->GetAnimInstance());
    if (!ZombieAnim) return;

    EnableRagdoll();

    /* 문제 발생
    * 좀비의 Death Animation 재생 시 몸이 땅에 박혀서 Ragdoll로 변경하면 날아가는 문제 있음.
    * 제대로 된 Animation을 구하기 전까지는 바로 Ragdoll을 활성화하는 방식으로 갈 것.
    */
}

void AZombie::EnableRagdoll()
{
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);
}

void AZombie::OnDeath(const FDamageInfo& DamageInfo)
{
    if (AZombieAIController* AIController = Cast<AZombieAIController>(GetController()))
    {
         AIController->StopBehaviorTree();
    }

    Multicast_PlayDeathMontage();
    OnZombieDeath.Broadcast(this);
}

void AZombie::InitializeZombie(ACharacterBase* InAssignedCharacter)
{
    if (!InAssignedCharacter)
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("InAssignedCharacter is nullptr"));
        return;
    }

    AssignedCharacter = InAssignedCharacter;
}

ACharacterBase* AZombie::GetAssignedCharacter() const
{
    return AssignedCharacter;
}
