// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionGameMode.h"
#include "GamePlay/GameFramework/ExpeditionGameState.h"
#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "GamePlay/GameFramework/ExpeditionPlayerState.h"
#include "GamePlay/Characters/CharacterBase/CharacterBase.h"
#include "GamePlay/Components/SpawnDirectorComponent.h"
#include "GamePlay/Components/TimeManagementComponent.h"
#include "CommonLogCategories.h"

#include "RandomWorldGeneration/Actors/WorldGenerator.h"

#include "Kismet/GameplayStatics.h"

AExpeditionGameMode::AExpeditionGameMode()
{
	DefaultPawnClass = ACharacterBase::StaticClass();
	PlayerControllerClass = AExpeditionPlayerController::StaticClass();
	GameStateClass = AExpeditionGameState::StaticClass();
	PlayerStateClass = AExpeditionPlayerState::StaticClass();

	SpawnDirectorComponent = CreateDefaultSubobject<USpawnDirectorComponent>(TEXT("SpawnDirectorComponent"));
    TimeManagementComponent = CreateDefaultSubobject<UTimeManagementComponent>(TEXT("TimeManagementComponent"));
}

void AExpeditionGameMode::BeginPlay()
{
	Super::BeginPlay();

    TArray<AActor*> Generators;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldGenerator::StaticClass(), Generators);

    if (Generators.Num() > 0)
    {
        if (AWorldGenerator* Generator = Cast<AWorldGenerator>(Generators[0]))
        {
            Generator->OnWorldGenerationComplete.AddLambda([this]()
                {
                    SpawnDirectorComponent->InitializeSpawnData();
                });
        }
    }
}

void AExpeditionGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);    // 이 시점에 Pawn이 Spawn + Possess

    if (!NewPlayer->GetPawn())
    {
        COMMON_LOG(LogGameplay, Warning, TEXT("Can't find player pawn."));

        return;
    }

    SpawnDirectorComponent->RegisterPlayer(NewPlayer->GetPawn());
}

void AExpeditionGameMode::Logout(AController* Exiting)
{
    SpawnDirectorComponent->UnregisterPlayer(Exiting->GetPawn());
    
    Super::Logout(Exiting);
}
