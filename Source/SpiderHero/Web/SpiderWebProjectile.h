// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpiderWebProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebProjectileHitSignature, AActor*, HitActor, const FHitResult&, HitInfo, bool, bIsEnemy);

/**
 * ASpiderWebProjectile
 * Ballistic web projectile fired from web-shooters for combat stuns, enemy immobilization, and environmental tethering.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderWebProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpiderWebProjectile();

	virtual void BeginPlay() override;

	/** Called when projectile impacts a blocking surface or character */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Getters
	FORCEINLINE USphereComponent* GetCollisionComponent() const { return CollisionComp; }
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Projectile|Events")
	FOnWebProjectileHitSignature OnWebProjectileHit;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// Web Properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Projectile|Config")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Projectile|Config")
	float WebStunDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Projectile|Config")
	float ImpactForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Projectile|Config")
	bool bTethersTargetToWall;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Projectile|Config")
	float LifeSpanDuration;
};
