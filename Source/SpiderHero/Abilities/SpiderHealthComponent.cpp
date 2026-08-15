// Copyright SpiderHero Team. All Rights Reserved.

#include "Abilities/SpiderHealthComponent.h"
#include "SpiderHero.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USpiderHealthComponent::USpiderHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	MaxHealth = 200.0f;
	CurrentHealth = MaxHealth;
	DamageMitigationPercent = 0.0f;
	bIsInvulnerable = false;
	bIsDead = false;

	bEnableRegeneration = true;
	RegenRate = 12.0f; // 12 HP per second
	RegenDelay = 4.0f; // 4 seconds of no damage before regen kicks in
	TimeSinceLastDamage = 0.0f;
}

void USpiderHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	bIsDead = false;
	TimeSinceLastDamage = RegenDelay; // Ready to regen if damaged initially
}

void USpiderHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDead)
	{
		return;
	}

	TimeSinceLastDamage += DeltaTime;

	// Health regeneration logic
	if (bEnableRegeneration && TimeSinceLastDamage >= RegenDelay && CurrentHealth < MaxHealth)
	{
		float OldHealth = CurrentHealth;
		CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + (RegenRate * DeltaTime));
		float Delta = CurrentHealth - OldHealth;

		if (Delta > 0.0f)
		{
			OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, Delta, GetOwner());
		}
	}
}

float USpiderHealthComponent::ApplyDamage(float Amount, AActor* DamageCauser, const FHitResult& HitInfo)
{
	if (bIsDead || Amount <= 0.0f)
	{
		return 0.0f;
	}

	if (bIsInvulnerable)
	{
		OnDamageBlocked.Broadcast(Amount);
		UE_LOG(LogSpiderCombat, Log, TEXT("%s blocked %.1f damage due to invulnerability."), *GetOwner()->GetName(), Amount);
		return 0.0f;
	}

	// Calculate mitigated damage
	float MitigatedDamage = Amount * (1.0f - DamageMitigationPercent);
	float ActualDamage = FMath::Min(CurrentHealth, MitigatedDamage);

	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
	TimeSinceLastDamage = 0.0f;

	UE_LOG(LogSpiderCombat, Log, TEXT("%s took %.1f damage (Raw: %.1f). Health now: %.1f / %.1f"),
		*GetOwner()->GetName(), ActualDamage, Amount, CurrentHealth, MaxHealth);

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, -ActualDamage, DamageCauser);

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(DamageCauser);
		UE_LOG(LogSpiderHero, Warning, TEXT("%s has died!"), *GetOwner()->GetName());
	}

	return ActualDamage;
}

float USpiderHealthComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.0f || CurrentHealth >= MaxHealth)
	{
		return 0.0f;
	}

	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	float ActualHeal = CurrentHealth - OldHealth;

	if (ActualHeal > 0.0f)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, ActualHeal, GetOwner());
		UE_LOG(LogSpiderHero, Log, TEXT("%s healed %.1f HP. Health: %.1f / %.1f"),
			*GetOwner()->GetName(), ActualHeal, CurrentHealth, MaxHealth);
	}

	return ActualHeal;
}

void USpiderHealthComponent::GrantInvulnerability(float Duration)
{
	if (Duration <= 0.0f)
	{
		return;
	}

	SetInvulnerable(true, Duration);
}

void USpiderHealthComponent::SetInvulnerable(bool bInvulnerableState, float Duration)
{
	bIsInvulnerable = bInvulnerableState;
	OnInvulnerabilityChanged.Broadcast(bIsInvulnerable, Duration);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IFrameTimerHandle);

		if (bIsInvulnerable && Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				IFrameTimerHandle,
				this,
				&USpiderHealthComponent::ClearInvulnerability,
				Duration,
				false
			);
		}
	}
}

void USpiderHealthComponent::ClearInvulnerability()
{
	if (bIsInvulnerable)
	{
		bIsInvulnerable = false;
		OnInvulnerabilityChanged.Broadcast(false, 0.0f);
	}
}

void USpiderHealthComponent::SetMaxHealth(float NewMaxHealth, bool bHealToMax)
{
	MaxHealth = FMath::Max(1.0f, NewMaxHealth);
	if (bHealToMax)
	{
		CurrentHealth = MaxHealth;
	}
	else
	{
		CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
	}
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, 0.0f, GetOwner());
}
