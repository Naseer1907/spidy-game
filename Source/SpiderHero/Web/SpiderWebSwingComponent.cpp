// Copyright SpiderHero Team. All Rights Reserved.

#include "Web/SpiderWebSwingComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "Movement/SpiderMovementComponent.h"
#include "Web/SpiderWebTargetingComponent.h"
#include "Abilities/SpiderStaminaComponent.h"
#include "SpiderHero.h"

USpiderWebSwingComponent::USpiderWebSwingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GravityMultiplier = 1.85f;
	CentripetalForceMultiplier = 1.15f;
	SteeringTorqueStrength = 1400.0f;
	MaxSwingSpeed = 2900.0f;
	MinSwingSpeed = 500.0f;
	AirFrictionDamping = 0.04f;

	CableReelSpeed = 800.0f;
	MinCableLength = 400.0f;

	ApexBoostMultiplier = 1.35f;
	UpwardLaunchKicker = 550.0f;
	ManualBoostImpulse = 800.0f;
	BoostStaminaCost = 20.0f;

	bIsSwinging = false;
	CurrentRopeLength = 0.0f;
	CurrentSwingVelocity = FVector::ZeroVector;
	CachedSteeringInput = FVector2D::ZeroVector;
}

void USpiderWebSwingComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());
	if (CharacterOwner.IsValid())
	{
		MovementComponent = CharacterOwner->FindComponentByClass<USpiderMovementComponent>();
		TargetingComponent = CharacterOwner->FindComponentByClass<USpiderWebTargetingComponent>();
		StaminaComponent = CharacterOwner->FindComponentByClass<USpiderStaminaComponent>();
	}
}

void USpiderWebSwingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsSwinging)
	{
		SimulatePendulumPhysics(DeltaTime);
	}
}

bool USpiderWebSwingComponent::StartWebSwing()
{
	if (!CharacterOwner.IsValid() || !MovementComponent.IsValid())
	{
		return false;
	}

	if (!TargetingComponent.IsValid())
	{
		TargetingComponent = CharacterOwner->FindComponentByClass<USpiderWebTargetingComponent>();
		if (!TargetingComponent.IsValid())
		{
			return false;
		}
	}

	if (!TargetingComponent->FindBestSwingAnchor(CurrentAnchor))
	{
		UE_LOG(LogSpiderWeb, Verbose, TEXT("No valid swing anchor found in view frustum."));
		return false;
	}

	const FVector PlayerLoc = CharacterOwner->GetActorLocation();
	CurrentRopeLength = FMath::Max((CurrentAnchor.AnchorPoint - PlayerLoc).Size(), MinCableLength);
	CurrentSwingVelocity = MovementComponent->Velocity;

	// Add minimum forward swing impetus if stationary
	if (CurrentSwingVelocity.Size() < MinSwingSpeed)
	{
		FVector ForwardDir = CharacterOwner->GetActorForwardVector();
		CurrentSwingVelocity = (ForwardDir * MinSwingSpeed) + (FVector::DownVector * 200.0f);
	}

	bIsSwinging = true;
	MovementComponent->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::WebSwinging, false);
	MovementComponent->ApplySwingVelocity(CurrentSwingVelocity);

	CharacterOwner->SetSwingState(ESpiderSwingState::Swinging);
	CharacterOwner->SetActiveAnchorInfo(CurrentAnchor);

	OnSwingAttached.Broadcast(CurrentAnchor);
	UE_LOG(LogSpiderWeb, Log, TEXT("Web Swing attached to %s (Length: %.1f cm)"), *CurrentAnchor.AnchorPoint.ToString(), CurrentRopeLength);

	return true;
}

void USpiderWebSwingComponent::ReleaseWebSwing(bool bApplyLaunchBoost)
{
	if (!bIsSwinging)
	{
		return;
	}

	bIsSwinging = false;
	FVector LaunchVelocity = CurrentSwingVelocity;
	float LaunchBoost = 0.0f;

	if (bApplyLaunchBoost)
	{
		// Boost bonus if releasing during rising arc of swing (v_z > 0)
		if (LaunchVelocity.Z > 100.0f)
		{
			LaunchBoost = (LaunchVelocity.Z / 500.0f) * ApexBoostMultiplier;
			LaunchVelocity *= (1.0f + (LaunchBoost * 0.15f));
			LaunchVelocity.Z += UpwardLaunchKicker;
		}
	}

	if (MovementComponent.IsValid())
	{
		MovementComponent->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, false);
		MovementComponent->Velocity = LaunchVelocity;
	}

	if (CharacterOwner.IsValid())
	{
		CharacterOwner->SetSwingState(ESpiderSwingState::Releasing);
		FSpiderWebAnchorInfo EmptyAnchor;
		CharacterOwner->SetActiveAnchorInfo(EmptyAnchor);
	}

	CurrentAnchor.Reset();
	OnSwingReleased.Broadcast(LaunchVelocity, LaunchBoost);
	UE_LOG(LogSpiderWeb, Log, TEXT("Web Swing released with velocity: %s (Boost: %.2f)"), *LaunchVelocity.ToString(), LaunchBoost);
}

bool USpiderWebSwingComponent::TriggerSwingBoost()
{
	if (!bIsSwinging || !StaminaComponent.IsValid())
	{
		return false;
	}

	if (!StaminaComponent->ConsumeStamina(BoostStaminaCost))
	{
		return false;
	}

	FVector TangentDir = CurrentSwingVelocity.GetSafeNormal();
	CurrentSwingVelocity += (TangentDir * ManualBoostImpulse);
	CurrentSwingVelocity.Z += (ManualBoostImpulse * 0.3f);

	if (MovementComponent.IsValid())
	{
		MovementComponent->ApplySwingVelocity(CurrentSwingVelocity);
	}

	if (CharacterOwner.IsValid())
	{
		CharacterOwner->SetSwingState(ESpiderSwingState::Boost);
	}

	OnSwingBoost.Broadcast(ManualBoostImpulse);
	UE_LOG(LogSpiderWeb, Log, TEXT("Manual swing boost triggered (+%.1f velocity)"), ManualBoostImpulse);

	return true;
}

void USpiderWebSwingComponent::ApplySteeringInput(const FVector2D& SteeringInput)
{
	CachedSteeringInput = SteeringInput;
}

FVector USpiderWebSwingComponent::CalculateTensionForce(const FVector& PlayerLoc, const FVector& Velocity, float RopeLength) const
{
	const FVector ToAnchor = CurrentAnchor.AnchorPoint - PlayerLoc;
	const float CurrentDistance = ToAnchor.Size();

	if (CurrentDistance < RopeLength || RopeLength <= 0.0f)
	{
		return FVector::ZeroVector; // Slack rope -> no tension
	}

	const FVector UnitToAnchor = ToAnchor / CurrentDistance;
	const FVector Gravity = FVector(0.0f, 0.0f, -980.0f * GravityMultiplier);

	// 1. Gravity counter-tension
	float GravityAlongRope = FMath::Max(0.0f, FVector::DotProduct(-Gravity, UnitToAnchor));

	// 2. Centripetal tension: T_c = m * (v^2 / L)
	FVector TangentialVelocity = Velocity - (FVector::DotProduct(Velocity, UnitToAnchor) * UnitToAnchor);
	float SpeedSq = TangentialVelocity.SizeSquared();
	float CentripetalTension = (SpeedSq / RopeLength) * CentripetalForceMultiplier;

	return UnitToAnchor * (GravityAlongRope + CentripetalTension);
}

void USpiderWebSwingComponent::SimulatePendulumPhysics(float DeltaTime)
{
	if (!CharacterOwner.IsValid() || !MovementComponent.IsValid())
	{
		ReleaseWebSwing(false);
		return;
	}

	const FVector PlayerLoc = CharacterOwner->GetActorLocation();
	const FVector ToAnchor = CurrentAnchor.AnchorPoint - PlayerLoc;
	const float DistanceToAnchor = ToAnchor.Size();
	const FVector UnitToAnchor = ToAnchor.GetSafeNormal();

	// If player swings past anchor height or loses tether line, auto release
	if (PlayerLoc.Z > CurrentAnchor.AnchorPoint.Z + 150.0f)
	{
		ReleaseWebSwing(true);
		return;
	}

	// 1. Gravity Force
	const FVector Gravity = FVector(0.0f, 0.0f, -980.0f * GravityMultiplier);

	// 2. Pendulum Constraint Tension Force
	FVector Tension = CalculateTensionForce(PlayerLoc, CurrentSwingVelocity, CurrentRopeLength);

	// 3. Lateral Steering Torque
	FVector SwingRight = FVector::CrossProduct(UnitToAnchor, FVector::UpVector).GetSafeNormal();
	FVector SteeringForce = SwingRight * (CachedSteeringInput.X * SteeringTorqueStrength);

	// 4. Net Acceleration & Velocity Integration
	FVector NetAcceleration = Gravity + Tension + SteeringForce - (CurrentSwingVelocity * AirFrictionDamping);
	CurrentSwingVelocity += NetAcceleration * DeltaTime;

	// 5. Constrain Velocity radially (prevent rope elongation beyond CurrentRopeLength)
	if (DistanceToAnchor >= CurrentRopeLength)
	{
		float RadialSpeed = FVector::DotProduct(CurrentSwingVelocity, -UnitToAnchor);
		if (RadialSpeed > 0.0f)
		{
			CurrentSwingVelocity += (UnitToAnchor * RadialSpeed);
		}
	}

	// 6. Clamp Max Swing Speed
	CurrentSwingVelocity = CurrentSwingVelocity.GetClampedToMaxSize(MaxSwingSpeed);

	// 7. Apply to Character Movement Component
	MovementComponent->ApplySwingVelocity(CurrentSwingVelocity);
}
