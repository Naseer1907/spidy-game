// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderMovementComponent.generated.h"

class ASpiderHeroCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomMovementPhysicsChangedSignature, ESpiderCustomMovementMode, PreviousMode, ESpiderCustomMovementMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWallRunStateChangedSignature, bool, bIsWallRunning, ESpiderWallRunSide, WallRunSide);

/**
 * USpiderMovementComponent
 * Advanced custom character movement component implementing superhero physics:
 * Web Swinging, Wall Running, Wall Climbing, Parkour, Dive Bomb, and Slide.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USpiderMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Custom Physics Dispatcher
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;

	/** Switches to a custom Spider movement mode with optional momentum preservation */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement")
	void SetSpiderCustomMovementMode(ESpiderCustomMovementMode NewMode, bool bPreserveMomentum = true);

	/** Returns current custom movement mode */
	UFUNCTION(BlueprintPure, Category = "Spider|Movement")
	ESpiderCustomMovementMode GetSpiderCustomMovementMode() const;

	/** Initiates a Wall Run attempt against detected geometry */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement|WallRun")
	bool TryStartWallRun();

	/** Stops wall running and returns to falling or default locomotion */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement|WallRun")
	void StopWallRun(bool bLaunchOffWall = false);

	/** Performs a directional wall jump away from current wall */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement|WallRun")
	void PerformWallJump();

	/** Initiates a ground slide */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement|Slide")
	bool TryStartSlide();

	/** Stops ground slide */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement|Slide")
	void StopSlide();

	/** Initiates high-speed dive bomb */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement|DiveBomb")
	void StartDiveBomb();

	/** Ends dive bomb */
	UFUNCTION(BlueprintCallable, Category = "Spider|Movement|DiveBomb")
	void StopDiveBomb();

	/** External physics injection for swinging component */
	void ApplySwingVelocity(const FVector& NewVelocity);

	/** Get current wall hit info */
	UFUNCTION(BlueprintPure, Category = "Spider|Movement|WallRun")
	const FHitResult& GetWallHitResult() const { return CurrentWallHit; }

	/** Get current wall run side */
	UFUNCTION(BlueprintPure, Category = "Spider|Movement|WallRun")
	ESpiderWallRunSide GetCurrentWallRunSide() const { return CurrentWallRunSide; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Movement|Events")
	FOnCustomMovementPhysicsChangedSignature OnCustomMovementPhysicsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Movement|Events")
	FOnWallRunStateChangedSignature OnWallRunStateChanged;

protected:
	// Mode Specific Physics Implementations
	virtual void PhysWebSwinging(float deltaTime, int32 Iterations);
	virtual void PhysWallRunning(float deltaTime, int32 Iterations);
	virtual void PhysWallClimbing(float deltaTime, int32 Iterations);
	virtual void PhysParkour(float deltaTime, int32 Iterations);
	virtual void PhysZiplining(float deltaTime, int32 Iterations);
	virtual void PhysSlide(float deltaTime, int32 Iterations);
	virtual void PhysDiveBomb(float deltaTime, int32 Iterations);
	virtual void PhysGlide(float deltaTime, int32 Iterations);

	// Wall Running Helpers
	bool CheckWallAtDirection(const FVector& Direction, float Distance, FHitResult& OutHit) const;
	bool FindRunnableWall(FHitResult& OutHit, ESpiderWallRunSide& OutSide) const;

	// Configuration - Wall Running
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallRunMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallRunMinSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallRunDeceleration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallRunGravityScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallRunMaxDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallJumpUpImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallJumpOffImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallJumpForwardImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|WallRun")
	float WallTraceDistance;

	// Configuration - Slide
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Slide")
	float SlideInitialImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Slide")
	float SlideFriction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Slide")
	float SlideMinSpeedToEnd;

	// Configuration - Dive Bomb
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|DiveBomb")
	float DiveBombGravityMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|DiveBomb")
	float DiveBombMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|DiveBomb")
	float DiveBombTerminalVelocity;

	// Configuration - Super Hero Aerial Feel
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Aerial")
	float AerialApexGravityMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Aerial")
	float ApexVelocityThreshold;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Movement|State")
	ESpiderWallRunSide CurrentWallRunSide;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Movement|State")
	FHitResult CurrentWallHit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Movement|State")
	float CurrentWallRunTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Movement|State")
	FVector WallRunDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Movement|State")
	bool bIsSliding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Movement|State")
	bool bIsDiveBombing;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> SpiderCharacterOwner;
};
