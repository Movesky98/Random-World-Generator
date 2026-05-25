// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/UI/PlayerHUD.h"
#include "GamePlay/Items/WeaponBase.h"
#include "CommonLogCategories.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

UPlayerHUD::UPlayerHUD()
{

}

void UPlayerHUD::SetUp()
{
	this->AddToViewport();

	FInputModeGameOnly InputModeData;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController)) return;

	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(false);
}

void UPlayerHUD::SetAmmo(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (MaxAmmo < 0 || CurrentAmmo < 0)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("MaxAmmo: %d, CurrentAmmo: %d"), MaxAmmo, CurrentAmmo);
		return;
	}

	FString AmmoStr = FString::FromInt(CurrentAmmo) + FString(" / ") + FString::FromInt(MaxAmmo);	
	AmmoTextBlock->SetText(FText::FromString(AmmoStr));
}

void UPlayerHUD::SetWeapon(AWeaponBase* Weapon)
{
	// Do nothing yet.
	
	Weapon ? AmmoTextBlock->SetVisibility(ESlateVisibility::Visible) : 
		AmmoTextBlock->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerHUD::SetHealth(float CurrentHealth, float MaxHealth)
{
	if (MaxHealth <= 0.0f) return;
	HealthProgressBar->SetPercent(CurrentHealth / MaxHealth);
}
