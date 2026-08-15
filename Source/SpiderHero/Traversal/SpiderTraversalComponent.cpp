// Copyright SpiderHero Team. All Rights Reserved.

#include "Traversal/SpiderTraversalComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "Movement/SpiderMovementComponent.h"
#include "SpiderHero.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

USpiderTraversalComponent::USpiderTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ForwardTraceDistance = 150.0f;
	MaxLedgeDetectionHeight = 220.0f;
	MinLedgeDetectionHeight = 40.0f;
	SphereTraceRadius = 20.0f;

	LowVaultMaxHeight = 85.0f;
	HighVaultMaxHeight = 145.0f;
	MantleMaxHeight = 220.0f;
	VaultMaxDepth = 140.0f;

	VaultDuration = 0.45f;
	MantleDuration = 0.75f;
	ClimbUpDuration = 0.85f;

	bIsExecutingTraversal = false;
	CurrentParkourAction = ESpiderParkourAction::None;
	TraversalProgress = 0.0f;
	CurrentActionDuration = 0.5f;
}

void USpiderTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());
	if (CharacterOwner.IsValid())
	{
		SpiderMovementComp = CharacterOwner->FindComponentByClass<USpiderMovementComponent>();
	}
}

void USpiderTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsExecutingTraversal)
	{
		UpdateTraversalExecution(DeltaTime);
	}
	else if (CharacterOwner.IsValid() && CharacterOwner->IsSprinting())
	{
		// Auto-detect parkour opportunities while sprinting towards obstacles
		FSpiderLedgeInfo LedgeInfo;
		if (DetectLedge(LedgeInfo))
		{
			ESpiderParkourAction Action = ClassifyObstacle(LedgeInfo);
			if (Action != ESpiderParkourAction::None)
			{
				OnParkourObstacleDetected.Broadcast(Action);
			}
		}
	}
}

bool USpiderTraversalComponent::CheckCapsuleClearance(const FVector& Location) const
{
	if (!CharacterOwner.IsValid())
	{
		return false;
	}

	UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	float Radius = Capsule->GetScaledCapsuleRadius();
	float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CharacterOwner.Get());

	FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius * 0.95f, HalfHeight * 0.95f);
	return !GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, ECC_WorldStatic, Shape, Params);
}

bool USpiderTraversalComponent::DetectLedge(FSpiderLedgeInfo& OutLedgeInfo) const
{
	if (!CharacterOwner.IsValid())
	{
		return false;
	}

	OutLedgeInfo.bIsValid = false;

	const FVector ActorLoc = CharacterOwner->GetActorLocation();
	const FVector Forward = CharacterOwner->GetActorForwardVector();
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float BaseZ = ActorLoc.Z - HalfHeight;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner.Get());
	QueryParams.bTraceComplex = false;

	// Multi-stage Forward Traces (Waist, Chest, Eye)
	const float WaistZ = BaseZ + (HalfHeight * 0.5f);
	const float ChestZ = BaseZ + HalfHeight;
	const float EyeZ = BaseZ + (HalfHeight * 1.6f);

	FHitResult WallHit;
	bool bHitWall = false;

	TArray<float> TraceHeights = { ChestZ, WaistZ, EyeZ };
	for (float Z : TraceHeights)
	{
		FVector Start(ActorLoc.X, ActorLoc.Y, Z);
		FVector End = Start + (Forward * ForwardTraceDistance);

		if (GetWorld()->SweepSingleByChannel(WallHit, Start, End, FQuat::Identity, SPIDER_TRACE_PARKOUR_OBSTACLE, FCollisionShape::MakeSphere(SphereTraceRadius), QueryParams) ||
		    GetWorld()->SweepSingleByChannel(WallHit, Start, End, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(SphereTraceRadius), QueryParams))
		{
			bHitWall = true;
			break;
		}
	}

	if (!bHitWall || !WallHit.IsValidBlockingHit())
	{
		return false;
	}

	OutLedgeInfo.WallHitLocation = WallHit.ImpactPoint;
	OutLedgeInfo.WallNormal = WallHit.ImpactNormal;

	// Downward Trace from Above Obstacle Top to locate Ledge Surface
	const FVector DownwardStart = WallHit.ImpactPoint + (-WallHit.ImpactNormal * 25.0f) + (FVector::UpVector * MaxLedgeDetectionHeight);
	const FVector DownwardEnd = DownwardStart - (FVector::UpVector * (MaxLedgeDetectionHeight + 50.0f));

	FHitResult LedgeHit;
	if (!GetWorld()->LineTraceSingleByChannel(LedgeHit, DownwardStart, DownwardEnd, ECC_WorldStatic, QueryParams))
	{
		return false;
	}

	OutLedgeInfo.LedgeTopLocation = LedgeHit.ImpactPoint;
	OutLedgeInfo.LedgeNormal = LedgeHit.ImpactNormal;
	OutLedgeInfo.ObstacleHeight = LedgeHit.ImpactPoint.Z - BaseZ;

	if (OutLedgeInfo.ObstacleHeight < MinLedgeDetectionHeight || OutLedgeInfo.ObstacleHeight > MaxLedgeDetectionHeight)
	{
		return false;
	}

	// Forward Check across obstacle to determine depth
	const FVector DepthCheckStart = LedgeHit.ImpactPoint + (Forward * 20.0f) + (FVector::UpVector * 10.0f);
	const FVector DepthCheckEnd = DepthCheckStart + (Forward * VaultMaxDepth);
	FHitResult FarEdgeHit;
	GetWorld()->LineTraceSingleByChannel(FarEdgeHit, DepthCheckEnd, DepthCheckEnd - (FVector::UpVector * 100.0f), ECC_WorldStatic, QueryParams);

	OutLedgeInfo.ObstacleDepth = FarEdgeHit.IsValidBlockingHit() ? VaultMaxDepth : 50.0f;

	// Validate Clearance for Character Capsule on landing spot
	const FVector TargetLanding = OutLedgeInfo.LedgeTopLocation + (FVector::UpVector * (HalfHeight + 2.0f)) + (Forward * 30.0f);
	OutLedgeInfo.bHasClearance = CheckCapsuleClearance(TargetLanding);
	OutLedgeInfo.bIsValid = true;

	return true;
}

ESpiderParkourAction USpiderTraversalComponent::ClassifyObstacle(const FSpiderLedgeInfo& LedgeInfo) const
{
	if (!LedgeInfo.bIsValid)
	{
		return ESpiderParkourAction::None;
	}

	if (LedgeInfo.ObstacleHeight <= LowVaultMaxHeight && LedgeInfo.ObstacleDepth <= VaultMaxDepth)
	{
		return ESpiderParkourAction::VaultLow;
	}
	else if (LedgeInfo.ObstacleHeight <= HighVaultMaxHeight && LedgeInfo.ObstacleDepth <= VaultMaxDepth)
	{
		return ESpiderParkourAction::VaultHigh;
	}
	else if (LedgeInfo.ObstacleHeight <= MantleMaxHeight && LedgeInfo.bHasClearance)
	{
		return ESpiderParkourAction::Mantle;
	}

	return ESpiderParkourAction::None;
}

bool USpiderTraversalComponent::TryPerformTraversal()
{
	if (bIsExecutingTraversal || !CharacterOwner.IsValid())
	{
		return false;
	}

	FSpiderLedgeInfo Ledge;
	if (!DetectLedge(Ledge))
	{
		return false;
	}

	ESpiderParkourAction Action = ClassifyObstacle(Ledge);
	if (Action == ESpiderParkourAction::None)
	{
		return false;
	}

	CurrentParkourAction = Action;
	bIsExecutingTraversal = true;
	TraversalProgress = 0.0f;

	TraversalStartTransform = CharacterOwner->GetActorTransform();

	const float HalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Forward = CharacterOwner->GetActorForwardVector();

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = (-Ledge.WallNormal).Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	switch (Action)
	{
	case ESpiderParkourAction::VaultLow:
		CurrentActionDuration = VaultDuration;
		TargetLocation = Ledge.LedgeTopLocation + (Forward * (Ledge.ObstacleDepth + 60.0f)) + (FVector::UpVector * HalfHeight);
		break;
	case ESpiderParkourAction::VaultHigh:
		CurrentActionDuration = VaultDuration * 1.15f;
		TargetLocation = Ledge.LedgeTopLocation + (Forward * (Ledge.ObstacleDepth + 60.0f)) + (FVector::UpVector * HalfHeight);
		break;
	case ESpiderParkourAction::Mantle:
		CurrentActionDuration = MantleDuration;
		TargetLocation = Ledge.LedgeTopLocation + (Forward * 40.0f) + (FVector::UpVector * (HalfHeight + 2.0f));
		break;
	default:
		CurrentActionDuration = ClimbUpDuration;
		TargetLocation = Ledge.LedgeTopLocation + (FVector::UpVector * (HalfHeight + 2.0f));
		break;
	}

	TraversalTargetTransform = FTransform(TargetRotation, TargetLocation, FVector::OneVector);

	if (SpiderMovementComp.IsValid())
	{
		SpiderMovementComp->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::Parkour, false);
	}

	OnTraversalActionStarted.Broadcast(CurrentParkourAction, TargetLocation);
	UE_LOG(LogSpiderMovement, Log, TEXT("Traversal started: Action %d towards %s"), static_cast<int32>(CurrentParkourAction), *TargetLocation.ToString());

	return true;
}

void USpiderTraversalComponent::CancelTraversal()
{
	if (!bIsExecutingTraversal)
	{
		return;
	}

	bIsExecutingTraversal = false;
	CurrentParkourAction = ESpiderParkourAction::None;

	if (SpiderMovementComp.IsValid())
	{
		SpiderMovementComp->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, true);
	}

	UE_LOG(LogSpiderMovement, Warning, TEXT("Traversal cancelled."));
}

void USpiderTraversalComponent::UpdateTraversalExecution(float DeltaTime)
{
	if (!CharacterOwner.IsValid())
	{
		CancelTraversal();
		return;
	}

	TraversalProgress += DeltaTime / CurrentActionDuration;

	// Hermite SmoothStep interpolation curve
	float Alpha = FMath::Clamp(TraversalProgress, 0.0f, 1.0f);
	float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);

	FVector NewLocation = FMath::Lerp(TraversalStartTransform.GetLocation(), TraversalTargetTransform.GetLocation(), SmoothAlpha);
	
	// Add an upward arch for vaults
	if (CurrentParkourAction == ESpiderParkourAction::VaultLow || CurrentParkourAction == ESpiderParkourAction::VaultHigh)
	{
		float ArcZ = FMath::Sin(Alpha * PI) * 35.0f;
		NewLocation.Z += ArcZ;
	}

	FQuat NewRotation = FQuat::Slerp(TraversalStartTransform.GetRotation(), TraversalTargetTransform.GetRotation(), SmoothAlpha);

	CharacterOwner->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (TraversalProgress >= 1.0f)
	{
		CompleteTraversal();
	}
}

void USpiderTraversalComponent::CompleteTraversal()
{
	ESpiderParkourAction CompletedAction = CurrentParkourAction;
	bIsExecutingTraversal = false;
	CurrentParkourAction = ESpiderParkourAction::None;

	if (CharacterOwner.IsValid())
	{
		CharacterOwner->SetActorTransform(TraversalTargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (SpiderMovementComp.IsValid())
	{
		SpiderMovementComp->SetSpiderCustomMovementMode(ESpiderCustomMovementMode::None, true);
	}

	OnTraversalActionEnded.Broadcast(CompletedAction);
	UE_LOG(LogSpiderMovement, Log, TEXT("Traversal completed: Action %d"), static_cast<int32>(CompletedAction));
}
