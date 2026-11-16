// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/WeaponBase.h"
#include "Gameplay/DataAssets/WeaponData.h"
#include "Gameplay/Components/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AWeaponBase::AWeaponBase()
{

}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// 떨어지는 무기가 얇은 바닥을 통과하지 않게 연속 충돌 검사를 켠다
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrimitive->SetAllUseCCD(true);
	}
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, AttachState);
}

void AWeaponBase::Interact(AActor* Interactor)
{
	if (!HasAuthority()) return;

	// 이미 다른 캐릭터에게 붙어 있는 무기는 주울 수 없다
	if (AttachState.OwnerCharacter) return;

	UCombatComponent* CombatComp = Interactor->FindComponentByClass<UCombatComponent>();
	if (!CombatComp) return;

	CombatComp->PickUpWeapon(this);
}

void AWeaponBase::StartAttack()
{

}

void AWeaponBase::StopAttack()
{

}

void AWeaponBase::ReleaseAttack()
{
	StopAttack();
}

void AWeaponBase::Equip(ACharacter* NewOwner)
{
	if (!NewOwner) return;

	UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
	if (!WeaponData) return;

	SetAttachState(NewOwner, WeaponData->EquipSocketName);
}

UAnimMontage* AWeaponBase::GetCharacterMontage(EWeaponActionState ActionState) const
{
	UWeaponData* WeaponData = GetItemData<UWeaponData>();
	if (!WeaponData) return nullptr;

	switch (ActionState)
	{
	case EWeaponActionState::Equip:		return WeaponData->CharacterEquipMontage;
	case EWeaponActionState::Unequip:	return WeaponData->CharacterUnequipMontage;
	default:							return nullptr;
	}
}

void AWeaponBase::PlayActionMontage(EWeaponActionState ActionState)
{
	// AWeaponBase에는 메시 컴포넌트가 없다. 메시를 가진 파생 클래스가 구현한다
}

void AWeaponBase::AttachToHolster(ACharacter* NewOwner)
{
	if (!NewOwner) return;

	UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
	if (!WeaponData) return;

	SetAttachState(NewOwner, WeaponData->HolsterSocketName);
}

void AWeaponBase::Drop(const FVector& DropLocation, const FRotator& DropRotation)
{
	if (!HasAuthority()) return;

	StopAttack();

	SetOwner(nullptr);
	SetAttachState(nullptr, NAME_None);

	SetActorLocationAndRotation(DropLocation, DropRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// 캐릭터에 붙어 움직이던 속도를 지운다
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrimitive->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		RootPrimitive->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void AWeaponBase::OnRep_AttachState()
{
	ApplyAttachState();
}

void AWeaponBase::SetAttachState(ACharacter* NewOwner, FName SocketName)
{
	if (!HasAuthority()) return;

	AttachState.OwnerCharacter = NewOwner;
	AttachState.SocketName = SocketName;

	ApplyAttachState();
}

void AWeaponBase::ApplyAttachState()
{
	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent());
	if (!RootPrimitive) return;

	if (AttachState.OwnerCharacter)
	{
		// 물리와 충돌을 끈 뒤 캐릭터 메시 소켓에 붙인다
		RootPrimitive->SetSimulatePhysics(false);
		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AttachToComponent(
			AttachState.OwnerCharacter->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			AttachState.SocketName
		);
	}
	else
	{
		// 캐릭터에서 떼고 쿼리·물리 충돌과 물리 시뮬레이션을 켠다
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootPrimitive->SetSimulatePhysics(true);
	}

	// 바닥에 놓인 동안만 서버의 위치·물리 상태를 클라이언트에 보낸다
	if (HasAuthority())
	{
		SetReplicateMovement(!AttachState.OwnerCharacter);
	}
}
