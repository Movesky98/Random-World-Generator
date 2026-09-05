// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/CombatComponent.h"
#include "Gameplay/Components/InventoryComponent.h"
#include "Gameplay/DataAssets/CombatInputConfig.h"
#include "Gameplay/Items/WeaponBase.h"
#include "Gameplay/Items/WeaponData.h"
#include "Gameplay/Items/GunBase.h"
#include "Gameplay/UI/PlayerHUD.h"
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

	InputComponent->BindAction(Config->UnequipWeaponAction, ETriggerEvent::Started, this, &ThisClass::Server_RequestUnequipWeapon);
	InputComponent->BindAction(Config->ReloadAction, ETriggerEvent::Started, this, &ThisClass::Server_RequestReload);
	InputComponent->BindAction(Config->AttackAction, ETriggerEvent::Started, this, &ThisClass::Server_RequestStartAttack);
	InputComponent->BindAction(Config->AttackAction, ETriggerEvent::Completed, this, &ThisClass::Server_RequestStopAttack);
}

void UCombatComponent::SelectWeaponSlot(int32 SlotIndex)
{
	if (!InventoryComponent) return;

	AWeaponBase* Weapon = InventoryComponent->GetWeaponAtSlot(SlotIndex);
	if (!Weapon) return;

	Server_RequestEquipWeapon(Weapon);
}

void UCombatComponent::SetCurrentWeapon(AWeaponBase* NewWeapon)
{
	if (AGunBase* Gun = Cast<AGunBase>(CurrentWeapon))
	{
		Gun->OnAmmoChangedDelegate.RemoveAll(this);
		Gun->SetOwner(nullptr);
	}

	CurrentWeapon = nullptr;

	if (AGunBase* Gun = Cast<AGunBase>(NewWeapon))
	{
		Gun->OnAmmoChangedDelegate.AddUObject(this, &ThisClass::OnAmmoChanged);
		Gun->SetOwner(GetOwner());
		CurrentWeapon = Gun;
		
		OnAmmoChangedDelegate.Broadcast(Gun->GetCurrentAmmo(), Gun->GetMagazineSize());
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

	UAnimMontage* Montage = CurrentWeapon->GetCharacterMontage(EWeaponActionState::Equip);

	Montage ? Multicast_PlayMontage(CurrentWeapon, EWeaponActionState::Equip) : OnEquipEnded();
}
void UCombatComponent::UnequipWeapon()
{
	if (!CurrentWeapon)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("UnequipWeapon() is called while CurrentWeapon is nullptr."));
		return;
	}

	CurrentWeapon->StopAttack();

	UAnimMontage* Montage = CurrentWeapon->GetCharacterMontage(EWeaponActionState::Unequip);

	WeaponActionState = EWeaponActionState::Unequip;

	Montage ? Multicast_PlayMontage(CurrentWeapon, EWeaponActionState::Unequip) : OnUnequipEnded();
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
	if (!CurrentWeapon)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("CurrentWeapon is nullptr."));
		return;
	}

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

void UCombatComponent::Reload(EWeaponActionState ActionState)
{
	CurrentWeapon->StopAttack();

	WeaponActionState = ActionState;
	Multicast_PlayMontage(CurrentWeapon, ActionState);
}

void UCombatComponent::OnReloadEnded()
{
	WeaponActionState = EWeaponActionState::None;

	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun) return;

	if (!InventoryComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InventoryComponent is nullptr."));
		return;
	}

	int32 AvailableAmmo = InventoryComponent->GetAmmoCount(Gun->GetAmmoType());
	int32 ConsumeAmmo = Gun->GetLoadableAmmo(AvailableAmmo);

	InventoryComponent->RemoveItem(Gun->GetAmmoType(), ConsumeAmmo);
	Gun->SetCurrentAmmo(Gun->GetCurrentAmmo() + ConsumeAmmo);
}

void UCombatComponent::OnAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (CurrentAmmo < 0 || MaxAmmo <= 0) return;

	OnAmmoChangedDelegate.Broadcast(CurrentAmmo, MaxAmmo);
}

void UCombatComponent::Server_RequestEquipWeapon_Implementation(AWeaponBase* NewWeapon)
{
	if (CurrentWeapon == NewWeapon) return;
	if (WeaponActionState != EWeaponActionState::None) return;

	if (!CurrentWeapon)
	{
		EquipWeapon(NewWeapon);
		return;
	}

	PendingWeapon = NewWeapon;
	UnequipWeapon();
}

void UCombatComponent::Server_RequestUnequipWeapon_Implementation()
{
	PendingWeapon = nullptr;

	if (!CurrentWeapon) return;
	if (WeaponActionState == EWeaponActionState::Unequip) return;

	UnequipWeapon();
}

void UCombatComponent::Server_RequestReload_Implementation()
{
	if (WeaponActionState != EWeaponActionState::None) return;

	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("Failed to cast CurrentWeapon to GunBase."));
		return;
	}

	if (!InventoryComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InventoryComponent is nullptr."));
		return;
	}

	// Reload 검사 시작
	EWeaponActionState ReloadActionState = EWeaponActionState::None;

	int32 AvailableAmmo = InventoryComponent->GetAmmoCount(Gun->GetAmmoType());
	EReloadCondition ReloadCondition = Gun->CheckReloadCondition(AvailableAmmo, ReloadActionState);

	if (ReloadCondition == EReloadCondition::Ready)
		Reload(ReloadActionState);
	else
		COMMON_LOG(LogGameplay, Warning, TEXT("Reload failed. State : %s"), *UEnum::GetValueAsString(ReloadCondition));
}

void UCombatComponent::Server_RequestStartAttack_Implementation()
{
	if (!CurrentWeapon || WeaponActionState != EWeaponActionState::None) return;

	CurrentWeapon->StartAttack();
}

void UCombatComponent::Server_RequestStopAttack_Implementation()
{
	if (!CurrentWeapon) return;

	CurrentWeapon->StopAttack();
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
		OnAmmoChangedDelegate.Broadcast(Gun->GetCurrentAmmo(), Gun->GetMagazineSize());
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
		case EWeaponActionState::Reload:
		case EWeaponActionState::ReloadEmpty:
			break;
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

void UCombatComponent::Multicast_PlayMontage_Implementation(AWeaponBase* Weapon, EWeaponActionState ActionState)
{
	if (!Weapon) return;

	// 무기 메시 연출은 무기가 자기 데이터에서 골라 직접 재생한다
	Weapon->PlayActionMontage(ActionState);

	UAnimMontage* Montage = Weapon->GetCharacterMontage(ActionState);
	if (!Montage) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	if (OwnerCharacter->HasAuthority())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ThisClass::OnMontageEnded, ActionState);

		PlayMontage(Montage, EndDelegate);
	}
	else
	{
		PlayMontage(Montage);
	}
}

void UCombatComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, EWeaponActionState EndedAction)
{
	if (bInterrupted)
	{
		if (WeaponActionState == EndedAction)
			WeaponActionState = EWeaponActionState::None;

		return;
	}

	switch (EndedAction)
	{
	case EWeaponActionState::Equip:
		OnEquipEnded();
		break;
	case EWeaponActionState::Unequip:
		OnUnequipEnded();
		break;
	case EWeaponActionState::Reload:
	case EWeaponActionState::ReloadEmpty:
		OnReloadEnded();
		break;
	}
}

/********************************** IWidgetBindable **********************************/

TArray<TSubclassOf<UUserWidgetBase>> UCombatComponent::GetDefaultWidgetClasses() const
{
	return { UPlayerHUD::StaticClass() };
}

void UCombatComponent::BindComponent(UUserWidgetBase* Widget)
{
	if (UPlayerHUD* HUD = Cast<UPlayerHUD>(Widget))
	{
		OnAmmoChangedDelegate.AddUObject(HUD, &UPlayerHUD::SetAmmo);
		OnCurrentWeaponChanged.AddUObject(HUD, &UPlayerHUD::SetWeapon);
	}
}

void UCombatComponent::UnbindComponent(UUserWidgetBase* Widget)
{
	if (UPlayerHUD* HUD = Cast<UPlayerHUD>(Widget))
	{
		OnAmmoChangedDelegate.RemoveAll(HUD);
		OnCurrentWeaponChanged.RemoveAll(HUD);
	}
}
