// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Actors/BulletProjectile.h"
#include "Gameplay/DataAssets/BulletImpactData.h"
#include "CommonLogCategories.h"

#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
ABulletProjectile::ABulletProjectile()
{
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;

	CollisionComponent->SetSphereRadius(5.0f);
	CollisionComponent->SetCollisionProfileName(FName("Projectile"));
	CollisionComponent->SetCanEverAffectNavigation(false);
	// 이동 중 부딪힌 표면의 Physical Material을 피격 결과에 담는다
	CollisionComponent->bReturnMaterialOnMove = true;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 30000.0f;
	ProjectileMovementComponent->MaxSpeed = 30000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void ABulletProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABulletProjectile, ImpactData, COND_InitialOnly);
	DOREPLIFETIME(ABulletProjectile, ImpactInfo);
}

void ABulletProjectile::InitProjectile(float InDamage, float InInitialSpeed, float LifeTime, UBulletImpactData* InImpactData)
{
	Damage = InDamage;
	ImpactData = InImpactData;
	ProjectileMovementComponent->InitialSpeed = InInitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InInitialSpeed;

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &ThisClass::DestroyProjectile, LifeTime, false);
	}
}

// Called when the game starts or when spawned
void ABulletProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProjectileBeginOverlap);
	}
}

void ABulletProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	HandleImpact(OtherActor, Hit, true);
}

void ABulletProjectile::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (IsActorBeingDestroyed() || ImpactInfo.bHasImpacted) return;

	if (OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

	UGameplayStatics::ApplyPointDamage(
		OtherActor,
		Damage,
		GetActorForwardVector(),
		SweepResult,
		GetInstigatorController(),
		GetOwner(),
		UDamageType::StaticClass()
	);

	COMMON_LOG(LogGameplay, Log, TEXT("Projectile hits something. EventInstigator : %s, DamageCauser : %s"), *GetNameSafe(GetInstigatorController()), *GetNameSafe(GetOwner()));

	HandleImpact(OtherActor, SweepResult, bFromSweep);
}

void ABulletProjectile::DestroyProjectile()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

/*********************************************************************
*                         Bullet Impact 연출
*********************************************************************/
void ABulletProjectile::HandleImpact(AActor* HitActor, const FHitResult& Hit, bool bHasHitLocation)
{
	if (ImpactInfo.bHasImpacted) return;

	// 맞은 자리 정보를 채워 모든 클라이언트로 복제한다
	ImpactInfo.bHasImpacted = true;
	ImpactInfo.bHitCharacter = HitActor && HitActor->IsA<ACharacter>();
	ImpactInfo.Location = bHasHitLocation ? FVector(Hit.ImpactPoint) : GetActorLocation();
	ImpactInfo.Normal = bHasHitLocation ? FVector(Hit.ImpactNormal) : -GetActorForwardVector();
	ImpactInfo.SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

	StopAfterImpact();

	// 서버에는 OnRep이 불리지 않으므로 화면이 있는 서버는 직접 재생한다
	if (GetNetMode() != NM_DedicatedServer)
	{
		PlayImpactEffects();
	}

	// 클라이언트가 맞은 정보를 받을 때까지 숨긴 채 남겨 두었다가 지운다
	GetWorldTimerManager().ClearTimer(LifeTimerHandle);
	SetLifeSpan(ImpactLingerTime);
}

void ABulletProjectile::StopAfterImpact()
{
	ProjectileMovementComponent->StopSimulating(FHitResult());

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

void ABulletProjectile::OnRep_ImpactInfo()
{
	if (!ImpactInfo.bHasImpacted) return;

	SetActorLocation(ImpactInfo.Location);
	StopAfterImpact();
	PlayImpactEffects();
}

void ABulletProjectile::PlayImpactEffects() const
{
	if (!ImpactData) return;

	const FBulletImpactEffect& Effect = ImpactData->GetEffect(ImpactInfo.bHitCharacter, ImpactInfo.SurfaceType);
	const FVector Location = ImpactInfo.Location;
	const FVector Normal = ImpactInfo.Normal;

	// 파티클의 Z축을 표면 바깥 방향에 맞춰 재생한다
	if (Effect.Particle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, Effect.Particle, Location, FRotationMatrix::MakeFromZ(Normal).Rotator());
	}

	// 탄흔은 표면 안쪽으로 투영한다
	if (Effect.DecalMaterial)
	{
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(this, Effect.DecalMaterial, Effect.DecalSize, Location, (-Normal).Rotation());
		if (Decal)
		{
			Decal->SetFadeOut(Effect.DecalLifeSpan, Effect.DecalFadeDuration, false);
			Decal->SetFadeScreenSize(Effect.DecalFadeScreenSize);
		}
	}

	if (Effect.Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Effect.Sound, Location);
	}
}

