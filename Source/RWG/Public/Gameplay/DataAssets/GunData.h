// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DataAssets/WeaponData.h"
#include "Gameplay/Data/RecoilSettings.h"
#include "Gameplay/Enums/WeaponTypes.h"
#include "GunData.generated.h"

class ABulletProjectile;
class UBulletImpactData;
class UNiagaraSystem;
class USoundBase;
class USkeletalMesh;

/**
 * 
 */
UCLASS()
class RWG_API UGunData : public UWeaponData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	int32 MagazineSize = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float FireRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	float Range = 5000.0f;

	// 이 총이 쓸 수 있는 발사 모드 (첫 번째가 기본 모드, 전환 키를 누르면 순서대로 바뀐다)
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	TArray<EFireMode> AvailableFireModes = { EFireMode::Auto };

	// 점사 한 번에 쏘는 발수
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	int32 BurstCount = 3;

	// 한 발마다 수동 장전(볼트 조작)이 필요한지 여부
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	bool bRequiresManualCycling = false;

	// 재장전할 때 탄을 한 발씩 넣는지 여부 (시작 → 한 발 반복 → 끝 몽타주로 진행)
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	bool bLoadsOneRoundAtATime = false;

	// 한 번 쏠 때 나가는 투사체 수
	UPROPERTY(EditDefaultsOnly, Category = "Gun", meta = (ClampMin = "1"))
	int32 PelletCount = 1;

	// 투사체가 퍼지는 원뿔의 반각(도)
	UPROPERTY(EditDefaultsOnly, Category = "Gun", meta = (ClampMin = "0.0"))
	float PelletSpreadAngle = 0.0f;

	// 소모 탄약 타입
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	TObjectPtr<UItemData> AmmoType = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	FRecoilSettings Recoil;

	// 총 메시
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Mesh")
	TObjectPtr<USkeletalMesh> GunMesh;

	// 총 메시가 쓰는 애니메이션 블루프린트
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Mesh")
	TSubclassOf<UAnimInstance> GunAnimClass;

	// 총 메시와 따로 분리된 탄창 메시 (총 메시에 탄창이 포함되어 있으면 비워 둔다)
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Mesh")
	TObjectPtr<USkeletalMesh> MagazineMesh;

	// 탄창 메시를 붙일 총 메시의 본 이름
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Mesh")
	FName MagazineBoneName = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	TSubclassOf<ABulletProjectile> BulletProjectileClass;

	// 한 번 쏠 때의 총 피해 (투사체가 여러 개면 나눠 갖는다)
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	float BulletDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	float BulletSpeed = 30000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	float BulletLifeTime = 3.0f;

	// 탄이 맞은 대상별 Bullet Impact·Bullet Decal 목록
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Projectile")
	TObjectPtr<UBulletImpactData> BulletImpactData;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Socket")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Socket")
	FName EjectionPortSocketName = TEXT("EjectionPort");

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Socket")
	FName HandGuardSocketName = TEXT("HandGuard");

	UPROPERTY(EditDefaultsOnly, Category = "Gun|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|FX")
	TObjectPtr<UNiagaraSystem> ShellEjectFX;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Sound")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gun|Sound")
	TObjectPtr<USoundBase> DryFireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterReloadEmptyMontage;

	// 한 발씩 재장전할 때 한 발을 넣는 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterReloadLoopMontage;

	// 한 발씩 재장전을 마무리하는 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterReloadEndMontage;

	// 발사할 때 캐릭터가 재생하는 몽타주 (수동 장전 총은 볼트 조작까지 포함)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> CharacterFireMontage;

	// 아래는 무기 메시가 재생한다. 캐릭터 몽타주와 1:1 짝으로 authoring된 것을 물릴 것
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshReloadEmptyMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshReloadLoopMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshReloadEndMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshEquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshUnequipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> GunMeshFireMontage;
};
