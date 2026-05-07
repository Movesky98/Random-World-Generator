// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/InteractionComponent.h"
#include "GamePlay/DataAssets/InteractionInputConfig.h"
#include "GamePlay/Interfaces/Interactable.h"
#include "CommonLogCategories.h"
#include "Kismet/KismetSystemLibrary.h"

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

#if ENABLE_DRAW_DEBUG
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Owner);

	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		CameraLocation,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(TRACE_INTERACTION),
		true,
		IgnoreActors,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		2.0f
	);
#endif

	if (bIsHit && HitResult.GetActor())
	{
		AActor* TargetActor = HitResult.GetActor();

		if (TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
		{
			COMMON_LOG(LogGameplay, Log, TEXT("Interactable hit : %s"), *TargetActor->GetName());
			Server_RequestInteract(TargetActor);
			// Interactable->Interact(Cast<APawn>(Owner));
		}
	}
	else
		COMMON_LOG(LogGameplay, Log, TEXT("No hits"));
}

void UInteractionComponent::Server_RequestInteract_Implementation(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Interactable Actor is not valid."));
		return;
	}
	
	if (IInteractable* InteractableActor = Cast<IInteractable>(TargetActor))
	{
		InteractableActor->Interact(GetOwner());
	}
}
