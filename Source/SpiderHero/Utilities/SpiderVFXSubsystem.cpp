// Copyright SpiderHero Team. All Rights Reserved.

#include "Utilities/SpiderVFXSubsystem.h"
#include "SpiderHero.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

USpiderVFXSubsystem::USpiderVFXSubsystem()
{
}

void USpiderVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderVFXSubsystem initialized."));
}

void USpiderVFXSubsystem::SpawnWebBeamRibbon(const FVector& StartLocation, const FVector& EndLocation, float Duration)
{
	if (WebBeamSystem && GetWorld())
	{
		UNiagaraComponent* RibbonComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WebBeamSystem, StartLocation);
		if (RibbonComp)
		{
			RibbonComp->SetVectorParameter(TEXT("BeamEnd"), EndLocation);
			RibbonComp->SetFloatParameter(TEXT("Lifetime"), Duration);
		}
	}
}

void USpiderVFXSubsystem::SpawnWebImpactBurst(const FVector& Location, const FVector& Normal)
{
	if (WebImpactSystem && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WebImpactSystem, Location, Normal.Rotation());
	}
}

void USpiderVFXSubsystem::SpawnGroundSlamShockwave(const FVector& Location, float Radius)
{
	if (ShockwaveSystem && GetWorld())
	{
		UNiagaraComponent* ShockwaveComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShockwaveSystem, Location);
		if (ShockwaveComp)
		{
			ShockwaveComp->SetFloatParameter(TEXT("Radius"), Radius);
		}
	}
}

void USpiderVFXSubsystem::SpawnSpiderSenseHalo(const FVector& HeadLocation, ESpiderSenseAlertLevel AlertLevel)
{
	if (SpiderSenseSystem && GetWorld() && AlertLevel != ESpiderSenseAlertLevel::None)
	{
		UNiagaraComponent* Halo = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpiderSenseSystem, HeadLocation);
		if (Halo)
		{
			Halo->SetIntParameter(TEXT("AlertLevel"), static_cast<int32>(AlertLevel));
		}
	}
}

void USpiderVFXSubsystem::SpawnFinisherImpact(const FVector& Location, const FVector& Direction)
{
	if (FinisherImpactSystem && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FinisherImpactSystem, Location, Direction.Rotation());
	}
}
