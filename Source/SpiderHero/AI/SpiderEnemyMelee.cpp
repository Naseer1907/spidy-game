// Copyright SpiderHero Team. All Rights Reserved.

#include "AI/SpiderEnemyMelee.h"
#include "SpiderHero.h"
#include "Combat/Interfaces/IDamageable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"

ASpiderEnemyMelee::ASpiderEnemyMelee(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ThreatLevel = 1;
	MaxHealth = 80.0f;
	CurrentHealth = 80.0f;

	AttackDamage = 15.0f;
	AttackRange = 180.0f;
	AttackRadius = 60.0f;
	TelegraphWindupDuration = 0.85f;
	AttackCooldownDuration = 1.5f;
	KnockbackForce = 450.0f;

	AttackSectionName = NAME_None;
}

void ASpiderEnemyMelee::BeginPlay()
{
	Super::BeginPlay();
}

void ASpiderEnemyMelee::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

bool ASpiderEnemyMelee::StartAttackTelegraph(float CustomWindup)
{
	if (CurrentAIState != ESpiderEnemyAIState::Approaching && 
		CurrentAIState != ESpiderEnemyAIState::Circling && 
		CurrentAIState != ESpiderEnemyAIState::Idle)
	{
		return false;
	}

	SetAIState(ESpiderEnemyAIState::PreparingAttack);
	GetCharacterMovement()->StopMovementImmediately();

	const float WindupTime = CustomWindup > 0.0f ? CustomWindup : TelegraphWindupDuration;
	OnTelegraphAttackStarted.Broadcast(WindupTime);

	if (UAnimMontage* Montage = MeleeAttackMontage.Get())
	{
		PlayAnimMontage(Montage, 1.0f, AttackSectionName);
	}

	GetWorld()->GetTimerManager().SetTimer(TelegraphTimerHandle, this, &ASpiderEnemyMelee::ExecuteMeleeStrike, WindupTime, false);
	return true;
}

void ASpiderEnemyMelee::ExecuteMeleeStrike()
{
	if (CurrentAIState != ESpiderEnemyAIState::PreparingAttack)
	{
		return;
	}

	SetAIState(ESpiderEnemyAIState::Attacking);
	OnMeleeAttackExecuted.Broadcast();

	const FVector TraceStart = GetActorLocation();
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * AttackRange);

	TArray<FHitResult> Hits;
	FCollisionShape Shape = FCollisionShape::MakeSphere(AttackRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->SweepMultiByChannel(Hits, TraceStart, TraceEnd, FQuat::Identity, ECC_Pawn, Shape, Params);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor != this)
		{
			if (HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
			{
				FSpiderHitReaction React;
				React.DamageTaken = AttackDamage;
				React.Attacker = this;
				React.HitLocation = Hit.ImpactPoint;
				React.ImpactForce = GetActorForwardVector() * KnockbackForce;

				IDamageable::Execute_TakeDamageCustom(HitActor, AttackDamage, React);
				IDamageable::Execute_ApplyHitReaction(HitActor, React);
			}
			else
			{
				FDamageEvent DmgEvent;
				HitActor->TakeDamage(AttackDamage, DmgEvent, GetController(), this);
			}
		}
	}

	// Post attack cooldown window
	GetWorld()->GetTimerManager().SetTimer(AttackCooldownTimerHandle, this, &ASpiderEnemyMelee::FinishAttackCooldown, AttackCooldownDuration, false);
}

void ASpiderEnemyMelee::AbortAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(TelegraphTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);

	if (CurrentAIState == ESpiderEnemyAIState::PreparingAttack || CurrentAIState == ESpiderEnemyAIState::Attacking)
	{
		SetAIState(ESpiderEnemyAIState::Approaching);
	}
}

void ASpiderEnemyMelee::FinishAttackCooldown()
{
	if (CurrentAIState == ESpiderEnemyAIState::Attacking)
	{
		SetAIState(ESpiderEnemyAIState::Circling);
	}
}
