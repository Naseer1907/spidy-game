// Copyright SpiderHero Team. All Rights Reserved.

#include "Web/SpiderWebProjectile.h"
#include "SpiderHero.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

ASpiderWebProjectile::ASpiderWebProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Sphere Collision
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(12.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->OnComponentHit.AddDynamic(this, &ASpiderWebProjectile::OnHit);
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.0f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	RootComponent = CollisionComp;

	// Visual Mesh
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Projectile Movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 4500.0f;
	ProjectileMovement->MaxSpeed = 4500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.15f;

	// Properties
	BaseDamage = 15.0f;
	WebStunDuration = 3.0f;
	ImpactForce = 600.0f;
	bTethersTargetToWall = true;
	LifeSpanDuration = 4.0f;
}

void ASpiderWebProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpanDuration);
}

void ASpiderWebProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	bool bIsEnemy = (OtherActor->GetClass()->GetName().Contains(TEXT("Enemy")) || OtherActor->ActorHasTag(TEXT("Enemy")));

	// Apply Damage
	if (BaseDamage > 0.0f)
	{
		UGameplayStatics::ApplyPointDamage(
			OtherActor,
			BaseDamage,
			-Hit.ImpactNormal,
			Hit,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);
	}

	// Apply Physics Impulse if component simulates physics
	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity().GetSafeNormal() * ImpactForce, Hit.ImpactPoint);
	}

	OnWebProjectileHit.Broadcast(OtherActor, Hit, bIsEnemy);
	UE_LOG(LogSpiderCombat, Log, TEXT("Web Projectile impacted %s at %s"), *OtherActor->GetName(), *Hit.ImpactPoint.ToString());

	Destroy();
}
