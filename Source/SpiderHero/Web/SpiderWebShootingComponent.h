// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderWebShootingComponent.generated.h"

class ASpiderHeroCharacter;
class ASpiderWebProjectile;
class UNiagaraSystem;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebFiredSignature, const FVector&, MuzzleLocation, const FVector&, Direction, bool, bIsLeftHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebYankExecutedSignature, AActor*, YankedActor, const FVector&, PullDirection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebAmmoChangedSignature, int32, CurrentShots, int32, MaxShots);

/**
 * USpiderWebShootingComponent
 * Handles web projectile firing, wrist-socket alternating, web fluid ammunition,
 * enemy yanking, and rapid environmental tethering.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderWebShootingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderWebShootingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Fires a rapid web shot projectile from alternating wrist shooters */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Shooting")
	bool FireWebShot();

	/** Yanks an enemy or physics object towards player */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Shooting")
	bool PerformWebYank(AActor* TargetActor);

	/** Tethers two targets together with a tensile web line */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Shooting")
	bool FireWebTether(AActor* TargetActor, const FVector& AnchorLocation);

	/** Replenishes web shooter ammunition */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Shooting")
	void ReplenishWebShots(int32 Amount);

	/** Get current web ammo */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Shooting")
	int32 GetCurrentWebShots() const { return CurrentWebShots; }

	/** Get max web ammo */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Shooting")
	int32 GetMaxWebShots() const { return MaxWebShots; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Shooting|Events")
	FOnWebFiredSignature OnWebFired;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Shooting|Events")
	FOnWebYankExecutedSignature OnWebYankExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Shooting|Events")
	FOnWebAmmoChangedSignature OnWebAmmoChanged;

protected:
	// Projectile Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Projectile")
	TSubclassOf<ASpiderWebProjectile> WebProjectileClass;

	// Ammo & Cooldown
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Ammo")
	int32 MaxWebShots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Shooting|Ammo")
	int32 CurrentWebShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Ammo")
	float WebReloadRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Ammo")
	float FireRateLimit;

	// Wrist Sockets
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Sockets")
	FName LeftWristSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Sockets")
	FName RightWristSocket;

	// Yank Forces
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Yank")
	float WebYankForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Shooting|Yank")
	float WebYankUpwardLift;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Shooting|State")
	bool bNextShotIsLeftHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Shooting|State")
	float LastFireTime;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;

	FVector GetMuzzleLocation(bool bLeftHand) const;
	FVector GetAimDirection() const;
};
