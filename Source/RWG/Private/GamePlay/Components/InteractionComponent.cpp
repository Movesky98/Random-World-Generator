// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/InteractionComponent.h"
#include "GamePlay/DataAssets/InteractionInputConfig.h"
#include "GamePlay/Interfaces/Interactable.h"
#include "CommonLogCategories.h"

#define TRACE_INTERACTION ECC_GameTraceChannel1

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{

}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UInteractionComponent::BindInputActions(UEnhancedInputComponent* InputComponent)
{
	UInteractionInputConfig* InteractionConfig = Cast<UInteractionInputConfig>(LoadedConfig);

	if (!InputComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InputComponent is null. SetupInputComponent may not have been called."));
		return;
	}

	if (!InteractionConfig)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InventoryConfig is null. Check Asset Manager or config assignment."));
		return;
	}

	InputComponent->BindAction(InteractionConfig->InteractAction, ETriggerEvent::Triggered, this, &ThisClass::PerformInteractionTrace);
}

TSubclassOf<UBaseInputConfig> UInteractionComponent::GetConfigClass()
{
	return UInteractionInputConfig::StaticClass();
}

void UInteractionComponent::PerformInteractionTrace()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController());
	if (!PC) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceEnd = CameraLocation + CameraRotation.Vector() * TraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		TraceEnd,
		TRACE_INTERACTION
	);

	if (bIsHit && HitResult.GetActor())
	{
		IInteractable* Interactable = Cast<IInteractable>(HitResult.GetActor());
		if (Interactable)
		{
			COMMON_LOG(LogGameplay, Log, TEXT("Interactable hit : %s"), *HitResult.GetActor()->GetName());
			Interactable->Interact(Cast<APawn>(Owner));
		}
	}
	else
		COMMON_LOG(LogGameplay, Log, TEXT("No hits"));
}
