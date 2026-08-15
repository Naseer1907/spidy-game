// Copyright SpiderHero Team. All Rights Reserved.

#include "Web/SpiderWebTargetingComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "Core/SpiderHeroPlayerController.h"
#include "SpiderHero.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"

USpiderWebTargetingComponent::USpiderWebTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.033f; // ~30Hz targeting scan for optimal performance

	MaxSwingDistance = 7500.0f;
	MinSwingDistance = 600.0f;
	MaxZipDistance = 6000.0f;
	MaxTargetingAngle = 65.0f;
	FrustumRaycastCount = 24;

	AngleWeight = 0.45f;
	DistanceWeight = 0.25f;
	HeightBonusWeight = 0.30f;
}

void USpiderWebTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());
	if (CharacterOwner.IsValid())
	{
		PlayerControllerOwner = Cast<ASpiderHeroPlayerController>(CharacterOwner->GetController());
	}
}

void USpiderWebTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PlayerControllerOwner.IsValid() && CharacterOwner.IsValid())
	{
		PlayerControllerOwner = Cast<ASpiderHeroPlayerController>(CharacterOwner->GetController());
	}

	ScanEnvironmentForTargets();
}

bool USpiderWebTargetingComponent::ProjectWorldToScreen(const FVector& WorldLoc, FVector2D& OutScreenPos) const
{
	if (PlayerControllerOwner.IsValid())
	{
		return UGameplayStatics::ProjectWorldToScreen(PlayerControllerOwner.Get(), WorldLoc, OutScreenPos);
	}
	return false;
}

float USpiderWebTargetingComponent::EvaluateAnchorScore(const FVector& PlayerLoc, const FVector& ViewForward, const FHitResult& Hit, bool bForSwinging) const
{
	const FVector DirToHit = (Hit.ImpactPoint - PlayerLoc).GetSafeNormal();
	const float Distance = (Hit.ImpactPoint - PlayerLoc).Size();

	// 1. Angle Score (Dot product against camera forward)
	const float Dot = FVector::DotProduct(ViewForward, DirToHit);
	if (Dot < FMath::Cos(FMath::DegreesToRadians(MaxTargetingAngle)))
	{
		return -1.0f; // Outside targeting cone
	}
	const float AngleScore = FMath::Clamp((Dot + 1.0f) * 0.5f, 0.0f, 1.0f);

	// 2. Distance Score (Prefer middle of range)
	const float IdealDistance = (MaxSwingDistance + MinSwingDistance) * 0.5f;
	const float DistDiff = FMath::Abs(Distance - IdealDistance);
	const float DistanceScore = FMath::Clamp(1.0f - (DistDiff / IdealDistance), 0.0f, 1.0f);

	// 3. Height Score (Prefer anchors significantly higher than player for swing pendulum)
	const float HeightDelta = Hit.ImpactPoint.Z - PlayerLoc.Z;
	float HeightScore = 0.0f;
	if (bForSwinging)
	{
		if (HeightDelta < 200.0f)
		{
			return -1.0f; // Reject anchors too low for swinging
		}
		HeightScore = FMath::Clamp(HeightDelta / 2000.0f, 0.0f, 1.0f);
	}
	else
	{
		// For point zips, any visible horizontal/vertical perch is valid
		HeightScore = 0.5f + FMath::Clamp(HeightDelta / 4000.0f, -0.5f, 0.5f);
	}

	return (AngleScore * AngleWeight) + (DistanceScore * DistanceWeight) + (HeightScore * HeightBonusWeight);
}

void USpiderWebTargetingComponent::ScanEnvironmentForTargets()
{
	if (!CharacterOwner.IsValid() || !PlayerControllerOwner.IsValid())
	{
		return;
	}

	FVector CameraLoc;
	FRotator CameraRot;
	PlayerControllerOwner->GetPlayerViewPoint(CameraLoc, CameraRot);

	const FVector ViewForward = CameraRot.Vector();
	const FVector PlayerLoc = CharacterOwner->GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner.Get());
	QueryParams.bTraceComplex = false;

	FSpiderWebTargetCandidate BestSwing;
	FSpiderWebTargetCandidate BestZip;
	float BestSwingScore = -1.0f;
	float BestZipScore = -1.0f;

	// Distribute raycasts in concentric rings across view cone
	const int32 Rings = 3;
	const int32 RaysPerRing = FrustumRaycastCount / Rings;

	for (int32 r = 0; r < Rings; ++r)
	{
		float RingAngle = ((r + 1) / static_cast<float>(Rings)) * MaxTargetingAngle;
		for (int32 i = 0; i < RaysPerRing; ++i)
		{
			float Angle = (i / static_cast<float>(RaysPerRing)) * 2.0f * PI;
			FVector RayDir = CameraRot.RotateVector(FVector(
				FMath::Cos(FMath::DegreesToRadians(RingAngle)),
				FMath::Sin(FMath::DegreesToRadians(RingAngle)) * FMath::Cos(Angle),
				FMath::Sin(FMath::DegreesToRadians(RingAngle)) * FMath::Sin(Angle)
			)).GetSafeNormal();

			FVector TraceEnd = CameraLoc + (RayDir * MaxSwingDistance);
			FHitResult Hit;

			if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, TraceEnd, SPIDER_TRACE_WEB_ANCHOR, QueryParams) ||
			    GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, TraceEnd, ECC_WorldStatic, QueryParams))
			{
				// Validate line of sight from player chest to anchor
				FHitResult LOSHit;
				FVector ChestLoc = PlayerLoc + FVector(0.0f, 0.0f, 50.0f);
				if (!GetWorld()->LineTraceSingleByChannel(LOSHit, ChestLoc, Hit.ImpactPoint, ECC_WorldStatic, QueryParams) ||
				    (LOSHit.ImpactPoint - Hit.ImpactPoint).SizeSquared() < 400.0f)
				{
					// Evaluate Swing Score
					float SwingScore = EvaluateAnchorScore(PlayerLoc, ViewForward, Hit, true);
					if (SwingScore > BestSwingScore)
					{
						BestSwingScore = SwingScore;
						BestSwing.WorldLocation = Hit.ImpactPoint;
						BestSwing.SurfaceNormal = Hit.ImpactNormal;
						BestSwing.Score = SwingScore;
						BestSwing.Distance = (Hit.ImpactPoint - PlayerLoc).Size();
						BestSwing.HitActor = Hit.GetActor();
						BestSwing.HitComponent = Hit.GetComponent();
						BestSwing.bIsSwingAnchor = true;
						BestSwing.bIsValid = true;
						BestSwing.bIsOnScreen = ProjectWorldToScreen(Hit.ImpactPoint, BestSwing.ScreenPosition);
					}

					// Evaluate Zip Score
					float ZipScore = EvaluateAnchorScore(PlayerLoc, ViewForward, Hit, false);
					if (ZipScore > BestZipScore && (Hit.ImpactPoint - PlayerLoc).Size() <= MaxZipDistance)
					{
						BestZipScore = ZipScore;
						BestZip.WorldLocation = Hit.ImpactPoint;
						BestZip.SurfaceNormal = Hit.ImpactNormal;
						BestZip.Score = ZipScore;
						BestZip.Distance = (Hit.ImpactPoint - PlayerLoc).Size();
						BestZip.HitActor = Hit.GetActor();
						BestZip.HitComponent = Hit.GetComponent();
						BestZip.bIsZipAnchor = true;
						BestZip.bIsValid = true;
						BestZip.bIsOnScreen = ProjectWorldToScreen(Hit.ImpactPoint, BestZip.ScreenPosition);
					}
				}
			}
		}
	}

	// Update cached best targets
	if (BestSwing.bIsValid)
	{
		CurrentBestSwingTarget = BestSwing;
		OnBestSwingTargetUpdated.Broadcast(CurrentBestSwingTarget);
	}
	else if (CurrentBestSwingTarget.bIsValid)
	{
		CurrentBestSwingTarget.bIsValid = false;
		OnSwingTargetLost.Broadcast();
	}

	if (BestZip.bIsValid)
	{
		CurrentBestZipTarget = BestZip;
		OnBestZipTargetUpdated.Broadcast(CurrentBestZipTarget);
	}
}

bool USpiderWebTargetingComponent::FindBestSwingAnchor(FSpiderWebAnchorInfo& OutAnchorInfo)
{
	if (CurrentBestSwingTarget.bIsValid)
	{
		OutAnchorInfo.AnchorPoint = CurrentBestSwingTarget.WorldLocation;
		OutAnchorInfo.AttachNormal = CurrentBestSwingTarget.SurfaceNormal;
		OutAnchorInfo.AttachedActor = CurrentBestSwingTarget.HitActor;
		OutAnchorInfo.AttachedComponent = CurrentBestSwingTarget.HitComponent;
		OutAnchorInfo.CableLength = CurrentBestSwingTarget.Distance;
		OutAnchorInfo.bIsValid = true;

		// Hand determination: positive cross product Z means anchor is to the left
		if (CharacterOwner.IsValid())
		{
			FVector ToAnchor = (CurrentBestSwingTarget.WorldLocation - CharacterOwner->GetActorLocation()).GetSafeNormal();
			FVector Cross = FVector::CrossProduct(CharacterOwner->GetActorForwardVector(), ToAnchor);
			OutAnchorInfo.bIsLeftHand = (Cross.Z > 0.0f);
		}

		return true;
	}

	OutAnchorInfo.Reset();
	return false;
}

bool USpiderWebTargetingComponent::FindBestZipTarget(FSpiderWebTargetCandidate& OutZipTarget)
{
	if (CurrentBestZipTarget.bIsValid)
	{
		OutZipTarget = CurrentBestZipTarget;
		return true;
	}
	return false;
}
