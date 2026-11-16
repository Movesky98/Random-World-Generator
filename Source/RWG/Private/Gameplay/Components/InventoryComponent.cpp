// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/InventoryComponent.h"
#include "Gameplay/DataAssets/InventoryInputConfig.h"
#include "Gameplay/Items/WeaponBase.h"
#include "Gameplay/DataAssets/WeaponData.h"
#include "Gameplay/DataAssets/ArmorData.h"
#include "Gameplay/UI/InventoryWidget.h"
#include "Gameplay/UI/PlayerHUD.h"
#include "CommonLogCategories.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);

	Weapons.Init(nullptr, WeaponSlotCount);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryComponent, Slots, COND_OwnerOnly);
	DOREPLIFETIME(UInventoryComponent, Weapons);
}


const TArray<FItemInstance> UInventoryComponent::GetSlots()
{
	return Slots.Num() ? Slots : TArray<FItemInstance>();
}

void UInventoryComponent::BindInputActions(UEnhancedInputComponent* InputComponent)
{
	UInventoryInputConfig* InventoryConfig = Cast<UInventoryInputConfig>(LoadedConfig);

	if (!InputComponent)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InputComponent is null. SetupInputComponent may not have been called."));
		return;
	}

	if (!InventoryConfig)
	{
		COMMON_LOG(LogGameplay, Error, TEXT("InventoryConfig is null. Check Asset Manager or config assignment."));
		return;
	}

	InputComponent->BindAction(InventoryConfig->ToggleInventoryAction, ETriggerEvent::Triggered, this, &ThisClass::ToggleInventory);
}

TSubclassOf<UInputConfigBase> UInventoryComponent::GetConfigClass()
{
	return UInventoryInputConfig::StaticClass();
}

void UInventoryComponent::ToggleInventory()
{
	OnInventoryToggled.Broadcast();

	COMMON_LOG(LogGameplay, Warning, TEXT("--------------------- Inventory State ---------------------"));

	if (Slots.Num())
	{
		for (int32 i = 0; i < Slots.Num(); i++)
		{
			FItemInstance& ItemInstance = Slots[i];

			COMMON_LOG(LogGameplay, Warning, TEXT("%d\t%s\t%d"),
				i,
				*ItemInstance.ItemData->DisplayName.ToString(),
				ItemInstance.Quantity
			);
		}
	}
	else
	{
		COMMON_LOG(LogGameplay, Warning, TEXT("No Item Slots."));
	}
	
}

void UInventoryComponent::OnRep_Slots()
{
	COMMON_LOG(LogGameplay, Warning, TEXT("Slots is changed."));
	OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::AddItem(const FItemInstance& NewItem)
{
	if (!NewItem.IsValid()) return false;
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;

	int32 RemainingQuantity = NewItem.Quantity;

	while (RemainingQuantity > 0)
	{
		int32 ExistingIndex = FindExistingSlot(NewItem.ItemData);

		if (ExistingIndex != INDEX_NONE)
		{
			FItemInstance& Existing = Slots[ExistingIndex];
			int32 CanAdd = NewItem.ItemData->MaxStackSize - Existing.Quantity;

			if (CanAdd > 0)
			{
				int32 Added = FMath::Min(CanAdd, RemainingQuantity);
				Existing.Quantity += Added;
				RemainingQuantity -= Added;
			}
			else
			{
				// 새 슬롯 생성
				FItemInstance NewSlot;
				NewSlot.ItemData = NewItem.ItemData;
				NewSlot.Quantity = FMath::Min(RemainingQuantity, NewItem.ItemData->MaxStackSize);
				RemainingQuantity -= NewSlot.Quantity;
				Slots.Add(NewSlot);
			}
		}
		else
		{
			// 새 슬롯 생성
			FItemInstance NewSlot;
			NewSlot.ItemData = NewItem.ItemData;
			NewSlot.Quantity = FMath::Min(RemainingQuantity, NewItem.ItemData->MaxStackSize);
			RemainingQuantity -= NewSlot.Quantity;
			Slots.Add(NewSlot);
		}
	}

	OnInventoryChanged.Broadcast();
	OnItemAdded.Broadcast(NewItem.ItemData, NewItem.Quantity);
	return true;
}

bool UInventoryComponent::RemoveItem(UItemData* ItemData, int32 Quantity)
{
	if (!ItemData || Quantity <= 0) return false;

	int32 Remainig = Quantity;
	while (Remainig > 0)
	{
		int32 Index = FindSlot(ItemData);
		if (Index == INDEX_NONE) return false;

		int32 ToRemove = FMath::Min(Remainig, Slots[Index].Quantity);
		Slots[Index].Quantity -= ToRemove;
		Remainig -= ToRemove;

		if (Slots[Index].Quantity == 0)
		{
			Slots.RemoveAt(Index);
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex)) return false;

	FItemInstance& instance = Slots[SlotIndex];
	UItemData* Data = instance.ItemData;
	if (!Data) return false;

	switch (Data->ItemType)
	{
	case EItemType::Weapon:
		break;
	case EItemType::Consumable:
		break;
	default:
		return false;
	}

	return true;
}

int32 UInventoryComponent::FindExistingSlot(UItemData* ItemData) const
{
	int32 Index = 0;

	for (auto& Slot : Slots)
	{
		if (Slot.ItemData == ItemData && Slot.Quantity < ItemData->MaxStackSize)
		{
			return Index;
		}

		Index++;
	}

	return INDEX_NONE;
}

int32 UInventoryComponent::FindSlot(UItemData* ItemData) const
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].ItemData == ItemData)
			return i;
	}

	return INDEX_NONE;
}

int32 UInventoryComponent::AddWeapon(AWeaponBase* NewWeapon, AWeaponBase* HeldWeapon, AWeaponBase*& OutDroppedWeapon)
{
	OutDroppedWeapon = nullptr;

	if (!NewWeapon) return INDEX_NONE;
	if (!GetOwner() || !GetOwner()->HasAuthority()) return INDEX_NONE;
	if (Weapons.Contains(NewWeapon)) return INDEX_NONE;

	UWeaponData* WeaponData = NewWeapon->GetItemData<UWeaponData>();
	if (!WeaponData) return INDEX_NONE;

	int32 FirstSlot = 0;
	int32 LastSlot = 0;
	GetSlotRange(WeaponData->SlotType, FirstSlot, LastSlot);

	// 칸 종류 범위에서 앞쪽 빈 칸을 찾는다
	int32 TargetSlot = INDEX_NONE;
	for (int32 i = FirstSlot; i <= LastSlot; i++)
	{
		if (!Weapons[i])
		{
			TargetSlot = i;
			break;
		}
	}

	// 빈 칸이 없으면 들고 있는 무기의 칸, 들고 있는 무기가 범위에 없으면 첫 칸의 무기를 버릴 대상으로 정한다
	if (TargetSlot == INDEX_NONE)
	{
		TargetSlot = FirstSlot;
		for (int32 i = FirstSlot; i <= LastSlot; i++)
		{
			if (HeldWeapon && Weapons[i] == HeldWeapon)
			{
				TargetSlot = i;
				break;
			}
		}

		OutDroppedWeapon = Weapons[TargetSlot];
	}

	Weapons[TargetSlot] = NewWeapon;
	NewWeapon->AttachToHolster(Cast<ACharacter>(GetOwner()));

	// 주운 무기를 캐릭터에 붙인 뒤 기존 무기를 버린다
	if (OutDroppedWeapon)
	{
		FRotator DropRotation;
		const FVector DropLocation = GetDropLocation(DropRotation);

		OutDroppedWeapon->Drop(DropLocation, DropRotation);
	}

	COMMON_LOG(LogGameplay, Log, TEXT("Weapon acquired: %s (Slot %d, Dropped: %s)"), *NewWeapon->GetName(), TargetSlot, *GetNameSafe(OutDroppedWeapon));
	return TargetSlot;
}

void UInventoryComponent::GetSlotRange(EWeaponSlotType SlotType, int32& OutFirstSlot, int32& OutLastSlot) const
{
	switch (SlotType)
	{
	case EWeaponSlotType::Primary:		OutFirstSlot = 0; OutLastSlot = 1; break;
	case EWeaponSlotType::Secondary:	OutFirstSlot = 2; OutLastSlot = 2; break;
	case EWeaponSlotType::Throwable:	OutFirstSlot = 3; OutLastSlot = 3; break;
	default:							OutFirstSlot = 0; OutLastSlot = 1; break;
	}
}

FVector UInventoryComponent::GetDropLocation(FRotator& OutDropRotation) const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		OutDropRotation = FRotator::ZeroRotator;
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	// 카메라 시선의 수평 방향
	OutDropRotation = FRotator(0.0f, OwnerPawn->GetControlRotation().Yaw, 0.0f);
	const FVector Forward = OutDropRotation.Vector();

	const FVector Start = OwnerPawn->GetActorLocation();
	const FVector Target = Start + Forward * DropDistance + FVector(0.0f, 0.0f, DropHeight);

	// 캐릭터와 캐릭터에 붙은 무기는 트레이스에서 뺀다
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	TArray<AActor*> AttachedActors;
	OwnerPawn->GetAttachedActors(AttachedActors);
	Params.AddIgnoredActors(AttachedActors);

	// 버릴 자리까지 막혀 있으면 막힌 지점에서 캐릭터 쪽으로 당긴다
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, Target, ECC_Visibility, Params))
	{
		return HitResult.Location - Forward * DropWallOffset;
	}

	return Target;
}

AWeaponBase* UInventoryComponent::GetWeaponAtSlot(int32 SlotIndex) const
{
	return Weapons.IsValidIndex(SlotIndex) ? Weapons[SlotIndex] : nullptr;
}

int32 UInventoryComponent::GetAmmoCount(UItemData* AmmoType) const
{
	if (!AmmoType) return 0;
	
	int32 Total = 0;
	for (const FItemInstance& Item : Slots)
	{
		if (Item.ItemData == AmmoType)
		{
			Total += Item.Quantity;
		}
	}

	return Total;
}

float UInventoryComponent::ProcessArmorHit(FName BoneName)
{
	// 머리
	if (BoneName == "spine_05")
	{
		if (HelmetSlot.ItemData && HelmetDurability > 0.0f)
		{
			if (UArmorData* ArmorData = Cast<UArmorData>(HelmetSlot.ItemData))
			{
				HelmetDurability = FMath::Max(HelmetDurability - 1.0f, 0.0f);
				return 1.0f - ArmorData->DamageReductionRate;
			}
		}
	}
	else if (BoneName == "spine_02" || BoneName == "spine_04")
	{
		// 몸통
		if (VestSlot.ItemData && VestDurability > 0.0f)
		{
			if (UArmorData* ArmorData = Cast<UArmorData>(VestSlot.ItemData))
			{
				VestDurability = FMath::Max(VestDurability - 1.0f, 0.0f);
				return 1.0f - ArmorData->DamageReductionRate;
			}
		}
	}

	return 1.0f;
}

TArray<TSubclassOf<UUserWidgetBase>> UInventoryComponent::GetDefaultWidgetClasses() const
{
	return { UInventoryWidget::StaticClass(), UPlayerHUD::StaticClass() };
}

void UInventoryComponent::BindComponent(UUserWidgetBase* Widget)
{
	if (UInventoryWidget* InvWidget = Cast<UInventoryWidget>(Widget))
	{
		InvWidget->InitInventory(this);
	}
	else if (UPlayerHUD* HUD = Cast<UPlayerHUD>(Widget))
	{
		COMMON_LOG(LogGameplay, Log, TEXT("Bind delegate for PlayerHUD"));
	}
}

void UInventoryComponent::UnbindComponent(UUserWidgetBase* Widget)
{
	if (UInventoryWidget* InvWidget = Cast<UInventoryWidget>(Widget))
	{
		OnInventoryChanged.RemoveAll(InvWidget);
		OnInventoryToggled.RemoveAll(InvWidget);
	}
	else if (UPlayerHUD* HUD = Cast<UPlayerHUD>(Widget))
	{
		COMMON_LOG(LogGameplay, Log, TEXT("Unbind delegate for PlayerHUD"));
	}
}

