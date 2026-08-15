// Copyright SpiderHero Team. All Rights Reserved.

#include "Abilities/SpiderStaminaComponent.h"
#include "SpiderHero.h"

USpiderStaminaComponent::USpiderStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	MaxStamina = 100.0f;
	CurrentStamina = MaxStamina;
	StaminaRegenRate = 25.0f; // 25 stamina per sec
	StaminaRegenDelay = 1.0f; // 1 second delay after consumption
	FatigueThreshold = 15.0f;
	bIsFatigued = false;
	bRegenPaused = false;
	TimeSinceLastConsumption = 0.0f;
}

void USpiderStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
	bIsFatigued = false;
	TimeSinceLastConsumption = StaminaRegenDelay;
}

void USpiderStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastConsumption += DeltaTime;

	if (!bRegenPaused && TimeSinceLastConsumption >= StaminaRegenDelay && CurrentStamina < MaxStamina)
	{
		float OldStamina = CurrentStamina;
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));
		float Delta = CurrentStamina - OldStamina;

		if (Delta > 0.0f)
		{
			UpdateFatigueState();
			OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, Delta, bIsFatigued);
		}
	}
}

bool USpiderStaminaComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return true;
	}

	if (CurrentStamina < Amount)
	{
		UE_LOG(LogSpiderHero, Verbose, TEXT("Not enough stamina to consume %.1f (Current: %.1f)"), Amount, CurrentStamina);
		return false;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);
	TimeSinceLastConsumption = 0.0f;

	UpdateFatigueState();
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, -Amount, bIsFatigued);

	return true;
}

bool USpiderStaminaComponent::HasEnoughStamina(float RequiredAmount) const
{
	return CurrentStamina >= RequiredAmount;
}

void USpiderStaminaComponent::RestoreStamina(float Amount)
{
	if (Amount <= 0.0f || CurrentStamina >= MaxStamina)
	{
		return;
	}

	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
	float Delta = CurrentStamina - OldStamina;

	if (Delta > 0.0f)
	{
		UpdateFatigueState();
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, Delta, bIsFatigued);
	}
}

void USpiderStaminaComponent::ResetStamina()
{
	CurrentStamina = MaxStamina;
	UpdateFatigueState();
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, 0.0f, bIsFatigued);
}

void USpiderStaminaComponent::SetMaxStamina(float NewMaxStamina, bool bFillToMax)
{
	MaxStamina = FMath::Max(10.0f, NewMaxStamina);
	if (bFillToMax)
	{
		CurrentStamina = MaxStamina;
	}
	else
	{
		CurrentStamina = FMath::Min(CurrentStamina, MaxStamina);
	}
	UpdateFatigueState();
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, 0.0f, bIsFatigued);
}

void USpiderStaminaComponent::UpdateFatigueState()
{
	bool bNewFatigue = (CurrentStamina <= FatigueThreshold);
	if (bIsFatigued != bNewFatigue)
	{
		bIsFatigued = bNewFatigue;
		OnFatigueStateChanged.Broadcast(bIsFatigued);
		UE_LOG(LogSpiderHero, Log, TEXT("Spider Fatigue state changed: %s"), bIsFatigued ? TEXT("FATIGUED") : TEXT("RESTORED"));
	}
}
