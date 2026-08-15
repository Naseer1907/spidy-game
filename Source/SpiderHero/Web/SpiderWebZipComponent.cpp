// Copyright SpiderHero Team. All Rights Reserved.

#include "Web/SpiderWebZipComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "Movement/SpiderMovementComponent.h"
#include "Web/SpiderWebTargetingComponent.h"
#include "SpiderHero.h"

USpiderWebZipComponent::USpiderWebZipComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ZipSpeed = 3800.0f;
	DecelerationDistance = 450.0f;
	PointLaunchWindowPercent = 0.35f;
	PointLaunchForwardImpulse = 1850.0f;
	PointLaunchUpwardImpulse = 750.0f;

	bIsZipping = false;
	bPointLaunchWindowActive = false;
	ZipStartLocation = FVector::ZeroVector;
	ZipTargetLocation = FVector::ZeroVector;
	ZipTargetNormal = FVector::UpVector;
	TotalZipDistance = 0.0f;
}

void USpiderWebZipComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());
	if (CharacterOwner.IsValid())
	{
		MovementComponent = CharacterOwner->FindComponentByClass<USpiderMovementComponent>();
		TargetingComponent = CharacterOwner->FindComponentByClass<USpiderWebTargetingComponent>();
	}
}

void USpiderWebZipComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsZipping)
	{
		UpdateZipTravel(DeltaTime);
	}
}

bool USpiderWebZipComponent::StartPointZip()
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

	FSpiderWebTargetCandidate BestZip;
	if (!TargetingComponent->FindBestZipTarget(BestZip))
	{
		UE_LOG(LogSpiderWeb, Verbose, TEXT("No valid zip target found."));
		return false;
	}

	ZipStartLocation = CharacterOwner->GetActorLocation();
	ZipTargetNormal = BestZip.SurfaceNormal;

	// Target landing location slightly offset along normal
	ZipTargetLocation = BestZip.WorldLocation + (ZipTargetNormal * 45.0f);
	TotalZipDistance = (ZipTargetLocation - ZipStartLocation).Size();

	bIsZipping = true;
	bPointLaunchWindowActive = false;

	MovementComponent->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::Ziplining, false);
	CharacterOwner->SetSpiderMovementMode(ESpiderCustomMovementMode::Ziplining);

	OnZipStarted.Broadcast(ZipTargetLocation);
	UE_LOG(LogSpiderWeb, Log, TEXT("Point Zip started towards %s (Distance: %.1f)"), *ZipTargetLocation.ToString(), TotalZipDistance);

	return true;
}

void USpiderWebZipComponent::CancelZip()
{
	if (!bIsZipping)
	{
		return;
	}

	bIsZipping = false;
	bPointLaunchWindowActive = false;

	if (MovementComponent.IsValid())
	{
		MovementComponent->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, true);
	}

	if (CharacterOwner.IsValid())
	{
		CharacterOwner->SetSpiderMovementMode(ESpiderCustomMovementMode::None);
	}

	UE_LOG(LogSpiderWeb, Warning, TEXT("Point Zip cancelled."));
}

bool USpiderWebZipComponent::TryTriggerPointLaunch()
{
	if (!bIsZipping || !bPointLaunchWindowActive || !CharacterOwner.IsValid() || !MovementComponent.IsValid())
	{
		return false;
	}

	FVector LookDir = CharacterOwner->GetActorForwardVector();
	FVector LaunchVel = (LookDir * PointLaunchForwardImpulse) + (FVector::UpVector * PointLaunchUpwardImpulse);

	bIsZipping = false;
	bPointLaunchWindowActive = false;

	MovementComponent->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, false);
	MovementComponent->Velocity = LaunchVel;

	if (CharacterOwner.IsValid())
	{
		CharacterOwner->SetSpiderMovementMode(ESpiderCustomMovementMode::PointLaunch);
	}

	OnPointLaunchExecuted.Broadcast(LaunchVel);
	UE_LOG(LogSpiderWeb, Log, TEXT("Point Launch executed with velocity: %s"), *LaunchVel.ToString());

	return true;
}

void USpiderWebZipComponent::UpdateZipTravel(float DeltaTime)
{
	if (!CharacterOwner.IsValid() || !MovementComponent.IsValid())
	{
		CancelZip();
		return;
	}

	const FVector CurrentLoc = CharacterOwner->GetActorLocation();
	const FVector ToTarget = ZipTargetLocation - CurrentLoc;
	const float RemainingDistance = ToTarget.Size();
	const FVector MoveDir = ToTarget.GetSafeNormal();

	// Check if Point Launch window should be opened
	if (RemainingDistance <= TotalZipDistance * PointLaunchWindowPercent)
	{
		bPointLaunchWindowActive = true;
	}

	// Calculate Speed with deceleration near target
	float CurrentSpeed = ZipSpeed;
	if (RemainingDistance < DecelerationDistance)
	{
		CurrentSpeed = FMath::Lerp(800.0f, ZipSpeed, RemainingDistance / DecelerationDistance);
	}

	FVector FrameVelocity = MoveDir * CurrentSpeed;
	MovementComponent->Velocity = FrameVelocity;

	// Check Arrival
	if (RemainingDistance <= (CurrentSpeed * DeltaTime) || RemainingDistance < 50.0f)
	{
		ArriveAtZipTarget();
	}
}

void USpiderWebZipComponent::ArriveAtZipTarget()
{
	bIsZipping = false;
	bPointLaunchWindowActive = false;

	if (CharacterOwner.IsValid())
	{
		CharacterOwner->SetActorLocation(ZipTargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		CharacterOwner->SetSpiderMovementMode(ESpiderCustomMovementMode::None);
	}

	if (MovementComponent.IsValid())
	{
		MovementComponent->Velocity = FVector::ZeroVector;
		MovementComponent->SetMovementMode(MOVE_Falling);
	}

	OnZipCompleted.Broadcast(true);
	UE_LOG(LogSpiderWeb, Log, TEXT("Point Zip arrived at target location."));
}
