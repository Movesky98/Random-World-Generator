// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_LoadOneRound.generated.h"

/**
 * 한 발씩 재장전하는 총의 탄창에 탄을 한 발 넣는 시점
 */
UCLASS()
class RWG_API UAnimNotify_LoadOneRound : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
