// Copyright SpiderHero Team. All Rights Reserved.

#include "Abilities/SpiderAbilityComponent.h"
#include "SpiderHero.h"
#include "Character/SpiderHeroCharacter.h"
#include "Combat/Interfaces/IWebAttachable.h"
#include "Combat/Interfaces/IDamageable.h"
#include "Combat/Interfaces/ICombatTarget.h"
#include "Camera/SpiderCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

USpiderAbilityComponent::USpiderAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxWebFluid = 100.0f;
	CurrentWebFluid = 100.0f;
	WebFluidRegenRate = 12.0f;
	WebFluidRegenDelay = 2.0f;

	WebShotFluidCost = 15.0f;
	WebShotRange = 2500.0f;
	WebShotGaugeAmount = 35.0f;
	WebShotDamage = 15.0f;

	WebBurstFluidCost = 40.0f;
	WebBurstRadius = 600.0f;
	WebBurstKnockbackForce = 700.0f;
	WebBurstGaugeAmount = 80.0f;

	WebPullFluidCost = 20.0f;
	WebPullRange = 2000.0f;
	WebPullYankForce = 1200.0f;
	WebStrikeLaunchSpeed = 2200.0f;

	WallPinTraceDistance = 180.0f;
	WallPinDuration = 8.0f;

	TimeSinceLastFluidConsume = 0.0f;
}

void USpiderAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		OwnerCharacter = Cast<ASpiderHeroCharacter>(OwnerActor);
		CameraComponent = OwnerActor->FindComponentByClass<USpiderCameraComponent>();
	}

	CurrentWebFluid = MaxWebFluid;
	UE_LOG(LogSpiderWeb, Log, TEXT("SpiderAbilityComponent initialized on %s"), OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"));
}

void USpiderAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastFluidConsume += DeltaTime;
	if (TimeSinceLastFluidConsume >= WebFluidRegenDelay && CurrentWebFluid < MaxWebFluid)
	{
		RestoreWebFluid(WebFluidRegenRate * DeltaTime);
	}
}

bool USpiderAbilityComponent::FireWebShot(AActor* OptionalTarget)
{
	if (!ConsumeWebFluid(WebShotFluidCost) || !OwnerCharacter.IsValid())
	{
		return false;
	}

	AActor* Target = OptionalTarget ? OptionalTarget : FindBestAbilityTarget(WebShotRange);
	FVector TraceStart = OwnerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
	FVector TargetLocation = Target ? Target->GetActorLocation() : TraceStart + (OwnerCharacter->GetActorForwardVector() * WebShotRange);

	if (Target && Target->GetClass()->ImplementsInterface(UWebAttachable::StaticClass()))
	{
		IWebAttachable::Execute_OnWebAttached(Target, OwnerCharacter.Get(), TargetLocation);
		IWebAttachable::Execute_ApplyWebGauge(Target, WebShotGaugeAmount, OwnerCharacter.Get());

		if (Target->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			FSpiderHitReaction HitReaction;
			HitReaction.DamageTaken = WebShotDamage;
			HitReaction.Attacker = OwnerCharacter.Get();
			HitReaction.HitLocation = TargetLocation;
			HitReaction.ImpactForce = OwnerCharacter->GetActorForwardVector() * 200.0f;

			IDamageable::Execute_TakeDamageCustom(Target, WebShotDamage, HitReaction);
		}

		if (IWebAttachable::Execute_IsWebBound(Target))
		{
			TryPinTargetToWall(Target);
		}
	}
	else
	{
		// Ray trace check against world
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerCharacter.Get());

		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TargetLocation, ECC_Visibility, Params))
		{
			if (Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UWebAttachable::StaticClass()))
			{
				IWebAttachable::Execute_OnWebAttached(Hit.GetActor(), OwnerCharacter.Get(), Hit.ImpactPoint);
				IWebAttachable::Execute_ApplyWebGauge(Hit.GetActor(), WebShotGaugeAmount, OwnerCharacter.Get());
			}
		}
	}

	if (CameraComponent.IsValid())
	{
		CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::LightHit, OwnerCharacter->GetActorForwardVector(), 0.5f);
	}

	OnWebAbilityFired.Broadcast(ESpiderWebAbilityType::WebShot, Target);
	return true;
}

bool USpiderAbilityComponent::FireWebBurst()
{
	if (!ConsumeWebFluid(WebBurstFluidCost) || !OwnerCharacter.IsValid())
	{
		return false;
	}

	const FVector CenterLoc = OwnerCharacter->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(WebBurstRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	GetWorld()->OverlapMultiByChannel(Overlaps, CenterLoc, FQuat::Identity, ECC_Pawn, SphereShape, Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == OwnerCharacter.Get())
		{
			continue;
		}

		const FVector Direction = (HitActor->GetActorLocation() - CenterLoc).GetSafeNormal();

		if (HitActor->GetClass()->ImplementsInterface(UWebAttachable::StaticClass()))
		{
			IWebAttachable::Execute_ApplyWebGauge(HitActor, WebBurstGaugeAmount, OwnerCharacter.Get());
			if (IWebAttachable::Execute_IsWebBound(HitActor))
			{
				TryPinTargetToWall(HitActor);
			}
		}

		if (HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			FSpiderHitReaction Reaction;
			Reaction.DamageTaken = 30.0f;
			Reaction.Attacker = OwnerCharacter.Get();
			Reaction.ImpactForce = (Direction + FVector::UpVector * 0.3f).GetSafeNormal() * WebBurstKnockbackForce;
			Reaction.bKnockdown = true;

			IDamageable::Execute_TakeDamageCustom(HitActor, 30.0f, Reaction);
			IDamageable::Execute_ApplyKnockdown(HitActor, Reaction.ImpactForce);
		}
	}

	if (CameraComponent.IsValid())
	{
		CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::Explosion, FVector::UpVector, 1.5f);
	}

	OnWebAbilityFired.Broadcast(ESpiderWebAbilityType::WebBurst, nullptr);
	return true;
}

bool USpiderAbilityComponent::FireWebPull(AActor* OptionalTarget)
{
	if (!ConsumeWebFluid(WebPullFluidCost) || !OwnerCharacter.IsValid())
	{
		return false;
	}

	AActor* Target = OptionalTarget ? OptionalTarget : FindBestAbilityTarget(WebPullRange);
	if (!Target)
	{
		return false;
	}

	int32 ThreatLevel = 1;
	if (Target->GetClass()->ImplementsInterface(UCombatTarget::StaticClass()))
	{
		ThreatLevel = ICombatTarget::Execute_GetTargetThreatLevel(Target);
	}

	if (ThreatLevel <= 1)
	{
		// Light enemy: Yank enemy toward player
		const FVector PullDir = (OwnerCharacter->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
		if (Target->GetClass()->ImplementsInterface(UWebAttachable::StaticClass()))
		{
			IWebAttachable::Execute_OnWebPulled(Target, OwnerCharacter.Get(), PullDir, WebPullYankForce);
		}
		if (Target->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			IDamageable::Execute_ApplyKnockdown(Target, (PullDir + FVector::UpVector * 0.4f).GetSafeNormal() * WebPullYankForce);
		}
	}
	else
	{
		// Heavy enemy / Boss: Web Strike launch player to target
		const FVector LaunchDir = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
		OwnerCharacter->LaunchCharacter(LaunchDir * WebStrikeLaunchSpeed, true, true);
		OwnerCharacter->SetCombatState(ESpiderCombatState::WebStrike);
	}

	if (CameraComponent.IsValid())
	{
		CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::LightHit, OwnerCharacter->GetActorForwardVector(), 1.0f);
	}

	OnWebAbilityFired.Broadcast(ESpiderWebAbilityType::WebPull, Target);
	return true;
}

bool USpiderAbilityComponent::TryPinTargetToWall(AActor* TargetActor)
{
	if (!TargetActor || !TargetActor->GetClass()->ImplementsInterface(UWebAttachable::StaticClass()))
	{
		return false;
	}

	FHitResult WallHit;
	if (CheckNearbyWall(TargetActor->GetActorLocation(), WallHit))
	{
		const bool bPinned = IWebAttachable::Execute_TryPinToSurface(TargetActor, WallHit.ImpactPoint, WallHit.ImpactNormal);
		if (bPinned)
		{
			OnEnemyWebBound.Broadcast(TargetActor, true);
			UE_LOG(LogSpiderWeb, Log, TEXT("Enemy %s successfully PINNED to wall!"), *TargetActor->GetName());
			return true;
		}
	}

	OnEnemyWebBound.Broadcast(TargetActor, false);
	return false;
}

bool USpiderAbilityComponent::CheckNearbyWall(const FVector& Location, FHitResult& OutWallHit) const
{
	const FVector Directions[] = {
		FVector::ForwardVector,
		-FVector::ForwardVector,
		FVector::RightVector,
		-FVector::RightVector
	};

	FCollisionQueryParams Params;
	if (OwnerCharacter.IsValid())
	{
		Params.AddIgnoredActor(OwnerCharacter.Get());
	}

	for (const FVector& Dir : Directions)
	{
		if (GetWorld()->LineTraceSingleByChannel(OutWallHit, Location, Location + (Dir * WallPinTraceDistance), ECC_WorldStatic, Params))
		{
			return true;
		}
	}

	return false;
}

bool USpiderAbilityComponent::ConsumeWebFluid(float Amount)
{
	if (CurrentWebFluid >= Amount)
	{
		CurrentWebFluid -= Amount;
		TimeSinceLastFluidConsume = 0.0f;
		OnWebFluidChanged.Broadcast(CurrentWebFluid, MaxWebFluid, -Amount);
		return true;
	}
	return false;
}

void USpiderAbilityComponent::RestoreWebFluid(float Amount)
{
	const float OldFluid = CurrentWebFluid;
	CurrentWebFluid = FMath::Clamp(CurrentWebFluid + Amount, 0.0f, MaxWebFluid);
	const float Delta = CurrentWebFluid - OldFluid;

	if (Delta > 0.0f)
	{
		OnWebFluidChanged.Broadcast(CurrentWebFluid, MaxWebFluid, Delta);
	}
}

AActor* USpiderAbilityComponent::FindBestAbilityTarget(float MaxRange) const
{
	if (!OwnerCharacter.IsValid())
	{
		return nullptr;
	}

	const FVector Loc = OwnerCharacter->GetActorLocation();
	const FVector Forward = OwnerCharacter->GetActorForwardVector();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Shape = FCollisionShape::MakeSphere(MaxRange);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	GetWorld()->OverlapMultiByChannel(Overlaps, Loc, FQuat::Identity, ECC_Pawn, Shape, Params);

	AActor* BestTarget = nullptr;
	float BestDot = 0.3f; // minimum cone

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == OwnerCharacter.Get())
		{
			continue;
		}

		if (Candidate->GetClass()->ImplementsInterface(UWebAttachable::StaticClass()) ||
			Candidate->GetClass()->ImplementsInterface(UCombatTarget::StaticClass()))
		{
			const FVector ToCandidate = (Candidate->GetActorLocation() - Loc).GetSafeNormal2D();
			const float Dot = FVector::DotProduct(Forward, ToCandidate);

			if (Dot > BestDot)
			{
				BestDot = Dot;
				BestTarget = Candidate;
			}
		}
	}

	return BestTarget;
}
