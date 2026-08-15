// Copyright SpiderHero Team. All Rights Reserved.

#include "Character/SpiderHeroAnimInstance.h"
#include "Character/SpiderHeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

USpiderHeroAnimInstance::USpiderHeroAnimInstance()
{
	GroundSpeed = 0.0f;
	AirSpeed = 0.0f;
	Velocity = FVector::ZeroVector;
	MovementDirectionYaw = 0.0f;
	bIsAccelerating = false;
	bIsFalling = false;
	bIsOnGround = true;
	bIsSprinting = false;
	bIsCrouching = false;

	bIsSwinging = false;
	SwingState = ESpiderSwingState::None;
	SwingSpeed = 0.0f;
	SwingLeanAngle = 0.0f;
	SwingPitchAngle = 0.0f;
	bIsLeftHandSwing = false;
	AnchorLocation = FVector::ZeroVector;

	bIsWallRunning = false;
	WallRunSide = ESpiderWallRunSide::None;
	WallRunPitch = 0.0f;
	bIsWallClimbing = false;
	WallClimbDirection = FVector2D::ZeroVector;
	TraversalState = ESpiderTraversalState::None;
	bIsParkourVaulting = false;
	bIsZiplining = false;

	CombatState = ESpiderCombatState::Neutral;
	bIsInCombat = false;
	SpiderSenseAlertLevel = ESpiderSenseAlertLevel::None;
	PreviousRotation = FRotator::ZeroRotator;
}

void USpiderHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* OwnerPawn = TryGetPawnOwner();
	if (OwnerPawn)
	{
		SpiderCharacter = Cast<ASpiderHeroCharacter>(OwnerPawn);
		MovementComponent = OwnerPawn->FindComponentByClass<UCharacterMovementComponent>();
		PreviousRotation = OwnerPawn->GetActorRotation();
	}
}

void USpiderHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (DeltaSeconds <= 0.0f)
	{
		return;
	}

	if (!SpiderCharacter.IsValid())
	{
		APawn* OwnerPawn = TryGetPawnOwner();
		if (OwnerPawn)
		{
			SpiderCharacter = Cast<ASpiderHeroCharacter>(OwnerPawn);
			MovementComponent = OwnerPawn->FindComponentByClass<UCharacterMovementComponent>();
			PreviousRotation = OwnerPawn->GetActorRotation();
		}
		if (!SpiderCharacter.IsValid())
		{
			return;
		}
	}

	ASpiderHeroCharacter* Character = SpiderCharacter.Get();

	// Locomotion Updates
	Velocity = Character->GetVelocity();
	FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
	GroundSpeed = HorizontalVelocity.Size();
	AirSpeed = Velocity.Size();

	if (MovementComponent.IsValid())
	{
		bIsFalling = MovementComponent->IsFalling();
		bIsOnGround = MovementComponent->IsMovingOnGround();
		bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
		bIsCrouching = MovementComponent->IsCrouching();
	}
	else
	{
		bIsFalling = false;
		bIsOnGround = true;
		bIsAccelerating = false;
		bIsCrouching = false;
	}

	bIsSprinting = Character->IsSprinting();

	// Calculate Movement Direction Yaw (-180 to 180) relative to actor rotation
	if (GroundSpeed > 5.0f)
	{
		FRotator ActorRotation = Character->GetActorRotation();
		FRotator VelocityRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
		MovementDirectionYaw = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation).Yaw;
	}
	else
	{
		MovementDirectionYaw = 0.0f;
	}

	// State Updates from Character
	TraversalState = Character->GetTraversalState();
	bIsWallRunning = (TraversalState == ESpiderTraversalState::WallRun);
	WallRunSide = Character->GetWallRunSide();
	bIsWallClimbing = (TraversalState == ESpiderTraversalState::WallClimb);
	bIsParkourVaulting = (TraversalState == ESpiderTraversalState::VaultLow || TraversalState == ESpiderTraversalState::VaultHigh || TraversalState == ESpiderTraversalState::Mantle);
	bIsZiplining = (Character->GetSpiderMovementMode() == ESpiderCustomMovementMode::Ziplining);

	// Web-Swinging Updates
	SwingState = Character->GetSwingState();
	bIsSwinging = (SwingState == ESpiderSwingState::Swinging || SwingState == ESpiderSwingState::Attaching);
	SwingSpeed = Velocity.Size();

	const FSpiderWebAnchorInfo& Anchor = Character->GetActiveAnchorInfo();
	bIsLeftHandSwing = Anchor.bIsLeftHand;
	AnchorLocation = Anchor.AnchorPoint;

	// Calculate Swing Leaning and Pitch
	if (bIsSwinging)
	{
		FRotator CurrentRotation = Character->GetActorRotation();
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, PreviousRotation);
		float TurnYawRate = DeltaRot.Yaw / DeltaSeconds;
		
		// Procedural Roll lean based on turning speed during swing
		float TargetLean = FMath::Clamp(TurnYawRate * 0.35f, -45.0f, 45.0f);
		SwingLeanAngle = FMath::FInterpTo(SwingLeanAngle, TargetLean, DeltaSeconds, 8.0f);

		// Procedural Pitch based on vertical velocity
		float TargetPitch = FMath::Clamp(Velocity.Z / 30.0f, -40.0f, 50.0f);
		SwingPitchAngle = FMath::FInterpTo(SwingPitchAngle, TargetPitch, DeltaSeconds, 6.0f);

		PreviousRotation = CurrentRotation;
	}
	else
	{
		SwingLeanAngle = FMath::FInterpTo(SwingLeanAngle, 0.0f, DeltaSeconds, 10.0f);
		SwingPitchAngle = FMath::FInterpTo(SwingPitchAngle, 0.0f, DeltaSeconds, 10.0f);
		PreviousRotation = Character->GetActorRotation();
	}

	// Combat Updates
	CombatState = Character->GetCombatState();
	bIsInCombat = (CombatState != ESpiderCombatState::Neutral);
}
