// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GamePlay/Interfaces/Damageable.h"
#include "HealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float /* CurrentHealth */, float /* MaxHealth */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeath, const FDamageInfo& /* DamageInfo */);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWG_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	void DecreaseHealth(float FinalDamage, const FDamageInfo& DamageInfo);
	
	void HealInstant(float Amount);
	
	bool IsDead() const { return CurrentHealth <= 0.0f; }
	
	float GetCurrentHealth() const { return CurrentHealth; }

	float GetMaxHealth() const { return MaxHealth; }

	FOnHealthChanged OnHealthChanged;

	FOnDeath OnDeath;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth = 100;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, Category = "Health")
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Health|Regen")
	float RegenDelay = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health|Regen")
	float RegenAmount = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health|Regen")
	float RegenTickInterval = 1.0f;

	FTimerHandle RegenTimer;

	void StartRegen();

	void RegenHealth();

	void StopRegen();

	UFUNCTION()
	void OnRep_CurrentHealth();
};
