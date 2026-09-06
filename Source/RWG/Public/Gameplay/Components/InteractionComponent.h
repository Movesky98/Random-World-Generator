// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Components/InputComponentBase.h"
#include "InteractionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UInteractionComponent : public UInputComponentBase
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
public:
	// Sets default values for this component's properties
	UInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

/*********************************************************************
*                             입력 처리
*********************************************************************/
protected:
	void BindInputActions(UEnhancedInputComponent* InputComponent) override;

	TSubclassOf<UInputConfigBase> GetConfigClass() override;

/*********************************************************************
*                              상호작용
*********************************************************************/
protected:
	void PerformInteractionTrace();

	UFUNCTION(Server, Reliable)
	void Server_RequestInteract(AActor* TargetActor);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
	float TraceDistance = 300.0f;
};
