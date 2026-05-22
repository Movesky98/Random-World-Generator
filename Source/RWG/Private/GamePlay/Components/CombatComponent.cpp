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
	SetIsReplicatedByDefault(true);
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
	DOREPLIFETIME(UCombatComponent, WeaponActionState);
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

	InputComponent->BindAction(Config->ReloadAction, ETriggerEvent::Started, this, &ThisClass::TryReload);

	InputComponent->BindAction(Config->AttackAction, ETriggerEvent::Started, this, &ThisClass::RequestStartAttack);
	InputComponent->BindAction(Config->AttackAction, ETriggerEvent::Completed, this, &ThisClass::RequestStopAttack);
}

void UCombatComponent::RequestEquipWeapon_Implementation(AWeaponBase* NewWeapon)
{
	// 같은 무기 요청
	if (CurrentWeapon == NewWeapon)
	{
		PendingWeapon = nullptr;

		if (WeaponActionState == EWeaponActionState::None)
		{
			UnequipWeapon();
		}

		return;
	}

	// 다른 무기 요청
	PendingWeapon = NewWeapon;

	if (WeaponActionState != EWeaponActionState::None)
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

void UCombatComponent::RequestStartAttack()
{
	if (WeaponActionState != EWeaponActionState::None || !CurrentWeapon) return;

	Server_RequestStartAttack();
}

void UCombatComponent::RequestStopAttack()
{
	if (WeaponActionState != EWeaponActionState::Attack)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Check WeaponActionState. it must be Attack."));
	}

	Server_RequestStopAttack();
}

void UCombatComponent::SetCurrentWeapon(AWeaponBase* NewWeapon)
{
	// 이전 무기 있을 경우
	if (AGunBase* Gun = Cast<AGunBase>(CurrentWeapon))
	{
		Gun->OnAmmoChangedDelegate.RemoveAll(this);
		Gun->SetOwner(nullptr);
	}

	CurrentWeapon = nullptr;

	if (AGunBase* Gun = Cast<AGunBase>(NewWeapon))
	{
		Gun->OnAmmoChangedDelegate.AddUObject(this, &ThisClass::OnAmmoChanged);

		OnAmmoChangedDelegate.Broadcast(Gun->GetCurrentAmmo(), Gun->GetMagazineSize());

		CurrentWeapon = Gun;
		CurrentWeapon->SetOwner(GetOwner());
	}

	OnCurrentWeaponChanged.Broadcast(CurrentWeapon);
}
void UCombatComponent::EquipWeapon(AWeaponBase* NewWeapon)
{
	if (CurrentWeapon)
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("CurrentWeapon: %s. EquipWeapon() is called while CurrentWeapon is already valid."), *CurrentWeapon->GetName());
	}

	SetCurrentWeapon(NewWeapon);

	CurrentWeapon->Equip(Cast<ACharacter>(GetOwner()));
	WeaponActionState = EWeaponActionState::Equip;

	UAnimMontage* Montage = nullptr;
	if (UWeaponData* Data = CurrentWeapon->GetItemData<UWeaponData>())
	{
		Montage = Data->EquipMontage;
	}

	Montage ? Multicast_PlayMontage(Montage, EWeaponActionState::Equip) : OnEquipEnded();
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

	WeaponActionState = EWeaponActionState::Unequip;

	Montage ? Multicast_PlayMontage(Montage, EWeaponActionState::Unequip) : OnUnequipEnded();
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
		WeaponActionState = EWeaponActionState::None;
		return;
	}

	UnequipWeapon();
}
void UCombatComponent::OnUnequipEnded()
{
	CurrentWeapon->AttachToHolster(Cast<ACharacter>(GetOwner()));
	SetCurrentWeapon(nullptr);

	if (!PendingWeapon)
	{
		WeaponActionState = EWeaponActionState::None;
		return;
	}

	AWeaponBase* EquipToWeapon = PendingWeapon;
	PendingWeapon = nullptr;

	EquipWeapon(EquipToWeapon);
}

void UCombatComponent::TryReload()
{
	if (WeaponActionState != EWeaponActionState::None || !CurrentWeapon) return;

	RequestReload();
}

void UCombatComponent::RequestReload_Implementation()
{
	Reload();
}

void UCombatComponent::Reload()
{
	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun) return;

	UGunData* GunData = Gun->GetItemData<UGunData>();
	if (GunData && GunData->AmmoType)
	{
		if (GunData->MagazineSize - Gun->GetCurrentAmmo() <= 0) return;
		
		WeaponActionState = EWeaponActionState::Reload;
		Multicast_PlayMontage(GunData->ReloadMontage, EWeaponActionState::Reload);
	}
}

void UCombatComponent::OnReloadEnded()
{
	WeaponActionState = EWeaponActionState::None;

	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun) return;

	if (UGunData* GunData = Gun->GetItemData<UGunData>())
	{
		int32 NeededAmmo = GunData->MagazineSize - Gun->GetCurrentAmmo();
		int32 Available = InventoryComponent->GetAmmoCount(GunData->AmmoType);
		int32 ToLoad = FMath::Min(NeededAmmo, Available);

		InventoryComponent->ConsumeAmmo(GunData->AmmoType, ToLoad);
		Gun->SetCurrentAmmo(Gun->GetCurrentAmmo() + ToLoad);
	}
}

void UCombatComponent::OnAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (CurrentAmmo <= 0 || MaxAmmo <= 0) return;

	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, MaxAmmo);
}

void UCombatComponent::Server_RequestStartAttack_Implementation()
{
	if (!CurrentWeapon) return;

	WeaponActionState = EWeaponActionState::Attack;
	CurrentWeapon->StartAttack();
}

void UCombatComponent::Server_RequestStopAttack_Implementation()
{
	if (!CurrentWeapon) return;

	CurrentWeapon->StopAttack();
	WeaponActionState = EWeaponActionState::None;
}

void UCombatComponent::OnRep_CurrentWeapon(AWeaponBase* OldWeapon)
{
	if (AGunBase* OldGun = Cast<AGunBase>(OldWeapon))
	{
		OldGun->OnAmmoChangedDelegate.RemoveAll(this);
	}

	OnCurrentWeaponChanged.Broadcast(CurrentWeapon);

	if (AGunBase* Gun = Cast<AGunBase>(CurrentWeapon))
	{
		Gun->OnAmmoChangedDelegate.AddUObject(this, &ThisClass::OnAmmoChanged);
	}
}

void UCombatComponent::ApplyCurrentWeaponAnimLayer()
{
	if (CurrentWeapon)
	{
		UWeaponData* Data = CurrentWeapon->GetItemData<UWeaponData>();

		switch (WeaponActionState)
		{
		case EWeaponActionState::Equip:
			if (Data && Data->AnimLayerClass)
			{
				SetAnimLayer(Data->AnimLayerClass);
			}
			break;
		case EWeaponActionState::Unequip:
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

void UCombatComponent::Multicast_PlayMontage_Implementation(UAnimMontage* Montage, EWeaponActionState ActionState)
{
	if (!Montage) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	if (OwnerCharacter->HasAuthority())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ThisClass::OnMontageEnded);

		PlayMontage(Montage, EndDelegate);
	}
	else
	{
		PlayMontage(Montage);
	}
}

void UCombatComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		WeaponActionState = EWeaponActionState::None;
		return;
	}

	switch (WeaponActionState)
	{
	case EWeaponActionState::None:
		break;
	case EWeaponActionState::Equip:
		OnEquipEnded();
		break;
	case EWeaponActionState::Unequip:
		OnUnequipEnded();
		break;
	case EWeaponActionState::Reload:
		OnReloadEnded();
		break;
	default:
		break;
	}
}
