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
	CollisionComponent->SetCollisionProfileName(FName("Projectile"));
	CollisionComponent->SetCanEverAffectNavigation(false);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 30000.0f;
	ProjectileMovementComponent->MaxSpeed = 30000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void ABulletProjectile::InitProjectile(float InDamage, float InInitialSpeed, float LifeTime)
{
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

	UGameplayStatics::ApplyPointDamage(
		OtherActor,
		Damage,
		GetActorForwardVector(),
		Hit,
		GetInstigatorController(),
		GetOwner(),
		UDamageType::StaticClass()
	);

	COMMON_LOG(LogGameplay, Log, TEXT("Projectile hits something. EventInstigator : %s, DamageCauser : %s"), *GetNameSafe(GetInstigatorController()), *GetNameSafe(GetOwner()));

	DestroyProjectile();
}

void ABulletProjectile::DestroyProjectile()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

