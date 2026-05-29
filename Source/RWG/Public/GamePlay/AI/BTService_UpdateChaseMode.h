// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateChaseMode.generated.h"

/**
 * 
 */
UCLASS()
class RWG_API UBTService_UpdateChaseMode : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_UpdateChaseMode();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	FVector CalculateSlotLocation(AActor* Target, AActor* Owner, float MaxSlotDistanceFromTarget) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chase|Slot")
	float MaxSlotAngleOffset;
};
