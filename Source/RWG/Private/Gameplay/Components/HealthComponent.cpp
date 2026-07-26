// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/HealthComponent.h"
#include "Gameplay/UI/PlayerHUD.h"
#include "CommonLogCategories.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UHealthComponent::InitializeComponent()
{
	Super::InitializeComponent();

	CurrentHealth = MaxHealth;
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

void UHealthComponent::DecreaseHealth(float FinalDamage, const FDamageInfo& DamageInfo)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("This function must be called on the server."));
		return;
	}

	if (IsDead()) return;

	COMMON_LOG(LogGameplay, Log, TEXT("%s's CurrentHealth: %.3f"), *GetNameSafe(GetOwner()), CurrentHealth);
	CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (IsDead())
	{
		StopRegen();
		OnDeathByDamage.Broadcast(DamageInfo);

		FString MessageStr;
		MessageStr += GetNameSafe(GetOwner()) + FString(" is dead.\n");
		MessageStr += FString("killed by ") + GetNameSafe(DamageInfo.Instigator);
		MessageStr += FString(" with ") + GetNameSafe(DamageInfo.DamageCauser);

		COMMON_LOG(LogGameplay, Log, TEXT("%s"), *MessageStr);
		return;
	}

	StartRegen();
}

void UHealthComponent::HealInstant(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("This function must be called on the server."));
		return;
	}

	if (IsDead()) return;

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::StartRegen()
{
	StopRegen();

	GetWorld()->GetTimerManager().SetTimer(RegenTimer, this, &UHealthComponent::RegenHealth, RegenTickInterval, true, RegenDelay);
}

void UHealthComponent::RegenHealth()
{
	if (CurrentHealth >= MaxHealth)
	{
		StopRegen();
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + RegenAmount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::StopRegen()
{
	GetWorld()->GetTimerManager().ClearTimer(RegenTimer);
}

void UHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

/********************************** IWidgetBindable **********************************/

TArray<TSubclassOf<UUserWidgetBase>> UHealthComponent::GetDefaultWidgetClasses() const
{
	return { UPlayerHUD::StaticClass() };
}

void UHealthComponent::BindComponent(UUserWidgetBase* Widget)
{
	if (UPlayerHUD* HUD = Cast<UPlayerHUD>(Widget))
	{
		OnHealthChanged.AddUObject(HUD, &UPlayerHUD::SetHealth);
		HUD->SetHealth(CurrentHealth, MaxHealth);
	}
}

void UHealthComponent::UnbindComponent(UUserWidgetBase* Widget)
{
	if (UPlayerHUD* HUD = Cast<UPlayerHUD>(Widget))
	{
		OnHealthChanged.RemoveAll(HUD);
	}
}
