// Copyright SpiderHero Team. All Rights Reserved.

#include "Core/SpiderHeroGameState.h"
#include "SpiderHero.h"
#include "TimerManager.h"
#include "Engine/World.h"

ASpiderHeroGameState::ASpiderHeroGameState()
{
	CityCrimeRate = 50.0f;
	HeroReputation = 100;
	CurrentComboCount = 0;
	CurrentComboMultiplier = 1.0f;
	HeatLevel = 0;
	ComboTimeoutDuration = 3.5f;
	MaxComboMultiplier = 4.0f;
}

void ASpiderHeroGameState::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderHeroGameState initialized. Crime Rate: %.1f%%, Rep: %d"), CityCrimeRate, HeroReputation);
}

void ASpiderHeroGameState::ModifyCrimeRate(float Delta)
{
	SetCrimeRate(CityCrimeRate + Delta);
}

void ASpiderHeroGameState::SetCrimeRate(float NewRate)
{
	float ClampedRate = FMath::Clamp(NewRate, 0.0f, 100.0f);
	if (!FMath::IsNearlyEqual(CityCrimeRate, ClampedRate))
	{
		CityCrimeRate = ClampedRate;
		OnCrimeRateChanged.Broadcast(CityCrimeRate);
		UE_LOG(LogSpiderHero, Log, TEXT("City Crime Rate updated to: %.1f%%"), CityCrimeRate);
	}
}

void ASpiderHeroGameState::AddReputation(int32 Amount)
{
	HeroReputation = FMath::Max(0, HeroReputation + Amount);
	OnHeroReputationChanged.Broadcast(HeroReputation);
	UE_LOG(LogSpiderHero, Log, TEXT("Hero Reputation updated: %d (Delta: %+d)"), HeroReputation, Amount);
}

void ASpiderHeroGameState::RegisterComboHit()
{
	CurrentComboCount++;
	
	// Multiplier scales every 5 hits up to MaxComboMultiplier
	float CalculatedMultiplier = 1.0f + FMath::FloorToFloat(CurrentComboCount / 5.0f) * 0.25f;
	CurrentComboMultiplier = FMath::Min(CalculatedMultiplier, MaxComboMultiplier);

	OnComboScoreChanged.Broadcast(CurrentComboCount, CurrentComboMultiplier);

	// Reset timeout timer
	GetWorld()->GetTimerManager().ClearTimer(ComboTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		ComboTimerHandle,
		this,
		&ASpiderHeroGameState::HandleComboTimeout,
		ComboTimeoutDuration,
		false
	);
}

void ASpiderHeroGameState::ResetCombo()
{
	if (CurrentComboCount > 0)
	{
		CurrentComboCount = 0;
		CurrentComboMultiplier = 1.0f;
		GetWorld()->GetTimerManager().ClearTimer(ComboTimerHandle);
		OnComboScoreChanged.Broadcast(CurrentComboCount, CurrentComboMultiplier);
	}
}

void ASpiderHeroGameState::HandleComboTimeout()
{
	ResetCombo();
}

void ASpiderHeroGameState::SetHeatLevel(int32 InHeatLevel)
{
	int32 Clamped = FMath::Clamp(InHeatLevel, 0, 5);
	if (HeatLevel != Clamped)
	{
		HeatLevel = Clamped;
		OnHeatLevelChanged.Broadcast(HeatLevel);
		UE_LOG(LogSpiderHero, Log, TEXT("Heat Level updated to: %d"), HeatLevel);
	}
}
