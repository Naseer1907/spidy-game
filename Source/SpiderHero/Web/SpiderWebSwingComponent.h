// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderWebSwingComponent.generated.h"

class ASpiderHeroCharacter;
class USpiderMovementComponent;
class USpiderWebTargetingComponent;
class USpiderStaminaComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwingAttachedSignature, const FSpiderWebAnchorInfo&, Anchor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSwingReleasedSignature, const FVector&, LaunchVelocity, float, LaunchBoost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwingBoostSignature, float, BoostAmount);

/**
 * USpiderWebSwingComponent
 * Physics-driven pendulum web swing simulation engine.
 * Computes rope constraint tension, centripetal acceleration, steering torques,
 * dynamic cable contraction, and apex momentum launch boosts.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderWebSwingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderWebSwingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Attach web line to best available anchor and begin swinging */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Swing")
	bool StartWebSwing();

	/** Release web line, applying launch boost based on swing arc */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Swing")
	void ReleaseWebSwing(bool bApplyLaunchBoost = true);

	/** Applies a high-speed swing boost consuming stamina */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Swing")
	bool TriggerSwingBoost();

	/** Check if currently swinging */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Swing")
	bool IsSwinging() const { return bIsSwinging; }

	/** Get active swing anchor */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Swing")
	const FSpiderWebAnchorInfo& GetActiveAnchor() const { return CurrentAnchor; }

	/** Get current rope length */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Swing")
	float GetCurrentRopeLength() const { return CurrentRopeLength; }

	/** Apply directional steering torque to swing (e.g. from input stick) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Swing")
	void ApplySteeringInput(const FVector2D& SteeringInput);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Swing|Events")
	FOnSwingAttachedSignature OnSwingAttached;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Swing|Events")
	FOnSwingReleasedSignature OnSwingReleased;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Swing|Events")
	FOnSwingBoostSignature OnSwingBoost;

protected:
	// Pendulum Physics Tuning
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Physics")
	float GravityMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Physics")
	float CentripetalForceMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Physics")
	float SteeringTorqueStrength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Physics")
	float MaxSwingSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Physics")
	float MinSwingSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Physics")
	float AirFrictionDamping;

	// Cable Shortening / Reel
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Cable")
	float CableReelSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Cable")
	float MinCableLength;

	// Launch Boosts
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Boost")
	float ApexBoostMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Boost")
	float UpwardLaunchKicker;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Boost")
	float ManualBoostImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Swing|Boost")
	float BoostStaminaCost;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Swing|State")
	bool bIsSwinging;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Swing|State")
	FSpiderWebAnchorInfo CurrentAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Swing|State")
	float CurrentRopeLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Swing|State")
	FVector CurrentSwingVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Swing|State")
	FVector2D CachedSteeringInput;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;
	TWeakObjectPtr<USpiderMovementComponent> MovementComponent;
	TWeakObjectPtr<USpiderWebTargetingComponent> TargetingComponent;
	TWeakObjectPtr<USpiderStaminaComponent> StaminaComponent;

	void SimulatePendulumPhysics(float DeltaTime);
	FVector CalculateTensionForce(const FVector& PlayerLoc, const FVector& Velocity, float RopeLength) const;
};
