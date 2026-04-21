// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/InputHandlerComponent.h"
#include "GamePlay/DataAssets/BaseInputConfig.h"
#include "GamePlay/Components/LocomotionComponent.h"
#include "GamePlay/Interfaces/InputBindable.h"
#include "GamePlay/Components/BaseInputComponent.h"
#include "CommonLogCategories.h"

#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

// Sets default values for this component's properties
UInputHandlerComponent::UInputHandlerComponent()
{
	bWantsInitializeComponent = true;
}

// Called when the game starts
void UInputHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UInputHandlerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerController = Cast<APlayerController>(GetOwner());
	checkf(OwnerController, TEXT("InputHandlerComponent must be attached to PlayerController"));
}

void UInputHandlerComponent::RegisterBindableComponents(TArray<TScriptInterface<IInputBindable>>& Components, UEnhancedInputComponent* EnhancedInput)
{
	RegisteredComponents = Components;

	for (auto& Comp : Components)
	{
		if (Comp->IsConfigLoaded())
		{
			RequestActivateIMC(Comp.GetInterface());
			Comp->BindInputActions(EnhancedInput);
			
		}
		else
		{
			Comp->BindOnConfigLoaded([this, Comp, EnhancedInput]()
				{
					Comp->BindInputActions(EnhancedInput);
					RequestActivateIMC(Comp.GetInterface());
				});
		}
	}
}

void UInputHandlerComponent::RequestActivateIMC(IInputBindable* Requester, bool bExclusive)
{
	if (!OwnerController) 
	{
		COMMON_LOG(LogGameplay, Error, TEXT("OwnerController is not found."));
		return;
	}

	ULocalPlayer* LocalPlayer = OwnerController->GetLocalPlayer();
	if (!LocalPlayer) 
	{
		COMMON_LOG(LogGameplay, Error, TEXT("LocalPlayer is not found."));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)	
	{
		COMMON_LOG(LogGameplay, Error, TEXT("EnhancedInputLocalPlayerSubsystem is not found."));
		return;
	}

	if (bExclusive)
	{
		for (auto& Comp : RegisteredComponents)
		{
			if (Comp.GetInterface() != Requester && Comp->GetIMCPriority() < Requester->GetIMCPriority())
			{
				Subsystem->RemoveMappingContext(Comp->GetMappingContext());
			}
		}
	}

	if (Requester->GetMappingContext())
	{
		Subsystem->AddMappingContext(Requester->GetMappingContext(), Requester->GetIMCPriority());
	}
	else
		COMMON_LOG(LogGameplay, Error, TEXT("%s InputMappingContext is not found."), *Requester->GetInputConfig()->GetName());
}

void UInputHandlerComponent::RequestDeactiveIMC(IInputBindable* Requester)
{
	if (ULocalPlayer* LocalPlayer = OwnerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->RemoveMappingContext(Requester->GetMappingContext());
		}
	}
}