// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "GamePlay/GameFramework/ExpeditionGameMode.h"
#include "GamePlay/GameFramework/ExpeditionGameState.h"
#include "GamePlay/Components/UIManagerComponent.h"
#include "GamePlay/Components/InputHandlerComponent.h"
#include "GamePlay/Interfaces/InputBindable.h"
#include "GamePlay/Interfaces/WidgetBindable.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/UI/InventoryWidget.h"

#include "CommonLogCategories.h"

#include "EnhancedInputComponent.h"

AExpeditionPlayerController::AExpeditionPlayerController()
{
	InputHandlerComponent = CreateDefaultSubobject<UInputHandlerComponent>(TEXT("InputHandlerComponent"));

	UIManagerComponent = CreateDefaultSubobject<UUIManagerComponent>(TEXT("UIManagerComponent"));
}

void AExpeditionPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

void AExpeditionPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

}

void AExpeditionPlayerController::OnUnPossess()
{
	APawn* UnPossessPawn = GetPawn();
	if (UnPossessPawn)
	{
		if (UInventoryComponent* InventoryComp = UnPossessPawn->FindComponentByClass<UInventoryComponent>())
		{
			InventoryComp->OnInventoryToggled.RemoveAll(this);
		}
	}

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

void AExpeditionPlayerController::RequestReturnToLobby()
{
	Server_RequestReturnToLobby();
}

void AExpeditionPlayerController::HandleGameOver()
{
	// TODO: UIManagerComponent에게 알림

	if (IsLocalController() && UIManagerComponent)
	{
		UIManagerComponent->ShowGameOverWidget();
	}
}

void AExpeditionPlayerController::Server_RequestReturnToLobby_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	COMMON_LOG(LogGameplay, Log, TEXT("PC reqeust return to lobby."));
}
