// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/CombatComponent.h"
#include "Gameplay/Components/InventoryComponent.h"
#include "Gameplay/Components/RecoilComponent.h"
#include "Gameplay/DataAssets/CombatInputConfig.h"
#include "Gameplay/Items/WeaponBase.h"
#include "Gameplay/DataAssets/WeaponData.h"
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

	RecoilComponent = GetOwner()->FindComponentByClass<URecoilComponent>();
	if (!RecoilComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("CombatComponent must attach to Character with RecoilComponent."));
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

TSubclassOf<UInputConfigBase> UCombatComponent::GetConfigClass()
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
	InputComponent->BindAction(Config->SwitchFireModeAction, ETriggerEvent::Started, this, &ThisClass::Server_RequestSwitchFireMode);
}

void UCombatComponent::SelectWeaponSlot(int32 SlotIndex)
{
	if (!InventoryComponent) return;

	AWeaponBase* Weapon = InventoryComponent->GetWeaponAtSlot(SlotIndex);
	if (!Weapon) return;

	Server_RequestEquipWeapon(Weapon);
}

void UCombatComponent::PickUpWeapon(AWeaponBase* NewWeapon)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!NewWeapon || !InventoryComponent) return;

	AWeaponBase* DroppedWeapon = nullptr;
	const int32 SlotIndex = InventoryComponent->AddWeapon(NewWeapon, CurrentWeapon, DroppedWeapon);
	if (SlotIndex == INDEX_NONE) return;

	// 빈 칸에 들어갔으면 등에 붙은 채로 끝난다
	if (!DroppedWeapon) return;

	// 칸이 차서 무기를 버렸으면 진행 중인 동작을 끊고 주운 무기를 손에 든다
	CancelActionMontage();

	if (CurrentWeapon == DroppedWeapon)
	{
		SetCurrentWeapon(nullptr);
	}

	if (!CurrentWeapon)
	{
		EquipWeapon(NewWeapon);
		return;
	}

	// 다른 칸의 무기를 들고 있으면 그 무기를 넣은 뒤 주운 무기를 든다
	PendingWeapon = NewWeapon;
	UnequipWeapon();
}

void UCombatComponent::CancelActionMontage()
{
	PendingWeapon = nullptr;
	bReloadStopRequested = false;

	if (WeaponActionState == EWeaponActionState::None) return;

	WeaponActionState = EWeaponActionState::None;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInstance = OwnerCharacter ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	UAnimMontage* ActiveMontage = AnimInstance->GetCurrentActiveMontage();
	if (!ActiveMontage) return;

	// 끊기는 몽타주의 종료·블렌드 아웃 콜백을 풀고 멈춘다
	FOnMontageEnded EmptyEndDelegate;
	AnimInstance->Montage_SetEndDelegate(EmptyEndDelegate, ActiveMontage);

	FOnMontageBlendingOutStarted EmptyBlendingOutDelegate;
	AnimInstance->Montage_SetBlendingOutDelegate(EmptyBlendingOutDelegate, ActiveMontage);

	AnimInstance->Montage_Stop(0.2f, ActiveMontage);
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

	if (RecoilComponent)
	{
		RecoilComponent->SetWeapon(CurrentWeapon);
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

	bReloadStopRequested = false;
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

bool UCombatComponent::IsOneRoundReloadStep(EWeaponActionState ActionState) const
{
	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun || !Gun->LoadsOneRoundAtATime()) return false;

	return ActionState == EWeaponActionState::Reload
		|| ActionState == EWeaponActionState::ReloadEmpty
		|| ActionState == EWeaponActionState::ReloadLoop;
}

void UCombatComponent::AdvanceOneRoundReload()
{
	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun || !InventoryComponent)
	{
		WeaponActionState = EWeaponActionState::None;
		return;
	}

	// 사격 요청이 없고 더 넣을 탄이 있으면 한 발을 더 넣고, 아니면 끝 몽타주로 마무리한다
	const int32 AvailableAmmo = InventoryComponent->GetAmmoCount(Gun->GetAmmoType());
	const bool bCanLoadMore = !bReloadStopRequested && Gun->GetLoadableAmmo(AvailableAmmo) > 0;
	const EWeaponActionState NextAction = bCanLoadMore ? EWeaponActionState::ReloadLoop : EWeaponActionState::ReloadEnd;

	if (!CurrentWeapon->GetCharacterMontage(NextAction))
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("Montage for %s is missing. Weapon : %s"), *UEnum::GetValueAsString(NextAction), *Gun->GetName());
		WeaponActionState = EWeaponActionState::None;
		return;
	}

	WeaponActionState = NextAction;
	Multicast_PlayMontage(CurrentWeapon, NextAction);
}

void UCombatComponent::LoadOneRound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!IsOneRoundReloadStep(WeaponActionState)) return;

	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun || !InventoryComponent) return;

	const int32 AvailableAmmo = InventoryComponent->GetAmmoCount(Gun->GetAmmoType());
	if (Gun->GetLoadableAmmo(AvailableAmmo) <= 0) return;

	InventoryComponent->RemoveItem(Gun->GetAmmoType(), 1);
	Gun->SetCurrentAmmo(Gun->GetCurrentAmmo() + 1);
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
	// 한 발씩 재장전 중이면 쏘지 않고, 지금 단계가 끝나면 끝 몽타주로 가도록 표시만 한다
	if (IsOneRoundReloadStep(WeaponActionState))
	{
		bReloadStopRequested = true;
		return;
	}

	if (!CurrentWeapon || WeaponActionState != EWeaponActionState::None) return;

	CurrentWeapon->StartAttack();
}

void UCombatComponent::Server_RequestStopAttack_Implementation()
{
	if (!CurrentWeapon) return;

	CurrentWeapon->ReleaseAttack();
}

void UCombatComponent::Server_RequestSwitchFireMode_Implementation()
{
	if (!CurrentWeapon || WeaponActionState != EWeaponActionState::None) return;

	AGunBase* Gun = Cast<AGunBase>(CurrentWeapon);
	if (!Gun) return;

	Gun->SwitchFireMode();
}

void UCombatComponent::OnRep_CurrentWeapon(AWeaponBase* OldWeapon)
{
	if (AGunBase* OldGun = Cast<AGunBase>(OldWeapon))
	{
		OldGun->OnAmmoChangedDelegate.RemoveAll(this);
	}

	if (RecoilComponent)
	{
		RecoilComponent->SetWeapon(CurrentWeapon);
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
		case EWeaponActionState::ReloadLoop:
		case EWeaponActionState::ReloadEnd:
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

void UCombatComponent::PlayMontage(UAnimMontage* Montage, FOnMontageEnded EndDelegate, FOnMontageBlendingOutStarted BlendingOutDelegate)
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

	if (BlendingOutDelegate.IsBound())
	{
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
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

		FOnMontageBlendingOutStarted BlendingOutDelegate;
		BlendingOutDelegate.BindUObject(this, &ThisClass::OnMontageBlendingOut, ActionState);

		PlayMontage(Montage, EndDelegate, BlendingOutDelegate);
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
		// 같은 몽타주를 이어서 다시 재생한 경우(한 발 넣기 반복)에는 상태를 유지한다
		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		UAnimInstance* AnimInstance = OwnerCharacter ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
		const bool bSameMontageReplayed = AnimInstance && AnimInstance->Montage_IsActive(Montage);

		if (WeaponActionState == EndedAction && !bSameMontageReplayed)
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
		// 한 발씩 재장전은 블렌드 아웃 시점에 다음 단계로 넘어가므로 여기서 채우지 않는다
		if (IsOneRoundReloadStep(EndedAction)) break;
		OnReloadEnded();
		break;
	case EWeaponActionState::ReloadEnd:
		if (WeaponActionState == EWeaponActionState::ReloadEnd)
			WeaponActionState = EWeaponActionState::None;
		break;
	}
}

void UCombatComponent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted, EWeaponActionState BlendingOutAction)
{
	// 한 발씩 재장전 단계가 끝까지 재생되어 빠져나가기 시작하면 다음 단계 몽타주를 이어 튼다
	if (bInterrupted) return;
	if (WeaponActionState != BlendingOutAction) return;
	if (!IsOneRoundReloadStep(BlendingOutAction)) return;

	AdvanceOneRoundReload();
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
