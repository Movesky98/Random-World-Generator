// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/GameFramework/ExpeditionPlayerController.h"
#include "GamePlay/Components/InputHandlerComponent.h"
#include "GamePlay/Interfaces/InputBindable.h"

#include "EnhancedInputComponent.h"

AExpeditionPlayerController::AExpeditionPlayerController()
{
	InputHandlerComponent = CreateDefaultSubobject<UInputHandlerComponent>(TEXT("InputHandlerComponent"));
}

void AExpeditionPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

void AExpeditionPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	TArray<UActorComponent*> Components = aPawn->GetComponentsByInterface(UInputBindable::StaticClass());
	
	TArray<TScriptInterface<IInputBindable>> BindableComponents;
	for (UActorComponent* Component : Components)
	{
		TScriptInterface<IInputBindable> Bindable;
		Bindable.SetObject(Component);
		Bindable.SetInterface(Cast<IInputBindable>(Component));
		BindableComponents.Add(Bindable);
	}

	// Priority Á¤·Ä
	BindableComponents.Sort([](const TScriptInterface<IInputBindable>& A, const TScriptInterface<IInputBindable>& B)
		{
			return A->GetIMCPriority() < B->GetIMCPriority();
		});

	if(UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
		InputHandlerComponent->RegisterBindableComponents(BindableComponents, EnhancedInput);
}
