// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderHeroAnimInstance.generated.h"

class ASpiderHeroCharacter;
class UCharacterMovementComponent;

/**
 * USpiderHeroAnimInstance
 * Animation instance driving locomotion blendspaces, web-swing procedural leaning,
 * wall-running, parkour transitions, and combat montages.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API USpiderHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	USpiderHeroAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// Owner References
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|References")
	TWeakObjectPtr<ASpiderHeroCharacter> SpiderCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|References")
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	// Locomotion
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	float AirSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	float MovementDirectionYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	bool bIsAccelerating;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	bool bIsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	bool bIsOnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	bool bIsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Locomotion")
	bool bIsCrouching;

	// Web-Swinging
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Web")
	bool bIsSwinging;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Web")
	ESpiderSwingState SwingState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Web")
	float SwingSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Web")
	float SwingLeanAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Web")
	float SwingPitchAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Web")
	bool bIsLeftHandSwing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Web")
	FVector AnchorLocation;

	// Traversal & Parkour
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	bool bIsWallRunning;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	ESpiderWallRunSide WallRunSide;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	float WallRunPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	bool bIsWallClimbing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	FVector2D WallClimbDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	ESpiderTraversalState TraversalState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	bool bIsParkourVaulting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Traversal")
	bool bIsZiplining;

	// Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Combat")
	ESpiderCombatState CombatState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Combat")
	bool bIsInCombat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Anim|Combat")
	ESpiderSenseAlertLevel SpiderSenseAlertLevel;

private:
	FRotator PreviousRotation;
};
