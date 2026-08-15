// Copyright SpiderHero Team. All Rights Reserved.

#include "Core/SpiderHeroPlayerController.h"
#include "SpiderHero.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

ASpiderHeroPlayerController::ASpiderHeroPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ASpiderHeroPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetupDefaultInputContext();
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderHeroPlayerController::BeginPlay initialized."));
}

void ASpiderHeroPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetupDefaultInputContext();
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderHeroPlayerController possessed pawn: %s"), InPawn ? *InPawn->GetName() : TEXT("None"));
}

void ASpiderHeroPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderHeroPlayerController unpossessed pawn."));
}

void ASpiderHeroPlayerController::SetupDefaultInputContext()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				OnInputContextChanged.Broadcast(DefaultMappingContext);
				UE_LOG(LogSpiderHero, Log, TEXT("Added DefaultMappingContext to EnhancedInput subsystem."));
			}
		}
	}
}

void ASpiderHeroPlayerController::AddMappingContext(const UInputMappingContext* ContextToAdd, int32 Priority)
{
	if (!ContextToAdd)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->AddMappingContext(ContextToAdd, Priority);
			OnInputContextChanged.Broadcast(ContextToAdd);
			UE_LOG(LogSpiderHero, Log, TEXT("Added Context %s with Priority %d"), *ContextToAdd->GetName(), Priority);
		}
	}
}

void ASpiderHeroPlayerController::RemoveMappingContext(const UInputMappingContext* ContextToRemove)
{
	if (!ContextToRemove)
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->RemoveMappingContext(ContextToRemove);
			UE_LOG(LogSpiderHero, Log, TEXT("Removed Context %s"), *ContextToRemove->GetName());
		}
	}
}

void ASpiderHeroPlayerController::NotifySpiderSenseThreat(const FSpiderThreatInfo& ThreatInfo)
{
	OnSpiderSenseTriggered.Broadcast(ThreatInfo);
	UE_LOG(LogSpiderCombat, Log, TEXT("Spider-Sense Alert Level: %d from Threat Source: %s"), 
		static_cast<int32>(ThreatInfo.AlertLevel), 
		ThreatInfo.ThreatSource.IsValid() ? *ThreatInfo.ThreatSource->GetName() : TEXT("None"));
}
