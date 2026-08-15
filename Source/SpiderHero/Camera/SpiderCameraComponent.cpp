// Copyright SpiderHero Team. All Rights Reserved.

#include "Camera/SpiderCameraComponent.h"
#include "SpiderHero.h"
#include "Character/SpiderHeroCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

USpiderCameraComponent::USpiderCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;

	// Dynamic FOV Defaults
	BaseFOV = 90.0f;
	SprintFOV = 98.0f;
	SwingFOV = 110.0f;
	DiveFOV = 115.0f;
	WallRunFOV = 95.0f;
	CombatFOV = 85.0f;
	CombatLockFOV = 82.0f;
	AimFOV = 75.0f;

	FOVInterpSpeedDefault = 4.5f;
	FOVInterpSpeedFast = 10.0f;

	// Boom Arm Length Defaults
	BaseArmLength = 400.0f;
	SprintArmLength = 470.0f;
	SwingArmLengthMax = 620.0f;
	DiveArmLength = 520.0f;
	CombatArmLength = 320.0f;
	ArmInterpSpeed = 4.0f;

	// Lag Defaults
	DefaultLagSpeed = 12.0f;
	SwingLagSpeed = 6.0f;
	CombatLagSpeed = 16.0f;
	DefaultRotLagSpeed = 15.0f;
	SwingRotLagSpeed = 8.0f;

	// Tilts
	WallRunTiltAngle = 14.0f;
	MaxSwingCentrifugalTilt = 12.0f;
	TiltInterpSpeed = 5.0f;

	// Trauma
	TraumaDecaySpeed = 1.4f;
	MaxShakePitch = 4.0f;
	MaxShakeYaw = 4.0f;
	MaxShakeRoll = 6.0f;
	MaxShakeOffset = 18.0f;
	ShakeFrequency = 25.0f;

	// Collision
	CollisionAvoidanceProbeRadius = 24.0f;
	MinSafeDistanceToGeometry = 40.0f;
	CollisionDampingSpeed = 8.0f;

	// Internal
	CurrentCameraState = ESpiderCameraState::Default;
	CurrentTrauma = 0.0f;
	TraumaTimeElapsed = 0.0f;
	CurrentRollTilt = 0.0f;
	DesiredRollTilt = 0.0f;
	ProceduralOffset = FVector::ZeroVector;
	ProceduralRotationOffset = FRotator::ZeroRotator;
}

void USpiderCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		OwnerCharacter = Cast<ASpiderHeroCharacter>(OwnerActor);
		AttachedBoom = OwnerActor->FindComponentByClass<USpringArmComponent>();
	}

	SetFieldOfView(BaseFOV);
	UE_LOG(LogSpiderCamera, Log, TEXT("SpiderCameraComponent initialized on %s"), OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"));
}

void USpiderCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAutoStateFromCharacter();
	UpdateFOV(DeltaTime);
	UpdateBoomArm(DeltaTime);
	UpdateTiltAndRoll(DeltaTime);
	UpdateTraumaShake(DeltaTime);
	UpdateLockOnFraming(DeltaTime);
	UpdateCollisionDamping(DeltaTime);
}

void USpiderCameraComponent::SetCameraState(ESpiderCameraState NewState)
{
	if (CurrentCameraState != NewState)
	{
		CurrentCameraState = NewState;
		UE_LOG(LogSpiderCamera, Verbose, TEXT("Camera state transitioned to: %d"), static_cast<int32>(CurrentCameraState));
	}
}

void USpiderCameraComponent::AddCameraTrauma(float TraumaAmount)
{
	CurrentTrauma = FMath::Clamp(CurrentTrauma + TraumaAmount, 0.0f, 1.0f);
}

void USpiderCameraComponent::ApplyCameraImpact(ESpiderCameraImpactType ImpactType, const FVector& ImpactDirection, float Intensity)
{
	Intensity = FMath::Clamp(Intensity, 0.1f, 3.0f);

	switch (ImpactType)
	{
	case ESpiderCameraImpactType::LightHit:
		AddCameraTrauma(0.25f * Intensity);
		break;

	case ESpiderCameraImpactType::HeavyImpact:
		AddCameraTrauma(0.6f * Intensity);
		break;

	case ESpiderCameraImpactType::HardLanding:
		AddCameraTrauma(0.45f * Intensity);
		break;

	case ESpiderCameraImpactType::Explosion:
		AddCameraTrauma(0.85f * Intensity);
		break;

	case ESpiderCameraImpactType::DodgeWhoosh:
		AddCameraTrauma(0.2f * Intensity);
		break;

	default:
		AddCameraTrauma(0.3f * Intensity);
		break;
	}
}

void USpiderCameraComponent::SetLockOnTarget(AActor* NewTarget)
{
	LockOnTarget = NewTarget;
	if (LockOnTarget.IsValid())
	{
		SetCameraState(ESpiderCameraState::CombatLock);
	}
	else if (CurrentCameraState == ESpiderCameraState::CombatLock)
	{
		SetCameraState(ESpiderCameraState::Default);
	}
}

void USpiderCameraComponent::ClearLockOnTarget()
{
	LockOnTarget = nullptr;
	if (CurrentCameraState == ESpiderCameraState::CombatLock)
	{
		SetCameraState(ESpiderCameraState::Default);
	}
}

void USpiderCameraComponent::UpdateAutoStateFromCharacter()
{
	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	if (LockOnTarget.IsValid())
	{
		CurrentCameraState = ESpiderCameraState::CombatLock;
		return;
	}

	const ESpiderCombatState CombatState = OwnerCharacter->GetCombatState();
	if (CombatState != ESpiderCombatState::Neutral)
	{
		if (CombatState == ESpiderCombatState::Finisher)
		{
			CurrentCameraState = ESpiderCameraState::Finisher;
			return;
		}
		CurrentCameraState = ESpiderCameraState::Combat;
		return;
	}

	const ESpiderSwingState SwingState = OwnerCharacter->GetSwingState();
	if (SwingState == ESpiderSwingState::Swinging || SwingState == ESpiderSwingState::Boost)
	{
		CurrentCameraState = ESpiderCameraState::Swing;
		return;
	}
	else if (SwingState == ESpiderSwingState::DiveBomb)
	{
		CurrentCameraState = ESpiderCameraState::Dive;
		return;
	}

	const ESpiderTraversalState TraversalState = OwnerCharacter->GetTraversalState();
	if (TraversalState == ESpiderTraversalState::WallRun)
	{
		CurrentCameraState = ESpiderCameraState::WallRun;
		return;
	}

	if (OwnerCharacter->IsSprinting())
	{
		CurrentCameraState = ESpiderCameraState::Sprint;
		return;
	}

	CurrentCameraState = ESpiderCameraState::Default;
}

float USpiderCameraComponent::GetTargetFOVForState(ESpiderCameraState State) const
{
	switch (State)
	{
	case ESpiderCameraState::Sprint:      return SprintFOV;
	case ESpiderCameraState::Swing:       return SwingFOV;
	case ESpiderCameraState::Dive:        return DiveFOV;
	case ESpiderCameraState::WallRun:     return WallRunFOV;
	case ESpiderCameraState::Combat:      return CombatFOV;
	case ESpiderCameraState::CombatLock:  return CombatLockFOV;
	case ESpiderCameraState::Aim:         return AimFOV;
	case ESpiderCameraState::Finisher:    return 80.0f;
	case ESpiderCameraState::Default:
	default:                              return BaseFOV;
	}
}

float USpiderCameraComponent::GetTargetArmLengthForState(ESpiderCameraState State, float SpeedRatio) const
{
	switch (State)
	{
	case ESpiderCameraState::Sprint:
		return FMath::Lerp(BaseArmLength, SprintArmLength, SpeedRatio);

	case ESpiderCameraState::Swing:
		return FMath::Lerp(SprintArmLength, SwingArmLengthMax, SpeedRatio);

	case ESpiderCameraState::Dive:
		return DiveArmLength;

	case ESpiderCameraState::WallRun:
		return SprintArmLength;

	case ESpiderCameraState::Combat:
		return CombatArmLength;

	case ESpiderCameraState::CombatLock:
		return CombatArmLength + 30.0f;

	case ESpiderCameraState::Finisher:
		return 220.0f;

	case ESpiderCameraState::Aim:
		return 240.0f;

	case ESpiderCameraState::Default:
	default:
		return FMath::Lerp(BaseArmLength, BaseArmLength + 40.0f, SpeedRatio);
	}
}

void USpiderCameraComponent::UpdateFOV(float DeltaTime)
{
	float TargetFOV = GetTargetFOVForState(CurrentCameraState);

	if (OwnerCharacter.IsValid() && CurrentCameraState == ESpiderCameraState::Swing)
	{
		const float Speed = OwnerCharacter->GetVelocity().Size();
		const float MaxSwingSpeed = 3000.0f;
		const float SpeedRatio = FMath::Clamp(Speed / MaxSwingSpeed, 0.0f, 1.0f);
		TargetFOV = FMath::Lerp(SprintFOV, SwingFOV, SpeedRatio);
	}

	const float InterpSpeed = (CurrentCameraState == ESpiderCameraState::Dive || CurrentCameraState == ESpiderCameraState::Swing) 
		? FOVInterpSpeedFast 
		: FOVInterpSpeedDefault;

	const float NewFOV = FMath::FInterpTo(FieldOfView, TargetFOV, DeltaTime, InterpSpeed);
	SetFieldOfView(NewFOV);
}

void USpiderCameraComponent::UpdateBoomArm(float DeltaTime)
{
	if (!AttachedBoom.IsValid())
	{
		return;
	}

	float SpeedRatio = 0.0f;
	if (OwnerCharacter.IsValid())
	{
		const float Speed = OwnerCharacter->GetVelocity().Size();
		SpeedRatio = FMath::Clamp(Speed / 2500.0f, 0.0f, 1.0f);
	}

	const float TargetArmLength = GetTargetArmLengthForState(CurrentCameraState, SpeedRatio);
	AttachedBoom->TargetArmLength = FMath::FInterpTo(AttachedBoom->TargetArmLength, TargetArmLength, DeltaTime, ArmInterpSpeed);

	// Dynamically modulate lag speeds
	if (CurrentCameraState == ESpiderCameraState::Swing)
	{
		AttachedBoom->CameraLagSpeed = FMath::FInterpTo(AttachedBoom->CameraLagSpeed, SwingLagSpeed, DeltaTime, 4.0f);
		AttachedBoom->CameraRotationLagSpeed = FMath::FInterpTo(AttachedBoom->CameraRotationLagSpeed, SwingRotLagSpeed, DeltaTime, 4.0f);
	}
	else if (CurrentCameraState == ESpiderCameraState::Combat || CurrentCameraState == ESpiderCameraState::CombatLock)
	{
		AttachedBoom->CameraLagSpeed = FMath::FInterpTo(AttachedBoom->CameraLagSpeed, CombatLagSpeed, DeltaTime, 6.0f);
		AttachedBoom->CameraRotationLagSpeed = FMath::FInterpTo(AttachedBoom->CameraRotationLagSpeed, DefaultRotLagSpeed + 5.0f, DeltaTime, 6.0f);
	}
	else
	{
		AttachedBoom->CameraLagSpeed = FMath::FInterpTo(AttachedBoom->CameraLagSpeed, DefaultLagSpeed, DeltaTime, 4.0f);
		AttachedBoom->CameraRotationLagSpeed = FMath::FInterpTo(AttachedBoom->CameraRotationLagSpeed, DefaultRotLagSpeed, DeltaTime, 4.0f);
	}
}

void USpiderCameraComponent::UpdateTiltAndRoll(float DeltaTime)
{
	float TargetTilt = 0.0f;

	if (OwnerCharacter.IsValid())
	{
		if (CurrentCameraState == ESpiderCameraState::WallRun)
		{
			const ESpiderWallRunSide WallSide = OwnerCharacter->GetWallRunSide();
			if (WallSide == ESpiderWallRunSide::Left)
			{
				TargetTilt = -WallRunTiltAngle;
			}
			else if (WallSide == ESpiderWallRunSide::Right)
			{
				TargetTilt = WallRunTiltAngle;
			}
		}
		else if (CurrentCameraState == ESpiderCameraState::Swing)
		{
			// Compute centrifugal banking from lateral velocity relative to forward
			const FVector Velocity = OwnerCharacter->GetVelocity();
			const FVector Right = OwnerCharacter->GetActorRightVector();
			const float LateralSpeed = FVector::DotProduct(Velocity, Right);
			const float NormalizedLateral = FMath::Clamp(LateralSpeed / 1500.0f, -1.0f, 1.0f);
			TargetTilt = NormalizedLateral * MaxSwingCentrifugalTilt;
		}
	}

	if (DesiredRollTilt != 0.0f)
	{
		TargetTilt = DesiredRollTilt;
		DesiredRollTilt = 0.0f;
	}

	CurrentRollTilt = FMath::FInterpTo(CurrentRollTilt, TargetTilt, DeltaTime, TiltInterpSpeed);

	if (AttachedBoom.IsValid())
	{
		FRotator BoomRotOffset = AttachedBoom->SocketOffset.Rotation();
		AttachedBoom->SocketOffset.Y = FMath::FInterpTo(AttachedBoom->SocketOffset.Y, CurrentRollTilt * 1.5f, DeltaTime, TiltInterpSpeed);
	}

	// Apply roll tilt to component relative rotation
	FRotator CurrentRelativeRot = GetRelativeRotation();
	CurrentRelativeRot.Roll = CurrentRollTilt;
	SetRelativeRotation(CurrentRelativeRot);
}

void USpiderCameraComponent::UpdateTraumaShake(float DeltaTime)
{
	if (CurrentTrauma <= 0.001f)
	{
		CurrentTrauma = 0.0f;
		ProceduralOffset = FVector::ZeroVector;
		ProceduralRotationOffset = FRotator::ZeroRotator;
		return;
	}

	TraumaTimeElapsed += DeltaTime * ShakeFrequency;

	// Non-linear trauma curve: Shake magnitude = Trauma^2
	const float ShakeIntensity = CurrentTrauma * CurrentTrauma;

	// Procedural pseudo-random harmonics
	const float PitchShake = (FMath::Sin(TraumaTimeElapsed * 1.1f) + 0.5f * FMath::Sin(TraumaTimeElapsed * 2.3f)) * MaxShakePitch * ShakeIntensity;
	const float YawShake   = (FMath::Cos(TraumaTimeElapsed * 0.9f) + 0.5f * FMath::Cos(TraumaTimeElapsed * 2.7f)) * MaxShakeYaw * ShakeIntensity;
	const float RollShake  = (FMath::Sin(TraumaTimeElapsed * 1.4f)) * MaxShakeRoll * ShakeIntensity;

	ProceduralRotationOffset = FRotator(PitchShake, YawShake, RollShake);

	const float OffsetX = (FMath::Sin(TraumaTimeElapsed * 1.3f)) * MaxShakeOffset * ShakeIntensity;
	const float OffsetY = (FMath::Cos(TraumaTimeElapsed * 1.7f)) * MaxShakeOffset * ShakeIntensity;
	const float OffsetZ = (FMath::Sin(TraumaTimeElapsed * 2.1f)) * (MaxShakeOffset * 0.6f) * ShakeIntensity;

	ProceduralOffset = FVector(OffsetX, OffsetY, OffsetZ);

	// Decay trauma over time
	CurrentTrauma = FMath::Max(0.0f, CurrentTrauma - TraumaDecaySpeed * DeltaTime);

	// Apply shake to camera relative transform
	FRotator BaseRot = GetRelativeRotation();
	BaseRot.Pitch = ProceduralRotationOffset.Pitch;
	BaseRot.Yaw = ProceduralRotationOffset.Yaw;
	BaseRot.Roll = CurrentRollTilt + ProceduralRotationOffset.Roll;
	SetRelativeRotation(BaseRot);

	SetRelativeLocation(ProceduralOffset);
}

void USpiderCameraComponent::UpdateLockOnFraming(float DeltaTime)
{
	if (!LockOnTarget.IsValid() || !OwnerCharacter.IsValid() || !AttachedBoom.IsValid())
	{
		return;
	}

	const FVector PlayerPos = OwnerCharacter->GetActorLocation();
	const FVector TargetPos = LockOnTarget->GetActorLocation();
	const FVector MidPoint = (PlayerPos + TargetPos) * 0.5f;

	const FVector ToTarget = (TargetPos - PlayerPos).GetSafeNormal2D();
	if (!ToTarget.IsNearlyZero())
	{
		const FRotator DesiredBoomRot = ToTarget.Rotation();
		FRotator CurrentControlRot = AttachedBoom->GetTargetRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentControlRot, DesiredBoomRot, DeltaTime, 5.0f);
		// Maintain smooth pitch
		NewRot.Pitch = FMath::Clamp(NewRot.Pitch, -30.0f, 10.0f);
	}
}

void USpiderCameraComponent::UpdateCollisionAvoidanceGeometry(UWorld* World, const FVector& CameraLoc, const FVector& PivotLoc)
{
	if (!World)
	{
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (OwnerCharacter.IsValid())
	{
		QueryParams.AddIgnoredActor(OwnerCharacter.Get());
	}

	FHitResult HitResult;
	const bool bHit = World->SweepSingleByChannel(
		HitResult,
		PivotLoc,
		CameraLoc,
		FQuat::Identity,
		ECC_Camera,
		FCollisionShape::MakeSphere(CollisionAvoidanceProbeRadius),
		QueryParams
	);

	if (bHit && AttachedBoom.IsValid())
	{
		const float DistanceToHit = FVector::Distance(PivotLoc, HitResult.Location);
		if (DistanceToHit < AttachedBoom->TargetArmLength)
		{
			AttachedBoom->TargetArmLength = FMath::Max(DistanceToHit - MinSafeDistanceToGeometry, 60.0f);
		}
	}
}

void USpiderCameraComponent::UpdateCollisionDamping(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World || !AttachedBoom.IsValid())
	{
		return;
	}

	const FVector PivotLoc = AttachedBoom->GetComponentLocation();
	const FVector CameraLoc = GetComponentLocation();
	UpdateCollisionAvoidanceGeometry(World, CameraLoc, PivotLoc);
}
