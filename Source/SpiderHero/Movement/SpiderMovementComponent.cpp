// Copyright SpiderHero Team. All Rights Reserved.

#include "Movement/SpiderMovementComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "SpiderHero.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

USpiderMovementComponent::USpiderMovementComponent()
{
	WallRunMaxSpeed = 1400.0f;
	WallRunMinSpeed = 450.0f;
	WallRunDeceleration = 120.0f;
	WallRunGravityScale = 0.15f;
	WallRunMaxDuration = 3.0f;
	WallJumpUpImpulse = 650.0f;
	WallJumpOffImpulse = 750.0f;
	WallJumpForwardImpulse = 950.0f;
	WallTraceDistance = 110.0f;

	SlideInitialImpulse = 1100.0f;
	SlideFriction = 0.25f;
	SlideMinSpeedToEnd = 280.0f;

	DiveBombGravityMultiplier = 3.5f;
	DiveBombMaxSpeed = 3200.0f;
	DiveBombTerminalVelocity = 3500.0f;

	AerialApexGravityMultiplier = 0.65f;
	ApexVelocityThreshold = 250.0f;

	CurrentWallRunSide = ESpiderWallRunSide::None;
	CurrentWallRunTime = 0.0f;
	WallRunDirection = FVector::ZeroVector;
	bIsSliding = false;
	bIsDiveBombing = false;
}

void USpiderMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	SpiderCharacterOwner = Cast<ASpiderHeroCharacter>(GetCharacterOwner());
}

void USpiderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Floatier apex during standard falling for comic-book heroic feel
	if (MovementMode == MOVE_Falling && !bIsDiveBombing)
	{
		if (FMath::Abs(Velocity.Z) < ApexVelocityThreshold)
		{
			GravityScale = 1.6f * AerialApexGravityMultiplier;
		}
		else
		{
			GravityScale = 1.6f;
		}
	}
}

float USpiderMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Custom:
		switch (static_cast<ESpiderCustomMovementMode>(CustomMovementMode))
		{
		case ESpiderCustomMovementMode::WallRunning:
			return WallRunMaxSpeed;
		case ESpiderCustomMovementMode::WebSwinging:
			return 2800.0f;
		case ESpiderCustomMovementMode::DiveBomb:
			return DiveBombMaxSpeed;
		case ESpiderCustomMovementMode::Ziplining:
			return 3000.0f;
		case ESpiderCustomMovementMode::Parkour:
			return 900.0f;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return Super::GetMaxSpeed();
}

float USpiderMovementComponent::GetMaxAcceleration() const
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ESpiderCustomMovementMode::WebSwinging))
	{
		return 4000.0f;
	}
	return Super::GetMaxAcceleration();
}

void USpiderMovementComponent::SetSpiderCustomMovementMode(ESpiderCustomMovementMode NewMode, bool bPreserveMomentum)
{
	ESpiderCustomMovementMode PreviousMode = GetSpiderCustomMovementMode();
	FVector CachedVelocity = Velocity;

	if (NewMode == ESpiderCustomMovementMode::None)
	{
		SetMovementMode(MOVE_Falling);
	}
	else
	{
		SetMovementMode(MOVE_Custom, static_cast<uint8>(NewMode));
	}

	if (bPreserveMomentum)
	{
		Velocity = CachedVelocity;
	}

	if (SpiderCharacterOwner.IsValid())
	{
		SpiderCharacterOwner->SetSpiderMovementMode(NewMode);
	}

	OnCustomMovementPhysicsChanged.Broadcast(PreviousMode, NewMode);
	UE_LOG(LogSpiderMovement, Log, TEXT("Changed Spider Custom Movement Mode to: %d"), static_cast<int32>(NewMode));
}

ESpiderCustomMovementMode USpiderMovementComponent::GetSpiderCustomMovementMode() const
{
	if (MovementMode == MOVE_Custom)
	{
		return static_cast<ESpiderCustomMovementMode>(CustomMovementMode);
	}
	return ESpiderCustomMovementMode::None;
}

void USpiderMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (static_cast<ESpiderCustomMovementMode>(CustomMovementMode))
	{
	case ESpiderCustomMovementMode::WebSwinging:
		PhysWebSwinging(deltaTime, Iterations);
		break;
	case ESpiderCustomMovementMode::WallRunning:
		PhysWallRunning(deltaTime, Iterations);
		break;
	case ESpiderCustomMovementMode::WallClimbing:
		PhysWallClimbing(deltaTime, Iterations);
		break;
	case ESpiderCustomMovementMode::Parkour:
		PhysParkour(deltaTime, Iterations);
		break;
	case ESpiderCustomMovementMode::Ziplining:
		PhysZiplining(deltaTime, Iterations);
		break;
	case ESpiderCustomMovementMode::Glide:
		PhysGlide(deltaTime, Iterations);
		break;
	case ESpiderCustomMovementMode::DiveBomb:
		PhysDiveBomb(deltaTime, Iterations);
		break;
	default:
		SetMovementMode(MOVE_Falling);
		break;
	}
}

void USpiderMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom && static_cast<ESpiderCustomMovementMode>(PreviousCustomMode) == ESpiderCustomMovementMode::WallRunning)
	{
		CurrentWallRunSide = ESpiderWallRunSide::None;
		OnWallRunStateChanged.Broadcast(false, ESpiderWallRunSide::None);
	}
}

bool USpiderMovementComponent::CheckWallAtDirection(const FVector& Direction, float Distance, FHitResult& OutHit) const
{
	if (!CharacterOwner)
	{
		return false;
	}

	FVector Start = CharacterOwner->GetActorLocation();
	FVector End = Start + (Direction.GetSafeNormal() * Distance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);
	QueryParams.bTraceComplex = false;

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, SPIDER_TRACE_WALL_SURFACE, QueryParams) ||
	       GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, QueryParams);
}

bool USpiderMovementComponent::FindRunnableWall(FHitResult& OutHit, ESpiderWallRunSide& OutSide) const
{
	if (!CharacterOwner)
	{
		return false;
	}

	FVector RightVector = CharacterOwner->GetActorRightVector();
	FVector ForwardVector = CharacterOwner->GetActorForwardVector();

	// Check Right Side
	if (CheckWallAtDirection(RightVector, WallTraceDistance, OutHit))
	{
		OutSide = ESpiderWallRunSide::Right;
		return true;
	}

	// Check Left Side
	if (CheckWallAtDirection(-RightVector, WallTraceDistance, OutHit))
	{
		OutSide = ESpiderWallRunSide::Left;
		return true;
	}

	// Check Forward Vertical Wall
	if (CheckWallAtDirection(ForwardVector, WallTraceDistance * 0.9f, OutHit))
	{
		OutSide = ESpiderWallRunSide::Vertical;
		return true;
	}

	return false;
}

bool USpiderMovementComponent::TryStartWallRun()
{
	if (!IsFalling())
	{
		return false;
	}

	FHitResult WallHit;
	ESpiderWallRunSide DetectedSide;

	if (FindRunnableWall(WallHit, DetectedSide))
	{
		CurrentWallHit = WallHit;
		CurrentWallRunSide = DetectedSide;
		CurrentWallRunTime = 0.0f;

		// Calculate run direction tangent to wall
		FVector WallNormal = WallHit.ImpactNormal;
		if (DetectedSide == ESpiderWallRunSide::Vertical)
		{
			WallRunDirection = FVector::UpVector;
		}
		else if (DetectedSide == ESpiderWallRunSide::Right)
		{
			WallRunDirection = FVector::CrossProduct(WallNormal, FVector::UpVector);
		}
		else
		{
			WallRunDirection = FVector::CrossProduct(FVector::UpVector, WallNormal);
		}

		// Align initial speed
		float CurrentSpeed = FMath::Clamp(Velocity.Size(), WallRunMinSpeed, WallRunMaxSpeed);
		Velocity = WallRunDirection * CurrentSpeed;

		SetSpiderCustomMovementMode(ESpiderCustomMovementMode::WallRunning, false);

		if (SpiderCharacterOwner.IsValid())
		{
			SpiderCharacterOwner->SetTraversalState(ESpiderTraversalState::WallRun);
			SpiderCharacterOwner->SetWallRunSide(CurrentWallRunSide);
		}

		OnWallRunStateChanged.Broadcast(true, CurrentWallRunSide);
		UE_LOG(LogSpiderMovement, Log, TEXT("Started Wall Run on %s side."), 
			DetectedSide == ESpiderWallRunSide::Right ? TEXT("RIGHT") : (DetectedSide == ESpiderWallRunSide::Left ? TEXT("LEFT") : TEXT("VERTICAL")));
		return true;
	}

	return false;
}

void USpiderMovementComponent::StopWallRun(bool bLaunchOffWall)
{
	if (GetSpiderCustomMovementMode() != ESpiderCustomMovementMode::WallRunning)
	{
		return;
	}

	if (bLaunchOffWall)
	{
		PerformWallJump();
	}
	else
	{
		SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, true);
		if (SpiderCharacterOwner.IsValid())
		{
			SpiderCharacterOwner->SetTraversalState(ESpiderTraversalState::None);
			SpiderCharacterOwner->SetWallRunSide(ESpiderWallRunSide::None);
		}
	}
}

void USpiderMovementComponent::PerformWallJump()
{
	FVector WallNormal = CurrentWallHit.ImpactNormal;
	FVector JumpDirection = (WallNormal * WallJumpOffImpulse) + 
	                       (WallRunDirection * WallJumpForwardImpulse) + 
	                       (FVector::UpVector * WallJumpUpImpulse);

	SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, false);
	Velocity = JumpDirection;

	if (SpiderCharacterOwner.IsValid())
	{
		SpiderCharacterOwner->SetTraversalState(ESpiderTraversalState::None);
		SpiderCharacterOwner->SetWallRunSide(ESpiderWallRunSide::None);
	}

	UE_LOG(LogSpiderMovement, Log, TEXT("Wall Jump performed with impulse: %s"), *JumpDirection.ToString());
}

void USpiderMovementComponent::PhysWallRunning(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	CurrentWallRunTime += deltaTime;

	// Check Wall Persistence
	FVector TraceDir = (CurrentWallRunSide == ESpiderWallRunSide::Right) ? CharacterOwner->GetActorRightVector() :
	                  ((CurrentWallRunSide == ESpiderWallRunSide::Left) ? -CharacterOwner->GetActorRightVector() : CharacterOwner->GetActorForwardVector());

	FHitResult WallCheck;
	if (!CheckWallAtDirection(TraceDir, WallTraceDistance * 1.25f, WallCheck) || CurrentWallRunTime >= WallRunMaxDuration)
	{
		StopWallRun(false);
		return;
	}

	CurrentWallHit = WallCheck;
	FVector WallNormal = WallCheck.ImpactNormal;

	// Recalculate Tangent Direction
	if (CurrentWallRunSide == ESpiderWallRunSide::Vertical)
	{
		WallRunDirection = FVector::UpVector;
	}
	else if (CurrentWallRunSide == ESpiderWallRunSide::Right)
	{
		WallRunDirection = FVector::CrossProduct(WallNormal, FVector::UpVector);
	}
	else
	{
		WallRunDirection = FVector::CrossProduct(FVector::UpVector, WallNormal);
	}

	// Update Velocity with Deceleration and Minor Wall Gravity
	float CurrentSpeed = Velocity.Size() - (WallRunDeceleration * deltaTime);
	if (CurrentSpeed < WallRunMinSpeed)
	{
		StopWallRun(false);
		return;
	}

	Velocity = (WallRunDirection * CurrentSpeed) + (FVector::DownVector * (980.0f * WallRunGravityScale * deltaTime));

	// Move Character
	FVector DeltaMove = Velocity * deltaTime;
	FRotator TargetRotation = WallRunDirection.Rotation();

	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, TargetRotation, true, MoveHit);

	if (MoveHit.IsValidBlockingHit())
	{
		SlideAlongSurface(DeltaMove, 1.0f - MoveHit.Time, MoveHit.Normal, MoveHit, true);
	}
}

void USpiderMovementComponent::PhysWebSwinging(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	FVector DeltaMove = Velocity * deltaTime;
	FRotator TargetRotation = Velocity.GetSafeNormal().Rotation();

	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, TargetRotation, true, MoveHit);

	if (MoveHit.IsValidBlockingHit())
	{
		// Wall hit during swing -> opportunity to wall run or slide along surface
		SlideAlongSurface(DeltaMove, 1.0f - MoveHit.Time, MoveHit.Normal, MoveHit, true);
		TryStartWallRun();
	}
}

void USpiderMovementComponent::PhysWallClimbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	FVector DeltaMove = Velocity * deltaTime;
	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, CharacterOwner->GetActorRotation(), true, MoveHit);
}

void USpiderMovementComponent::PhysParkour(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	FVector DeltaMove = Velocity * deltaTime;
	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, CharacterOwner->GetActorRotation(), true, MoveHit);
}

void USpiderMovementComponent::PhysZiplining(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	FVector DeltaMove = Velocity * deltaTime;
	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, Velocity.GetSafeNormal().Rotation(), true, MoveHit);
}

void USpiderMovementComponent::PhysGlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// Constant forward velocity with minimal descent
	FVector ForwardDir = CharacterOwner->GetActorForwardVector();
	Velocity = (ForwardDir * 1600.0f) + (FVector::DownVector * 200.0f);

	FVector DeltaMove = Velocity * deltaTime;
	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, CharacterOwner->GetActorRotation(), true, MoveHit);
}

void USpiderMovementComponent::PhysDiveBomb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// Rapid downward acceleration
	float DownwardAcc = 980.0f * DiveBombGravityMultiplier * deltaTime;
	Velocity.Z -= DownwardAcc;
	Velocity.Z = FMath::Clamp(Velocity.Z, -DiveBombTerminalVelocity, 0.0f);

	FVector DeltaMove = Velocity * deltaTime;
	FRotator DownRot = FRotator(-85.0f, CharacterOwner->GetActorRotation().Yaw, 0.0f);

	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, DownRot, true, MoveHit);

	if (MoveHit.IsValidBlockingHit())
	{
		StopDiveBomb();
	}
}

void USpiderMovementComponent::ApplySwingVelocity(const FVector& NewVelocity)
{
	Velocity = NewVelocity;
}

bool USpiderMovementComponent::TryStartSlide()
{
	if (!IsMovingOnGround() || Velocity.Size() < 500.0f)
	{
		return false;
	}

	bIsSliding = true;
	FVector SlideDir = Velocity.GetSafeNormal();
	Velocity = SlideDir * (Velocity.Size() + SlideInitialImpulse * 0.5f);

	if (CharacterOwner)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
	}

	return true;
}

void USpiderMovementComponent::StopSlide()
{
	bIsSliding = false;
	if (CharacterOwner)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	}
}

void USpiderMovementComponent::StartDiveBomb()
{
	bIsDiveBombing = true;
	SetSpiderCustomMovementMode(ESpiderCustomMovementMode::DiveBomb, false);
}

void USpiderMovementComponent::StopDiveBomb()
{
	bIsDiveBombing = false;
	SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, true);
}

void USpiderMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// Decelerate with custom slide friction
	float CurrentSpeed = Velocity.Size() - (SlideFriction * 1000.0f * deltaTime);
	if (CurrentSpeed < SlideMinSpeedToEnd)
	{
		StopSlide();
		SetMovementMode(MOVE_Walking);
		return;
	}

	Velocity = Velocity.GetSafeNormal() * CurrentSpeed;
	FVector DeltaMove = Velocity * deltaTime;

	FHitResult MoveHit;
	SafeMoveUpdatedComponent(DeltaMove, CharacterOwner->GetActorRotation(), true, MoveHit);
}
