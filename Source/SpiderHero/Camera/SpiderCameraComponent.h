// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderCameraComponent.generated.h"

class USpringArmComponent;
class ASpiderHeroCharacter;

/**
 * Camera preset modes for context-aware superhero camera dynamics
 */
UENUM(BlueprintType)
enum class ESpiderCameraState : uint8
{
	Default     = 0 UMETA(DisplayName = "Default / Exploration"),
	Sprint      = 1 UMETA(DisplayName = "Sprinting"),
	Swing       = 2 UMETA(DisplayName = "High-Speed Swing"),
	Dive        = 3 UMETA(DisplayName = "Dive Bomb"),
	WallRun     = 4 UMETA(DisplayName = "Wall Running"),
	Combat      = 5 UMETA(DisplayName = "Melee Combat"),
	CombatLock  = 6 UMETA(DisplayName = "Combat Lock-On"),
	Aim         = 7 UMETA(DisplayName = "Web Aiming"),
	Finisher    = 8 UMETA(DisplayName = "Cinematic Finisher")
};

/**
 * Impact impulse types for procedural superhero camera reactions
 */
UENUM(BlueprintType)
enum class ESpiderCameraImpactType : uint8
{
	LightHit        = 0 UMETA(DisplayName = "Light Strike"),
	HeavyImpact     = 1 UMETA(DisplayName = "Heavy Attack Impact"),
	HardLanding     = 2 UMETA(DisplayName = "Hard Landing Shockwave"),
	Explosion       = 3 UMETA(DisplayName = "Web / Radial Explosion"),
	DodgeWhoosh     = 4 UMETA(DisplayName = "Perfect Dodge Reaction")
};

/**
 * USpiderCameraComponent
 * Superhero camera component providing:
 * - Dynamic FOV transitions (90° Base, 98° Sprint, 110° Swing, 115° Dive, 82° Combat)
 * - Dynamic boom arm extension & lag modulation
 * - Wall run roll tilts & Swing centrifugal G-force banking
 * - Procedural trauma-based impact shakes & landing impulses
 * - Geometry collision avoidance damping
 */
UCLASS(ClassGroup = (SpiderCamera), meta = (BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	USpiderCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Sets the current camera state preset */
	UFUNCTION(BlueprintCallable, Category = "Spider|Camera")
	void SetCameraState(ESpiderCameraState NewState);

	/** Gets current camera state preset */
	UFUNCTION(BlueprintPure, Category = "Spider|Camera")
	ESpiderCameraState GetCameraState() const { return CurrentCameraState; }

	/** Apply procedural trauma to the camera (0.0 to 1.0 range, decays over time) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Camera|Impulse")
	void AddCameraTrauma(float TraumaAmount);

	/** Trigger a contextual camera impulse (impacts, landings, heavy hits) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Camera|Impulse")
	void ApplyCameraImpact(ESpiderCameraImpactType ImpactType, const FVector& ImpactDirection = FVector::ZeroVector, float Intensity = 1.0f);

	/** Set or clear lock-on target actor for framing */
	UFUNCTION(BlueprintCallable, Category = "Spider|Camera|LockOn")
	void SetLockOnTarget(AActor* NewTarget);

	/** Clear lock-on target */
	UFUNCTION(BlueprintCallable, Category = "Spider|Camera|LockOn")
	void ClearLockOnTarget();

	/** Returns true if currently framing a locked-on target */
	UFUNCTION(BlueprintPure, Category = "Spider|Camera|LockOn")
	bool HasLockOnTarget() const { return LockOnTarget.IsValid(); }

	/** Explicitly inject roll tilt angle (e.g. from wall runs or swing arcs) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Camera|Tilt")
	void SetDynamicRollTilt(float TargetRollDegrees) { DesiredRollTilt = TargetRollDegrees; }

protected:
	// Dynamic FOV Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float BaseFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float SprintFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float SwingFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float DiveFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float WallRunFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float CombatFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float CombatLockFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float AimFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float FOVInterpSpeedDefault;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|FOV")
	float FOVInterpSpeedFast;

	// Boom Arm Length Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Boom")
	float BaseArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Boom")
	float SprintArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Boom")
	float SwingArmLengthMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Boom")
	float DiveArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Boom")
	float CombatArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Boom")
	float ArmInterpSpeed;

	// Camera Lag Modulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Lag")
	float DefaultLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Lag")
	float SwingLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Lag")
	float CombatLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Lag")
	float DefaultRotLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Lag")
	float SwingRotLagSpeed;

	// Tilt & Bank Angles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Tilt")
	float WallRunTiltAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Tilt")
	float MaxSwingCentrifugalTilt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Tilt")
	float TiltInterpSpeed;

	// Trauma Shake Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Trauma")
	float TraumaDecaySpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Trauma")
	float MaxShakePitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Trauma")
	float MaxShakeYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Trauma")
	float MaxShakeRoll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Trauma")
	float MaxShakeOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Trauma")
	float ShakeFrequency;

	// Collision Avoidance Damping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Collision")
	float CollisionAvoidanceProbeRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Collision")
	float MinSafeDistanceToGeometry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Camera|Collision")
	float CollisionDampingSpeed;

private:
	// State Tracking
	UPROPERTY()
	ESpiderCameraState CurrentCameraState;

	UPROPERTY()
	TWeakObjectPtr<USpringArmComponent> AttachedBoom;

	UPROPERTY()
	TWeakObjectPtr<ASpiderHeroCharacter> OwnerCharacter;

	UPROPERTY()
	TWeakObjectPtr<AActor> LockOnTarget;

	float CurrentTrauma;
	float TraumaTimeElapsed;
	float CurrentRollTilt;
	float DesiredRollTilt;
	FVector ProceduralOffset;
	FRotator ProceduralRotationOffset;

	// Internal Update Routines
	void UpdateAutoStateFromCharacter();
	void UpdateFOV(float DeltaTime);
	void UpdateBoomArm(float DeltaTime);
	void UpdateTiltAndRoll(float DeltaTime);
	void UpdateTraumaShake(float DeltaTime);
	void UpdateLockOnFraming(float DeltaTime);
	void UpdateCollisionDamping(float DeltaTime);
	void UpdateCollisionAvoidanceGeometry(class UWorld* World, const FVector& CameraLoc, const FVector& PivotLoc);
	
	float GetTargetFOVForState(ESpiderCameraState State) const;
	float GetTargetArmLengthForState(ESpiderCameraState State, float SpeedRatio) const;
};
