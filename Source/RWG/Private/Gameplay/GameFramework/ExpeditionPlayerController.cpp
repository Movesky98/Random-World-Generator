// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/GameFramework/ExpeditionPlayerController.h"
#include "Gameplay/GameFramework/ExpeditionGameMode.h"
#include "Gameplay/GameFramework/ExpeditionGameState.h"
#include "Gameplay/Components/UIManagerComponent.h"
#include "Gameplay/Components/InputHandlerComponent.h"
#include "Gameplay/Interfaces/InputBindable.h"
#include "Gameplay/Interfaces/WidgetBindable.h"

#include "Common/GameFramework/GlobalUISubsystem.h"
#include "Common/CommonDelegates.h"
#include "CommonLogCategories.h"

#include "RandomWorldGeneration/GameFramework/WorldGenSubsystem.h"
#include "EnhancedInputComponent.h"

AExpeditionPlayerController::AExpeditionPlayerController()
{
	InputHandlerComponent = CreateDefaultSubobject<UInputHandlerComponent>(TEXT("InputHandlerComponent"));

	UIManagerComponent = CreateDefaultSubobject<UUIManagerComponent>(TEXT("UIManagerComponent"));
}

void AExpeditionPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocalPlayerController()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UWorldGenSubsystem* WorldGenSubsystem = World->GetSubsystem<UWorldGenSubsystem>();
	if (!WorldGenSubsystem) return;

	if (WorldGenSubsystem->IsWorldReady())
	{
		Server_ReportWorldGenerationCompleted();
	}
	else
		WorldGenSubsystem->OnWorldReady.AddUObject(this, &ThisClass::Server_ReportWorldGenerationCompleted);
	
	AExpeditionGameState* ExpeditionGS = World->GetGameState<AExpeditionGameState>();
	if (!ExpeditionGS) return;

	ExpeditionGS->OnGameplayStateChanged.AddUObject(this, &ThisClass::OnGameplayStateChanged);

}

void AExpeditionPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

}

void AExpeditionPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}

void AExpeditionPlayerController::AcknowledgePossession(APawn* aPawn)
{
	Super::AcknowledgePossession(aPawn);

	// IInputBindable 수집
	TArray<UActorComponent*> Components = aPawn->GetComponentsByInterface(UInputBindable::StaticClass());

	TArray<TScriptInterface<IInputBindable>> BindableComponents;
	for (UActorComponent* Component : Components)
	{
		TScriptInterface<IInputBindable> Bindable;
		Bindable.SetObject(Component);
		Bindable.SetInterface(Cast<IInputBindable>(Component));
		BindableComponents.Add(Bindable);
	}

	BindableComponents.Sort([](const TScriptInterface<IInputBindable>& A, const TScriptInterface<IInputBindable>& B)
		{
			return A->GetIMCPriority() < B->GetIMCPriority();
		});

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
		InputHandlerComponent->RegisterBindableComponents(BindableComponents, EnhancedInput);

	// IWidgetBindable 수집
	TArray<UActorComponent*> WidgetBindableComps = aPawn->GetComponentsByInterface(UWidgetBindable::StaticClass());

	TArray<TScriptInterface<IWidgetBindable>> WidgetBindables;
	for (UActorComponent* Component : WidgetBindableComps)
	{
		TScriptInterface<IWidgetBindable> Bindable;
		Bindable.SetObject(Component);
		Bindable.SetInterface(Cast<IWidgetBindable>(Component));
		WidgetBindables.Add(Bindable);
	}

	UIManagerComponent->InitializePawnWidgets(aPawn);
	UIManagerComponent->BindDelegatesFromGameplayWidgets(WidgetBindables);

	// GameOver 수신을 위해 델리게이트 구독
	if (AExpeditionGameState* GS = GetWorld()->GetGameState<AExpeditionGameState>())
	{
		GS->OnGameOver.AddUObject(this, &ThisClass::HandleGameOver);
	}
}

void AExpeditionPlayerController::Server_RequestReturnToLobby_Implementation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	COMMON_LOG(LogGameplay, Log, TEXT("PC reqeust return to lobby."));
}


void AExpeditionPlayerController::RequestReturnToLobby()
{
	Server_RequestReturnToLobby();
}

void AExpeditionPlayerController::Server_ReportWorldGenerationCompleted_Implementation()
{
	UWorld* World = GetWorld();
	if (World)
	{
		AExpeditionGameMode* GameMode = World->GetAuthGameMode<AExpeditionGameMode>();
		if (GameMode)
		{
			GameMode->ReportWorldGenerationCompleted(this);
		}
	}
}

void AExpeditionPlayerController::HandleGameOver()
{
	// TODO: UIManagerComponent에게 알림

	if (IsLocalPlayerController() && UIManagerComponent)
	{
		UIManagerComponent->ShowGameOverWidget();
	}
}

void AExpeditionPlayerController::OnGameplayStateChanged(EGameplayState GameplayState)
{
	if (GameplayState == EGameplayState::Preparing)
	{
		UGlobalUISubsystem* GlobalUISubsystem = GetGameInstance()->GetSubsystem<UGlobalUISubsystem>();
		AExpeditionGameState* ExpeditionGS = GetWorld()->GetGameState<AExpeditionGameState>();

		if (GlobalUISubsystem && ExpeditionGS)
		{
			FText TitleText = FText::FromString(TEXT("게임 시작까지"));
			FGetRemainingSecondsDelegate GetRemainingSecondsDelegate;
			GetRemainingSecondsDelegate.BindUObject(ExpeditionGS, &AExpeditionGameState::GetPrepareRemainingSeconds);
			
			GlobalUISubsystem->ShowCountdownWidget(TitleText, GetRemainingSecondsDelegate);
		}
	}

	if (GameplayState == EGameplayState::Playing)
	{
		if (UGlobalUISubsystem* GlobalUISubsystem = GetGameInstance()->GetSubsystem<UGlobalUISubsystem>())
		{
			GlobalUISubsystem->HideCountdownWidget();
		}
	}
}
