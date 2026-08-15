// Copyright SpiderHero Team. All Rights Reserved.

#include "Utilities/SpiderScalabilityManager.h"
#include "SpiderHero.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"

USpiderScalabilityManager::USpiderScalabilityManager()
{
	CurrentLevel = ESpiderScalabilityLevel::High;
}

void USpiderScalabilityManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderScalabilityManager initialized."));
}

void USpiderScalabilityManager::ExecuteConsoleCommand(const FString& Command)
{
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), *Command);
	}
}

void USpiderScalabilityManager::ApplyScalabilityPreset(ESpiderScalabilityLevel Level)
{
	CurrentLevel = Level;

	switch (Level)
	{
	case ESpiderScalabilityLevel::Low:
		ExecuteConsoleCommand(TEXT("sg.ViewDistanceQuality 0"));
		ExecuteConsoleCommand(TEXT("sg.ShadowQuality 0"));
		ExecuteConsoleCommand(TEXT("sg.PostProcessQuality 1"));
		ExecuteConsoleCommand(TEXT("sg.TextureQuality 1"));
		ExecuteConsoleCommand(TEXT("sg.EffectsQuality 0"));
		ExecuteConsoleCommand(TEXT("r.DynamicGlobalIlluminationMethod 0")); // Screen space / none
		ExecuteConsoleCommand(TEXT("r.Shadow.Virtual.Enable 0"));
		ExecuteConsoleCommand(TEXT("r.Nanite.Enable 1"));
		break;

	case ESpiderScalabilityLevel::Medium:
		ExecuteConsoleCommand(TEXT("sg.ViewDistanceQuality 1"));
		ExecuteConsoleCommand(TEXT("sg.ShadowQuality 1"));
		ExecuteConsoleCommand(TEXT("sg.PostProcessQuality 2"));
		ExecuteConsoleCommand(TEXT("sg.TextureQuality 2"));
		ExecuteConsoleCommand(TEXT("sg.EffectsQuality 1"));
		ExecuteConsoleCommand(TEXT("r.DynamicGlobalIlluminationMethod 1")); // Lumen
		ExecuteConsoleCommand(TEXT("r.Shadow.Virtual.Enable 1"));
		ExecuteConsoleCommand(TEXT("r.Nanite.Enable 1"));
		break;

	case ESpiderScalabilityLevel::High:
		ExecuteConsoleCommand(TEXT("sg.ViewDistanceQuality 2"));
		ExecuteConsoleCommand(TEXT("sg.ShadowQuality 2"));
		ExecuteConsoleCommand(TEXT("sg.PostProcessQuality 3"));
		ExecuteConsoleCommand(TEXT("sg.TextureQuality 3"));
		ExecuteConsoleCommand(TEXT("sg.EffectsQuality 2"));
		ExecuteConsoleCommand(TEXT("r.DynamicGlobalIlluminationMethod 1")); // Lumen Full
		ExecuteConsoleCommand(TEXT("r.Shadow.Virtual.Enable 1"));
		ExecuteConsoleCommand(TEXT("r.Nanite.Enable 1"));
		break;

	case ESpiderScalabilityLevel::Ultra:
		ExecuteConsoleCommand(TEXT("sg.ViewDistanceQuality 3"));
		ExecuteConsoleCommand(TEXT("sg.ShadowQuality 3"));
		ExecuteConsoleCommand(TEXT("sg.PostProcessQuality 3"));
		ExecuteConsoleCommand(TEXT("sg.TextureQuality 3"));
		ExecuteConsoleCommand(TEXT("sg.EffectsQuality 3"));
		ExecuteConsoleCommand(TEXT("r.DynamicGlobalIlluminationMethod 1"));
		ExecuteConsoleCommand(TEXT("r.Lumen.Reflections.Temporal 1"));
		ExecuteConsoleCommand(TEXT("r.Shadow.Virtual.Enable 1"));
		ExecuteConsoleCommand(TEXT("r.Nanite.Enable 1"));
		break;
	}

	OnScalabilityPresetApplied.Broadcast(CurrentLevel);
	UE_LOG(LogSpiderHero, Log, TEXT("Scalability preset applied: %d"), static_cast<int32>(CurrentLevel));
}

void USpiderScalabilityManager::SetDynamicResolutionEnabled(bool bEnabled, float TargetFPS)
{
	ExecuteConsoleCommand(FString::Printf(TEXT("r.DynamicRes.OperationMode %d"), bEnabled ? 2 : 0));
	ExecuteConsoleCommand(FString::Printf(TEXT("r.DynamicRes.FrameTimeBudget %f"), 1000.0f / TargetFPS));
	UE_LOG(LogSpiderHero, Log, TEXT("Dynamic Resolution set: %s (Target FPS: %.1f)"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"), TargetFPS);
}
