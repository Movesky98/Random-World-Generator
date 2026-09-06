// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "InputHandlerComponent.generated.h"

class UInputConfigBase;
class UInputComponentBase;
class ULocomotionComponent;
class IInputBindable;

namespace InputConfgTags
{
	const FName BaseName = FName("InputConfig");
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UInputHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this component's properties
	UInputHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void InitializeComponent() override;

/*********************************************************************
*                            캐시된 참조
*********************************************************************/
protected:
	UPROPERTY()
	APlayerController* OwnerController;

/*********************************************************************
*                          입력 바인딩 관리
*********************************************************************/
public:
	void RegisterBindableComponents(TArray<TScriptInterface<IInputBindable>>& Components, UEnhancedInputComponent* EnhancedInput);

	void RequestActivateIMC(IInputBindable* Requester, bool bExclusive = false);

	void RequestDeactiveIMC(IInputBindable* Requester);

protected:
	TArray<TScriptInterface<IInputBindable>> RegisteredComponents;
};
