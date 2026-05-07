// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Components/CombatComponent.h"
#include "GamePlay/Components/InventoryComponent.h"
#include "GamePlay/DataAssets/CombatInputConfig.h"
#include "GamePlay/Items/WeaponBase.h"
#include "GamePlay/Items/GunBase.h"
#include "GamePlay/Items/GunData.h"
#include "CommonLogCategories.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	bWantsInitializeComponent = true;
}


void UCombatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	InventoryComponent = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("CombatComponent must attach to Character with InventoryComponent."));
	}
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	SetAnimLayer(DefaultAnimLayerClass);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, CurrentWeapon);
}

TSubclassOf<UBaseInputConfig> UCombatComponent::GetConfigClass()
{
	return UCombatInputConfig::StaticClass();
}

void UCombatComponent::BindInputActions(UEnhancedInputComponent* InputComponent)
{
	UCombatInputConfig* Config = Cast<UCombatInputConfig>(LoadedConfig);

	if (!InputComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InputComponent is null. SetupInputComponent may not have been called."));
		return;
	}

	if (!Config)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InventoryConfig is null. Check Asset Manager or config assignment."));
		return;
	}

	for (int32 i = 0; i < Config->EquipWeaponActions.Num(); i++)
	{
		if (Config->EquipWeaponActions[i])
		{
			InputComponent->BindAction(Config->EquipWeaponActions[i], ETriggerEvent::Started, this, &ThisClass::SelectWeaponSlot, i);
		}
	}

	// InputComponent->BindAction(Config->ReloadAction, ETriggerEvent::Started, this, &ThisClass::Reload);
}

void UCombatComponent::RequestEquipWeapon(AWeaponBase* NewWeapon)
{
	// 같은 무기 요청
	if (CurrentWeapon == NewWeapon)
	{
		PendingWeapon = nullptr;

		if (WeaponTransitionState == EWeaponTransitionState::None)
		{
			UnequipWeapon();
		}

		return;
	}

	// 다른 무기 요청
	PendingWeapon = NewWeapon;

	if (WeaponTransitionState != EWeaponTransitionState::None)
	{
		return;
	}

	if (!CurrentWeapon)
	{
		EquipWeapon(PendingWeapon);
		PendingWeapon = nullptr;
		return;
	}

	UnequipWeapon();
}

void UCombatComponent::SelectWeaponSlot(int32 SlotIndex)
{
	if (!InventoryComponent) return;

	AWeaponBase* Weapon = InventoryComponent->GetWeaponAtSlot(SlotIndex);
	if (!Weapon) return;

	RequestEquipWeapon(Weapon);
}
void UCombatComponent::NotifyCurrentWeaponState()
{
	OnCurrentWeaponChanged.Broadcast(CurrentWeapon);

	if (!CurrentWeapon)
	{
		OnAmmoChanged.Broadcast(0, 0);
		return;
	}

	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun) return;
	
	UGunData* GunData = Gun->GetItemData<UGunData>();
	if (!GunData) return;

	OnAmmoChanged.Broadcast(Gun->GetCurrentAmmo(), GunData->MagazineSize);
}
void UCombatComponent::EquipWeapon(AWeaponBase* NewWeapon)
{
	if (CurrentWeapon)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("CurrentWeapon: %s. EquipWeapon() is called while CurrentWeapon is already valid."), *CurrentWeapon->GetName());
	}

	CurrentWeapon = NewWeapon;
	CurrentWeapon->Equip(Cast<ACharacter>(GetOwner()));
	WeaponTransitionState = EWeaponTransitionState::Equipping;

	NotifyCurrentWeaponState();

	UAnimMontage* Montage = nullptr;
	if (UWeaponData* Data = CurrentWeapon->GetItemData<UWeaponData>())
	{
		Montage = Data->EquipMontage;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ThisClass::OnEquipMontageEnded);

	Montage ? PlayMontage(Montage, EndDelegate) : OnEquipEnded();
}
void UCombatComponent::UnequipWeapon()
{
	if (!CurrentWeapon)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("UnequipWeapon() is called while CurrentWeapon is nullptr."));
		return;
	}

	UAnimMontage* Montage = nullptr;
	if (UWeaponData* Data = CurrentWeapon->GetItemData<UWeaponData>())
	{
		Montage = Data->UnequipMontage;
	}

	WeaponTransitionState = EWeaponTransitionState::Unequipping;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ThisClass::OnUnequipMontageEnded);

	Montage ? PlayMontage(Montage, EndDelegate) : OnUnequipEnded();
}

void UCombatComponent::OnEquipEnded()
{
	if (!CurrentWeapon)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("CurrentWeapon is nullptr."));
		return;
	}

	if (!PendingWeapon)
	{
		WeaponTransitionState = EWeaponTransitionState::None;
		return;
	}

	UnequipWeapon();
}
void UCombatComponent::OnUnequipEnded()
{
	CurrentWeapon->AttachToHolster(Cast<ACharacter>(GetOwner()));
	CurrentWeapon = nullptr;
	NotifyCurrentWeaponState();

	if (!PendingWeapon)
	{
		WeaponTransitionState = EWeaponTransitionState::None;
		return;
	}

	AWeaponBase* EquipToWeapon = PendingWeapon;
	PendingWeapon = nullptr;

	EquipWeapon(EquipToWeapon);
}

void UCombatComponent::Reload()
{

}

void UCombatComponent::onReloadEnded()
{

}

void UCombatComponent::OnUnequipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		WeaponTransitionState = EWeaponTransitionState::None;
		return;
	}

	OnUnequipEnded();
}

void UCombatComponent::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		WeaponTransitionState = EWeaponTransitionState::None;
		return;
	}

	OnEquipEnded();
}

void UCombatComponent::ApplyCurrentWeaponAnimLayer()
{
	if (CurrentWeapon)
	{
		UWeaponData* Data = CurrentWeapon->GetItemData<UWeaponData>();

		switch (WeaponTransitionState)
		{
		case EWeaponTransitionState::Equipping:
			if (Data && Data->AnimLayerClass)
			{
				SetAnimLayer(Data->AnimLayerClass);
			}
			break;
		case EWeaponTransitionState::Unequipping:
		default:
			SetAnimLayer(DefaultAnimLayerClass);
			break;
		}
	}
	else
		SetAnimLayer(DefaultAnimLayerClass);
}

void UCombatComponent::SetAnimLayer(TSubclassOf<UAnimInstance> AnimLayerClass)
{
	if (!AnimLayerClass) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->LinkAnimClassLayers(AnimLayerClass);
}

void UCombatComponent::PlayMontage(UAnimMontage* Montage, FOnMontageEnded EndDelegate)
{
	if (!Montage) return;
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(Montage);

	if (EndDelegate.IsBound())
	{
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

