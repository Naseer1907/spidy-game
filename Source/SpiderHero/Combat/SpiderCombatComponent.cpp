// Copyright SpiderHero Team. All Rights Reserved.

#include "Combat/SpiderCombatComponent.h"
#include "SpiderHero.h"
#include "Character/SpiderHeroCharacter.h"
#include "Combat/Interfaces/IDamageable.h"
#include "Combat/Interfaces/ICombatTarget.h"
#include "Camera/SpiderCameraComponent.h"
#include "Abilities/SpiderStaminaComponent.h"
#include "Abilities/SpiderHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"

USpiderCombatComponent::USpiderCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CurrentComboStep = 0;
	bIsAttacking = false;
	bIsInAirCombat = false;
	bComboWindowOpen = false;
	bBufferQueued = false;
	QueuedAttackType = ESpiderAttackType::LightGround;

	bIsWarping = false;
	WarpTargetLocation = FVector::ZeroVector;
	WarpTargetRotation = FRotator::ZeroRotator;
	WarpSpeed = 16.0f;

	bIsDodging = false;
	bPerfectDodgeActive = false;

	NormalDodgeDuration = 0.4f;
	PerfectDodgeWindowDuration = 0.25f;
	PerfectDodgeSlowMoDuration = 0.65f;
	PerfectDodgeTimeDilation = 0.15f;
	DodgeImpulse = 900.0f;

	MaxWarpDistance = 550.0f;
	WarpTargetPadding = 120.0f;
	AutoAimAngle = 70.0f;

	DefaultHitPauseDuration = 0.065f;
	DefaultHitPauseDilation = 0.02f;

	GroundSlamDamageRadius = 450.0f;
	GroundSlamKnockback = 800.0f;
}

void USpiderCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		OwnerCharacter = Cast<ASpiderHeroCharacter>(OwnerActor);
		CameraComponent = OwnerActor->FindComponentByClass<USpiderCameraComponent>();
		StaminaComponent = OwnerActor->FindComponentByClass<USpiderStaminaComponent>();
	}

	UE_LOG(LogSpiderCombat, Log, TEXT("SpiderCombatComponent initialized on %s"), OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"));
}

void USpiderCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsWarping)
	{
		UpdateMotionWarping(DeltaTime);
	}
}

bool USpiderCombatComponent::TryLightAttack()
{
	if (!OwnerCharacter.IsValid())
	{
		return false;
	}

	if (bIsAttacking)
	{
		if (bComboWindowOpen)
		{
			bBufferQueued = true;
			QueuedAttackType = OwnerCharacter->GetCharacterMovement()->IsFalling() ? ESpiderAttackType::LightAir : ESpiderAttackType::LightGround;
			return true;
		}
		return false;
	}

	const bool bIsFalling = OwnerCharacter->GetCharacterMovement()->IsFalling();
	USpiderCombatDataAsset* SelectedAttack = nullptr;

	if (bIsFalling)
	{
		bIsInAirCombat = true;
		if (AirComboChain.Num() > 0)
		{
			const int32 Index = FMath::Clamp(CurrentComboStep, 0, AirComboChain.Num() - 1);
			SelectedAttack = AirComboChain[Index];
		}
	}
	else
	{
		bIsInAirCombat = false;
		if (LightComboChain.Num() > 0)
		{
			const int32 Index = FMath::Clamp(CurrentComboStep, 0, LightComboChain.Num() - 1);
			SelectedAttack = LightComboChain[Index];
		}
	}

	if (SelectedAttack)
	{
		StartAttack(SelectedAttack);
		return true;
	}

	// Fallback dynamic attack logic if data assets are pending assignment in BP
	bIsAttacking = true;
	OwnerCharacter->SetCombatState(ESpiderCombatState::LightAttacking);

	// Assisted targeting & warping
	AActor* Target = FindBestCombatTarget(MaxWarpDistance, AutoAimAngle);
	if (Target)
	{
		SetCurrentTarget(Target);
	}

	ExecuteHitTrace();
	OpenComboWindow();

	GetWorld()->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &USpiderCombatComponent::OnAttackFinished, 0.45f, false);
	return true;
}

bool USpiderCombatComponent::TryHeavyAttack()
{
	if (!OwnerCharacter.IsValid() || bIsAttacking)
	{
		return false;
	}

	if (HeavyLauncherAttack)
	{
		StartAttack(HeavyLauncherAttack);
		return true;
	}

	// Default Uppercut Launcher behavior
	bIsAttacking = true;
	OwnerCharacter->SetCombatState(ESpiderCombatState::HeavyAttacking);

	AActor* Target = FindBestCombatTarget(MaxWarpDistance, AutoAimAngle);
	if (Target)
	{
		SetCurrentTarget(Target);
	}

	ExecuteHitTrace();

	GetWorld()->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &USpiderCombatComponent::OnAttackFinished, 0.6f, false);
	return true;
}

bool USpiderCombatComponent::TryDodge()
{
	if (!OwnerCharacter.IsValid() || bIsDodging)
	{
		return false;
	}

	bIsDodging = true;
	bIsAttacking = false;
	bIsWarping = false;
	OwnerCharacter->SetCombatState(ESpiderCombatState::Dodging);

	// Grant I-Frames
	USpiderHealthComponent* HealthComp = OwnerCharacter->FindComponentByClass<USpiderHealthComponent>();
	if (HealthComp)
	{
		HealthComp->GrantInvulnerability(NormalDodgeDuration);
	}

	// Check for incoming threats to evaluate Perfect Dodge
	TArray<FOverlapResult> Overlaps;
	FCollisionShape DetectionSphere = FCollisionShape::MakeSphere(500.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	bool bFoundThreat = false;
	AActor* ThreatActor = nullptr;

	if (GetWorld()->OverlapMultiByChannel(Overlaps, OwnerCharacter->GetActorLocation(), FQuat::Identity, ECC_Pawn, DetectionSphere, Params))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Candidate = Overlap.GetActor();
			if (Candidate && Candidate != OwnerCharacter.Get())
			{
				if (Candidate->GetClass()->ImplementsInterface(UDamageable::StaticClass()) ||
					Candidate->GetClass()->ImplementsInterface(UCombatTarget::StaticClass()))
				{
					bFoundThreat = true;
					ThreatActor = Candidate;
					break;
				}
			}
		}
	}

	if (bFoundThreat && ThreatActor)
	{
		TriggerPerfectDodge(ThreatActor);
	}

	// Directional Dodge Impulse
	const FVector InputDir = OwnerCharacter->GetLastMovementInputVector();
	FVector DodgeDirection = InputDir.IsNearlyZero() ? -OwnerCharacter->GetActorForwardVector() : InputDir.GetSafeNormal();
	OwnerCharacter->LaunchCharacter(DodgeDirection * DodgeImpulse, true, false);

	// Camera impulse
	if (CameraComponent.IsValid())
	{
		CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::DodgeWhoosh, DodgeDirection, 1.0f);
	}

	FTimerHandle DodgeTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, [this]()
	{
		bIsDodging = false;
		if (OwnerCharacter.IsValid() && OwnerCharacter->GetCombatState() == ESpiderCombatState::Dodging)
		{
			OwnerCharacter->SetCombatState(ESpiderCombatState::Neutral);
		}
	}, NormalDodgeDuration, false);

	return true;
}

bool USpiderCombatComponent::TryFinisher()
{
	if (!OwnerCharacter.IsValid() || bIsAttacking)
	{
		return false;
	}

	AActor* BestTarget = FindBestCombatTarget(450.0f, 90.0f);
	if (!BestTarget)
	{
		return false;
	}

	if (BestTarget->GetClass()->ImplementsInterface(IDamageable::StaticClass()))
	{
		const float HealthPct = IDamageable::Execute_GetHealthPercent(BestTarget);
		if (HealthPct > 0.45f)
		{
			// Finisher requires target to be weakened, stunned, or webbed
			return false;
		}
	}

	bIsAttacking = true;
	OwnerCharacter->SetCombatState(ESpiderCombatState::Finisher);
	SetCurrentTarget(BestTarget);

	// Cinematic Finisher Hit
	FSpiderHitReaction HitReaction;
	HitReaction.DamageTaken = 200.0f;
	HitReaction.Attacker = OwnerCharacter.Get();
	HitReaction.HitLocation = BestTarget->GetActorLocation();
	HitReaction.ImpactForce = OwnerCharacter->GetActorForwardVector() * 1200.0f;
	HitReaction.bIsCritical = true;

	if (BestTarget->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		IDamageable::Execute_TakeDamageCustom(BestTarget, 200.0f, HitReaction);
	}

	ApplyHitPause(0.12f, 0.01f);
	if (CameraComponent.IsValid())
	{
		CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::HeavyImpact, OwnerCharacter->GetActorForwardVector(), 2.0f);
	}

	OnAttackHit.Broadcast(BestTarget, 200.0f, true);

	GetWorld()->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &USpiderCombatComponent::OnAttackFinished, 0.8f, false);
	return true;
}

bool USpiderCombatComponent::TryGroundSlam()
{
	if (!OwnerCharacter.IsValid() || !OwnerCharacter->GetCharacterMovement()->IsFalling())
	{
		return false;
	}

	bIsAttacking = true;
	OwnerCharacter->SetCombatState(ESpiderCombatState::HeavyAttacking);

	// Slam down with immense velocity
	OwnerCharacter->GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, -2200.0f);

	// When landing, shockwave will be triggered
	FTimerHandle SlamCheckTimer;
	GetWorld()->GetTimerManager().SetTimer(SlamCheckTimer, [this]()
	{
		if (!OwnerCharacter.IsValid())
		{
			return;
		}

		if (!OwnerCharacter->GetCharacterMovement()->IsFalling())
		{
			// Ground impact AOE
			TArray<FOverlapResult> Hits;
			FCollisionShape SphereShape = FCollisionShape::MakeSphere(GroundSlamDamageRadius);
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(OwnerCharacter.Get());

			GetWorld()->OverlapMultiByChannel(Hits, OwnerCharacter->GetActorLocation(), FQuat::Identity, ECC_Pawn, SphereShape, Params);

			for (const FOverlapResult& Overlap : Hits)
			{
				AActor* Target = Overlap.GetActor();
				if (Target && Target->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
				{
					const FVector Dir = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
					FSpiderHitReaction HitReact;
					HitReact.DamageTaken = 60.0f;
					HitReact.Attacker = OwnerCharacter.Get();
					HitReact.ImpactForce = (Dir + FVector::UpVector * 0.5f).GetSafeNormal() * GroundSlamKnockback;
					HitReact.bKnockdown = true;

					IDamageable::Execute_TakeDamageCustom(Target, 60.0f, HitReact);
					IDamageable::Execute_ApplyKnockdown(Target, HitReact.ImpactForce);
				}
			}

			if (CameraComponent.IsValid())
			{
				CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::HardLanding, FVector::DownVector, 2.0f);
			}

			OnAttackFinished();
		}
	}, 0.05f, true);

	return true;
}

void USpiderCombatComponent::StartAttack(USpiderCombatDataAsset* AttackData)
{
	if (!AttackData || !OwnerCharacter.IsValid())
	{
		return;
	}

	ActiveAttackData = AttackData;
	bIsAttacking = true;
	bComboWindowOpen = false;
	bBufferQueued = false;

	// Consume Stamina
	if (StaminaComponent.IsValid() && AttackData->StaminaCost > 0.0f)
	{
		StaminaComponent->ConsumeStamina(AttackData->StaminaCost);
	}

	// Set Combat State
	if (AttackData->AttackType == ESpiderAttackType::HeavyGround || AttackData->bCausesLauncher)
	{
		OwnerCharacter->SetCombatState(ESpiderCombatState::HeavyAttacking);
	}
	else if (AttackData->bIsAirAttack)
	{
		OwnerCharacter->SetCombatState(ESpiderCombatState::AirCombo);
	}
	else
	{
		OwnerCharacter->SetCombatState(ESpiderCombatState::LightAttacking);
	}

	// Assisted Target Finding & Warping
	AActor* Target = FindBestCombatTarget(AttackData->MaxWarpDistance, AttackData->AutoTargetAngleThreshold);
	if (Target)
	{
		SetCurrentTarget(Target);
		const FVector TargetLoc = Target->GetActorLocation();
		const FVector DirToTarget = (TargetLoc - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
		
		WarpTargetLocation = TargetLoc - (DirToTarget * AttackData->AttackHitRadius);
		WarpTargetRotation = DirToTarget.Rotation();
		WarpSpeed = AttackData->WarpInterpSpeed;
		bIsWarping = true;
	}

	// Play Montage if loaded
	if (UAnimMontage* Montage = AttackData->AttackMontage.Get())
	{
		OwnerCharacter->PlayAnimMontage(Montage, AttackData->PlayRate, AttackData->MontageSectionName);
	}

	// Schedule Combo Window and Recovery
	const float WindowStartTime = AttackData->ComboWindowStart;
	const float WindowEndTime = AttackData->ComboWindowEnd;

	FTimerHandle OpenWindowTimer;
	GetWorld()->GetTimerManager().SetTimer(OpenWindowTimer, this, &USpiderCombatComponent::OpenComboWindow, WindowStartTime, false);

	FTimerHandle CloseWindowTimer;
	GetWorld()->GetTimerManager().SetTimer(CloseWindowTimer, this, &USpiderCombatComponent::CloseComboWindow, WindowEndTime, false);

	// Attack Finish / Recovery Timer
	GetWorld()->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &USpiderCombatComponent::OnAttackFinished, AttackData->RecoveryDuration + WindowEndTime, false);
}

void USpiderCombatComponent::UpdateMotionWarping(float DeltaTime)
{
	if (!OwnerCharacter.IsValid() || !bIsWarping)
	{
		return;
	}

	const FVector CurrentLoc = OwnerCharacter->GetActorLocation();
	const FVector NewLoc = FMath::VInterpTo(CurrentLoc, WarpTargetLocation, DeltaTime, WarpSpeed);
	OwnerCharacter->SetActorLocation(NewLoc, true);

	const FRotator CurrentRot = OwnerCharacter->GetActorRotation();
	const FRotator NewRot = FMath::RInterpTo(CurrentRot, WarpTargetRotation, DeltaTime, WarpSpeed);
	OwnerCharacter->SetActorRotation(FRotator(0.0f, NewRot.Yaw, 0.0f));

	if (FVector::DistSquared(CurrentLoc, WarpTargetLocation) < 250.0f)
	{
		bIsWarping = false;
	}
}

void USpiderCombatComponent::OpenComboWindow()
{
	bComboWindowOpen = true;
}

void USpiderCombatComponent::CloseComboWindow()
{
	bComboWindowOpen = false;

	if (bBufferQueued)
	{
		bBufferQueued = false;
		CurrentComboStep++;

		int32 MaxSteps = LightComboChain.Num() > 0 ? LightComboChain.Num() : 3;
		if (CurrentComboStep >= MaxSteps)
		{
			CurrentComboStep = 0;
		}

		OnComboChanged.Broadcast(CurrentComboStep + 1, MaxSteps);

		if (QueuedAttackType == ESpiderAttackType::LightAir || QueuedAttackType == ESpiderAttackType::LightGround)
		{
			TryLightAttack();
		}
		else if (QueuedAttackType == ESpiderAttackType::HeavyGround)
		{
			TryHeavyAttack();
		}
	}
}

void USpiderCombatComponent::ExecuteHitTrace()
{
	if (ActiveAttackData)
	{
		PerformHitSweep(ActiveAttackData);
	}
	else
	{
		// Default sweep when AttackData is null
		if (!OwnerCharacter.IsValid())
		{
			return;
		}

		const FVector TraceStart = OwnerCharacter->GetActorLocation();
		const FVector TraceEnd = TraceStart + (OwnerCharacter->GetActorForwardVector() * 260.0f);

		TArray<FHitResult> HitResults;
		FCollisionShape SweepSphere = FCollisionShape::MakeSphere(80.0f);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerCharacter.Get());

		GetWorld()->SweepMultiByChannel(HitResults, TraceStart, TraceEnd, FQuat::Identity, ECC_Pawn, SweepSphere, Params);

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != OwnerCharacter.Get())
			{
				if (HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
				{
					FSpiderHitReaction React;
					React.DamageTaken = 25.0f;
					React.Attacker = OwnerCharacter.Get();
					React.HitLocation = Hit.ImpactPoint;
					React.ImpactForce = OwnerCharacter->GetActorForwardVector() * 500.0f;

					IDamageable::Execute_TakeDamageCustom(HitActor, 25.0f, React);
					IDamageable::Execute_ApplyHitReaction(HitActor, React);
					ApplyHitPause(DefaultHitPauseDuration, DefaultHitPauseDilation);

					if (CameraComponent.IsValid())
					{
						CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::LightHit, OwnerCharacter->GetActorForwardVector(), 1.0f);
					}

					OnAttackHit.Broadcast(HitActor, 25.0f, false);
				}
			}
		}
	}
}

void USpiderCombatComponent::PerformHitSweep(const USpiderCombatDataAsset* AttackData)
{
	if (!AttackData || !OwnerCharacter.IsValid())
	{
		return;
	}

	const FVector TraceStart = OwnerCharacter->GetActorLocation();
	const FVector TraceEnd = TraceStart + (OwnerCharacter->GetActorForwardVector() * AttackData->AttackRange);

	TArray<FHitResult> HitResults;
	FCollisionShape SweepSphere = FCollisionShape::MakeSphere(AttackData->AttackHitRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	GetWorld()->SweepMultiByChannel(HitResults, TraceStart, TraceEnd, FQuat::Identity, ECC_Pawn, SweepSphere, Params);

	bool bHitAny = false;
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor != OwnerCharacter.Get())
		{
			ApplyAttackEffects(HitActor, Hit, AttackData);
			bHitAny = true;
		}
	}

	if (bHitAny)
	{
		ApplyHitPause(AttackData->HitPauseDuration, AttackData->HitPauseTimeDilation);
		if (CameraComponent.IsValid())
		{
			CameraComponent->ApplyCameraImpact(AttackData->CameraImpactType, OwnerCharacter->GetActorForwardVector(), AttackData->CameraImpactIntensity);
		}
	}
}

void USpiderCombatComponent::ApplyAttackEffects(AActor* HitActor, const FHitResult& HitInfo, const USpiderCombatDataAsset* AttackData)
{
	if (!HitActor || !AttackData || !OwnerCharacter.IsValid())
	{
		return;
	}

	FSpiderHitReaction Reaction;
	Reaction.DamageTaken = AttackData->BaseDamage;
	Reaction.Attacker = OwnerCharacter.Get();
	Reaction.HitLocation = HitInfo.ImpactPoint;
	Reaction.HitNormal = HitInfo.ImpactNormal;
	Reaction.ImpactForce = (OwnerCharacter->GetActorForwardVector() + AttackData->KnockbackDirectionOffset).GetSafeNormal() * AttackData->KnockbackForce;
	Reaction.bCausedStun = AttackData->StunDuration > 0.0f;
	Reaction.bKnockdown = AttackData->bCausesKnockdown;

	if (HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		IDamageable::Execute_TakeDamageCustom(HitActor, AttackData->BaseDamage, Reaction);
		IDamageable::Execute_ApplyHitReaction(HitActor, Reaction);

		if (AttackData->bCausesLauncher)
		{
			const FVector LauncherVelocity = FVector(0.0f, 0.0f, AttackData->LauncherVerticalImpulse);
			IDamageable::Execute_ApplyKnockdown(HitActor, LauncherVelocity);

			// Launch player into the air alongside the enemy for aerial juggling
			OwnerCharacter->LaunchCharacter(FVector(0.0f, 0.0f, AttackData->LauncherVerticalImpulse * 0.85f), true, true);
			bIsInAirCombat = true;
		}
		else if (AttackData->bCausesKnockdown)
		{
			IDamageable::Execute_ApplyKnockdown(HitActor, Reaction.ImpactForce);
		}
		else if (AttackData->StunDuration > 0.0f)
		{
			IDamageable::Execute_ApplyStun(HitActor, AttackData->StunDuration);
		}
	}

	OnAttackHit.Broadcast(HitActor, AttackData->BaseDamage, false);
}

void USpiderCombatComponent::OnAttackFinished()
{
	bIsAttacking = false;
	bIsWarping = false;
	bComboWindowOpen = false;

	if (!bBufferQueued)
	{
		ResetCombo();
	}

	if (OwnerCharacter.IsValid() && OwnerCharacter->GetCombatState() != ESpiderCombatState::Dodging)
	{
		OwnerCharacter->SetCombatState(ESpiderCombatState::Neutral);
	}
}

void USpiderCombatComponent::ResetCombo()
{
	CurrentComboStep = 0;
	bBufferQueued = false;
	ActiveAttackData = nullptr;
	OnComboChanged.Broadcast(0, LightComboChain.Num());
}

void USpiderCombatComponent::ApplyHitPause(float Duration, float TimeDilation)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilation);

	GetWorld()->GetTimerManager().SetTimer(HitPauseTimerHandle, this, &USpiderCombatComponent::EndHitPause, Duration * TimeDilation, false);
}

void USpiderCombatComponent::EndHitPause()
{
	if (!bPerfectDodgeActive)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}
}

void USpiderCombatComponent::TriggerPerfectDodge(AActor* ThreatActor)
{
	bPerfectDodgeActive = true;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), PerfectDodgeTimeDilation);

	if (CameraComponent.IsValid())
	{
		CameraComponent->ApplyCameraImpact(ESpiderCameraImpactType::DodgeWhoosh, FVector::UpVector, 1.8f);
	}

	OnPerfectDodgeTriggered.Broadcast(ThreatActor);
	UE_LOG(LogSpiderCombat, Log, TEXT("PERFECT DODGE executed against %s! Slow-mo engaged."), ThreatActor ? *ThreatActor->GetName() : TEXT("Threat"));

	GetWorld()->GetTimerManager().SetTimer(PerfectDodgeTimerHandle, this, &USpiderCombatComponent::EndPerfectDodgeSlowMo, PerfectDodgeSlowMoDuration * PerfectDodgeTimeDilation, false);
}

void USpiderCombatComponent::EndPerfectDodgeSlowMo()
{
	bPerfectDodgeActive = false;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

AActor* USpiderCombatComponent::FindBestCombatTarget(float MaxRange, float MaxAngleDegrees)
{
	if (!OwnerCharacter.IsValid())
	{
		return nullptr;
	}

	const FVector PlayerLoc = OwnerCharacter->GetActorLocation();
	const FVector PlayerForward = OwnerCharacter->GetActorForwardVector();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape SearchSphere = FCollisionShape::MakeSphere(MaxRange);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	GetWorld()->OverlapMultiByChannel(Overlaps, PlayerLoc, FQuat::Identity, ECC_Pawn, SearchSphere, Params);

	AActor* BestTarget = nullptr;
	float BestScore = -9999.0f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == OwnerCharacter.Get())
		{
			continue;
		}

		if (Candidate->GetClass()->ImplementsInterface(UCombatTarget::StaticClass()))
		{
			if (!ICombatTarget::Execute_IsCombatTargetValid(Candidate) || !ICombatTarget::Execute_CanBeLockedOn(Candidate))
			{
				continue;
			}
		}

		const FVector ToTarget = (Candidate->GetActorLocation() - PlayerLoc);
		const float Distance = ToTarget.Size();
		if (Distance > MaxRange)
		{
			continue;
		}

		const FVector ToTargetDir = ToTarget.GetSafeNormal2D();
		const float Dot = FVector::DotProduct(PlayerForward, ToTargetDir);
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));

		if (Angle > MaxAngleDegrees)
		{
			continue;
		}

		// Score prioritizes forward alignment and proximity
		const float DistanceScore = (1.0f - (Distance / MaxRange)) * 50.0f;
		const float AngleScore = Dot * 50.0f;
		const float TotalScore = DistanceScore + AngleScore;

		if (TotalScore > BestScore)
		{
			BestScore = TotalScore;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void USpiderCombatComponent::SetCurrentTarget(AActor* NewTarget)
{
	if (CurrentTarget.Get() != NewTarget)
	{
		if (CurrentTarget.IsValid() && CurrentTarget->GetClass()->ImplementsInterface(UCombatTarget::StaticClass()))
		{
			ICombatTarget::Execute_OnTargeted(CurrentTarget.Get(), false, OwnerCharacter.Get());
		}

		CurrentTarget = NewTarget;

		if (CurrentTarget.IsValid() && CurrentTarget->GetClass()->ImplementsInterface(UCombatTarget::StaticClass()))
		{
			ICombatTarget::Execute_OnTargeted(CurrentTarget.Get(), true, OwnerCharacter.Get());
		}

		if (CameraComponent.IsValid())
		{
			CameraComponent->SetLockOnTarget(NewTarget);
		}

		OnTargetChanged.Broadcast(NewTarget);
	}
}

bool USpiderCombatComponent::IsTargetWithinAttackCone(const AActor* TargetCandidate, float MaxDistance, float MaxAngleDegrees) const
{
	if (!TargetCandidate || !OwnerCharacter.IsValid())
	{
		return false;
	}

	const FVector PlayerLoc = OwnerCharacter->GetActorLocation();
	const FVector ToTarget = TargetCandidate->GetActorLocation() - PlayerLoc;
	if (ToTarget.Size() > MaxDistance)
	{
		return false;
	}

	const float Dot = FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), ToTarget.GetSafeNormal2D());
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));

	return Angle <= MaxAngleDegrees;
}
