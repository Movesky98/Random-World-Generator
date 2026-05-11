// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlay/Actors/BulletProjectile.h"
#include "CommonLogCategories.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABulletProjectile::ABulletProjectile()
{
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;

	CollisionComponent->SetSphereRadius(5.0f);
	// Set CollisionComponent's collision profile.
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 30000.0f;
	ProjectileMovementComponent->MaxSpeed = 30000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void ABulletProjectile::InitProjectile(AActor* InOwnerActor, float InDamage, float InInitialSpeed, float LifeTime)
{
	SetOwner(InOwnerActor);

	Damage = InDamage;
	ProjectileMovementComponent->InitialSpeed = InInitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InInitialSpeed;
	ProjectileMovementComponent->Velocity = GetActorForwardVector() * InInitialSpeed;

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
	}
}

void ABulletProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	if (OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);

	COMMON_LOG(LogGameplay, Log, TEXT("Projectile hits %s"), *OtherActor->GetName());

	DestroyProjectile();
}

void ABulletProjectile::DestroyProjectile()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

