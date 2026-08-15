// Copyright SpiderHero Team. All Rights Reserved.

#include "AI/SpiderEnemyBase.h"
#include "SpiderHero.h"
#include "Abilities/SpiderHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASpiderEnemyBase::ASpiderEnemyBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule setup
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 90.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	// Movement Defaults
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
	GetCharacterMovement()->GravityScale = 1.6f;

	// Health Component
	HealthComponent = CreateDefaultSubobject<USpiderHealthComponent>(TEXT("HealthComponent"));

	// Configuration Defaults
	ThreatLevel = 1;
	MaxHealth = 100.0f;
	CurrentHealth = 100.0f;
	MaxWebGauge = 100.0f;
	WebGauge = 0.0f;
	WebGaugeDecayRate = 10.0f;
	DefaultWebBoundDuration = 6.0f;
	StaggerDurationDefault = 0.6f;
	KnockdownRecoveryDelay = 1.8f;

	CurrentAIState = ESpiderEnemyAIState::Idle;
	bHasAttackToken = false;
	AssignedSlotIndex = -1;
	bIsTargetedByPlayer = false;
}

void ASpiderEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	if (HealthComponent)
	{
		HealthComponent->SetMaxHealth(MaxHealth, true);
	}

	UE_LOG(LogSpiderAI, Log, TEXT("SpiderEnemyBase spawned: %s"), *GetName());
}

void ASpiderEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Decay Web Gauge when not web-bound
	if (WebGauge > 0.0f && CurrentAIState != ESpiderEnemyAIState::WebBound && CurrentAIState != ESpiderEnemyAIState::WallPinned)
	{
		WebGauge = FMath::Max(0.0f, WebGauge - WebGaugeDecayRate * DeltaSeconds);
		OnWebGaugeChanged.Broadcast(WebGauge, MaxWebGauge);
	}
}

void ASpiderEnemyBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (CurrentAIState == ESpiderEnemyAIState::AirLaunched)
	{
		// Hard impact on landing after air juggle
		SetAIState(ESpiderEnemyAIState::KnockedDown);
		GetWorld()->GetTimerManager().SetTimer(StateRecoveryTimerHandle, this, &ASpiderEnemyBase::RecoverFromKnockdown, KnockdownRecoveryDelay, false);
	}
}

float ASpiderEnemyBase::TakeDamageCustom_Implementation(float DamageAmount, const FSpiderHitReaction& HitReaction)
{
	if (CurrentAIState == ESpiderEnemyAIState::Dead)
	{
		return 0.0f;
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

	if (CurrentHealth <= 0.0f)
	{
		HandleDeath(HitReaction.Attacker.Get());
		return DamageAmount;
	}

	// Apply Hit Reactions
	if (HitReaction.bKnockdown)
	{
		ApplyKnockdown_Implementation(HitReaction.ImpactForce);
	}
	else if (HitReaction.bCausedStun)
	{
		ApplyStun_Implementation(StaggerDurationDefault);
	}
	else
	{
		ApplyHitReaction_Implementation(HitReaction);
	}

	return DamageAmount;
}

float ASpiderEnemyBase::GetHealthPercent_Implementation() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void ASpiderEnemyBase::ApplyStun_Implementation(float Duration)
{
	if (CurrentAIState == ESpiderEnemyAIState::Dead || CurrentAIState == ESpiderEnemyAIState::WallPinned)
	{
		return;
	}

	SetAIState(ESpiderEnemyAIState::Staggered);
	GetCharacterMovement()->StopMovementImmediately();

	GetWorld()->GetTimerManager().SetTimer(StateRecoveryTimerHandle, this, &ASpiderEnemyBase::RecoverFromStagger, Duration, false);
}

void ASpiderEnemyBase::ApplyKnockdown_Implementation(const FVector& LaunchVelocity)
{
	if (CurrentAIState == ESpiderEnemyAIState::Dead || CurrentAIState == ESpiderEnemyAIState::WallPinned)
	{
		return;
	}

	if (LaunchVelocity.Z > 350.0f)
	{
		// Airborne launcher
		SetAIState(ESpiderEnemyAIState::AirLaunched);
		LaunchCharacter(LaunchVelocity, true, true);
	}
	else
	{
		SetAIState(ESpiderEnemyAIState::KnockedDown);
		LaunchCharacter(LaunchVelocity, true, false);

		GetWorld()->GetTimerManager().SetTimer(StateRecoveryTimerHandle, this, &ASpiderEnemyBase::RecoverFromKnockdown, KnockdownRecoveryDelay, false);
	}
}

void ASpiderEnemyBase::ApplyHitReaction_Implementation(const FSpiderHitReaction& HitReaction)
{
	if (CurrentAIState == ESpiderEnemyAIState::Dead || CurrentAIState == ESpiderEnemyAIState::WallPinned || CurrentAIState == ESpiderEnemyAIState::WebBound)
	{
		return;
	}

	SetAIState(ESpiderEnemyAIState::Staggered);
	LaunchCharacter(HitReaction.ImpactForce * 0.3f, false, false);

	GetWorld()->GetTimerManager().SetTimer(StateRecoveryTimerHandle, this, &ASpiderEnemyBase::RecoverFromStagger, StaggerDurationDefault, false);
}

FVector ASpiderEnemyBase::GetTargetLocation_Implementation() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, 30.0f);
}

FVector ASpiderEnemyBase::GetCombatSocketLocation_Implementation(FName SocketName) const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
		{
			return MeshComp->GetSocketLocation(SocketName);
		}
		if (MeshComp->DoesSocketExist(TEXT("spine_03")))
		{
			return MeshComp->GetSocketLocation(TEXT("spine_03"));
		}
	}
	return GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
}

void ASpiderEnemyBase::OnTargeted_Implementation(bool bIsTargeted, AActor* TargetingActor)
{
	bIsTargetedByPlayer = bIsTargeted;
}

float ASpiderEnemyBase::GetTargetRadius_Implementation() const
{
	return GetCapsuleComponent()->GetScaledCapsuleRadius();
}

void ASpiderEnemyBase::OnWebAttached_Implementation(AActor* AttachingActor, const FVector& AttachPoint)
{
	UE_LOG(LogSpiderWeb, Verbose, TEXT("Web attached to enemy %s"), *GetName());
}

void ASpiderEnemyBase::OnWebPulled_Implementation(AActor* PullingActor, const FVector& PullDirection, float Force)
{
	LaunchCharacter(PullDirection * Force, true, true);
	SetAIState(ESpiderEnemyAIState::Staggered);

	GetWorld()->GetTimerManager().SetTimer(StateRecoveryTimerHandle, this, &ASpiderEnemyBase::RecoverFromStagger, 0.8f, false);
}

void ASpiderEnemyBase::OnWebBound_Implementation(float Duration, AActor* InstigatorActor)
{
	SetAIState(ESpiderEnemyAIState::WebBound);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetWorld()->GetTimerManager().SetTimer(StateRecoveryTimerHandle, this, &ASpiderEnemyBase::RecoverFromWebBound, Duration, false);
}

void ASpiderEnemyBase::ApplyWebGauge_Implementation(float Amount, AActor* InstigatorActor)
{
	if (CurrentAIState == ESpiderEnemyAIState::Dead || CurrentAIState == ESpiderEnemyAIState::WallPinned)
	{
		return;
	}

	WebGauge = FMath::Clamp(WebGauge + Amount, 0.0f, MaxWebGauge);
	OnWebGaugeChanged.Broadcast(WebGauge, MaxWebGauge);

	if (WebGauge >= MaxWebGauge && CurrentAIState != ESpiderEnemyAIState::WebBound)
	{
		OnWebBound_Implementation(DefaultWebBoundDuration, InstigatorActor);
	}
}

FVector ASpiderEnemyBase::GetWebAnchorPoint_Implementation() const
{
	return GetCombatSocketLocation_Implementation(TEXT("spine_03"));
}

bool ASpiderEnemyBase::TryPinToSurface_Implementation(const FVector& SurfaceLocation, const FVector& SurfaceNormal)
{
	if (CurrentAIState == ESpiderEnemyAIState::Dead)
	{
		return false;
	}

	SetAIState(ESpiderEnemyAIState::WallPinned);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Snap against wall facing outwards
	SetActorLocation(SurfaceLocation + (SurfaceNormal * 25.0f), false);
	SetActorRotation((-SurfaceNormal).Rotation());

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// Permanently or long-term incapacitated
	GetWorld()->GetTimerManager().ClearTimer(StateRecoveryTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(StateRecoveryTimerHandle, this, &ASpiderEnemyBase::RecoverFromWebBound, 15.0f, false);

	return true;
}

void ASpiderEnemyBase::SetAIState(ESpiderEnemyAIState NewState)
{
	if (CurrentAIState != NewState)
	{
		const ESpiderEnemyAIState PrevState = CurrentAIState;
		CurrentAIState = NewState;
		OnAIStateChanged.Broadcast(PrevState, CurrentAIState);
		UE_LOG(LogSpiderAI, Verbose, TEXT("Enemy %s AI State: %d -> %d"), *GetName(), static_cast<int32>(PrevState), static_cast<int32>(NewState));
	}
}

void ASpiderEnemyBase::RecoverFromStagger()
{
	if (CurrentAIState == ESpiderEnemyAIState::Staggered)
	{
		SetAIState(ESpiderEnemyAIState::Approaching);
	}
}

void ASpiderEnemyBase::RecoverFromKnockdown()
{
	if (CurrentAIState == ESpiderEnemyAIState::KnockedDown)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		SetAIState(ESpiderEnemyAIState::Approaching);
	}
}

void ASpiderEnemyBase::RecoverFromWebBound()
{
	if (CurrentAIState == ESpiderEnemyAIState::WebBound || CurrentAIState == ESpiderEnemyAIState::WallPinned)
	{
		WebGauge = 0.0f;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		SetAIState(ESpiderEnemyAIState::Approaching);
		OnWebGaugeChanged.Broadcast(0.0f, MaxWebGauge);
	}
}

void ASpiderEnemyBase::HandleDeath(AActor* Killer)
{
	SetAIState(ESpiderEnemyAIState::Dead);
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	EnterRagdoll();

	// Schedule cleanup
	GetWorld()->GetTimerManager().SetTimer(RagdollCleanupTimerHandle, this, &ASpiderEnemyBase::CleanupCorpse, 6.0f, false);
}

void ASpiderEnemyBase::EnterRagdoll()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetAllPhysicsLinearVelocity(FVector(0.0f, 0.0f, 100.0f));
	}
}

void ASpiderEnemyBase::CleanupCorpse()
{
	Destroy();
}
