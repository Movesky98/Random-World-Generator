// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()
	
	// 공격 주체
	UPROPERTY()
	TObjectPtr<AActor> Instigator;

	// 데미지를 가한 무기
	UPROPERTY()
	TObjectPtr<AActor> DamageCauser;

	// 맞은 부위
	UPROPERTY()
	FName HitBoneName;

	// 기본 데미지
	UPROPERTY()
	float BaseDamage;

	// 데미지 유형 (총기, 폭발, 근접)
	UPROPERTY()
	TSubclassOf<UDamageType> DamageType;

	FDamageInfo() : Instigator(nullptr), DamageCauser(nullptr), HitBoneName(NAME_None), BaseDamage(0.0f), DamageType(nullptr)
	{  }
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RWG_API IDamageable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ProcessDamage(const FDamageInfo& DamageInfo) = 0;
};
