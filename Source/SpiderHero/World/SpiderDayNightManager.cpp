// Copyright SpiderHero Team. All Rights Reserved.

#include "World/SpiderDayNightManager.h"
#include "SpiderHero.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ASpiderDayNightManager::ASpiderDayNightManager()
{
	PrimaryActorTick.bCanEverTick = true;

	InitialTimeHours = 14.0f; // 2:00 PM
	TimeSpeedMultiplier = 60.0f; // 24 mins for a full day
	bPauseTimeProgression = false;

	SunLatitudeDegrees = 45.0f;
	SunMaxIntensity = 10.0f;
	MoonMaxIntensity = 1.2f;

	CurrentTimeHours = InitialTimeHours;
	CurrentTimePhase = ESpiderTimeOfDay::Noon;
	bIsNight = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
}

void ASpiderDayNightManager::BeginPlay()
{
	Super::BeginPlay();

	FindSceneReferences();
	CurrentTimeHours = InitialTimeHours;
	EvaluateTimePhase();
	UpdateCelestialBodies();
	UpdateMaterialParameters();

	UE_LOG(LogSpiderWorld, Log, TEXT("SpiderDayNightManager started at %0.2f hours."), CurrentTimeHours);
}

void ASpiderDayNightManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bPauseTimeProgression)
	{
		const float HoursElapsed = (DeltaSeconds / 3600.0f) * TimeSpeedMultiplier;
		CurrentTimeHours = FMath::Fmod(CurrentTimeHours + HoursElapsed, 24.0f);
	}

	EvaluateTimePhase();
	UpdateCelestialBodies();
	UpdateMaterialParameters();
}

void ASpiderDayNightManager::SetTimeOfDay(float NewTimeInHours)
{
	CurrentTimeHours = FMath::Clamp(NewTimeInHours, 0.0f, 24.0f);
	EvaluateTimePhase();
	UpdateCelestialBodies();
	UpdateMaterialParameters();
}

void ASpiderDayNightManager::EvaluateTimePhase()
{
	ESpiderTimeOfDay NewPhase = ESpiderTimeOfDay::Noon;
	bool bNewNight = false;

	if (CurrentTimeHours >= 5.0f && CurrentTimeHours < 7.0f)
	{
		NewPhase = ESpiderTimeOfDay::Dawn;
	}
	else if (CurrentTimeHours >= 7.0f && CurrentTimeHours < 11.5f)
	{
		NewPhase = ESpiderTimeOfDay::Morning;
	}
	else if (CurrentTimeHours >= 11.5f && CurrentTimeHours < 15.5f)
	{
		NewPhase = ESpiderTimeOfDay::Noon;
	}
	else if (CurrentTimeHours >= 15.5f && CurrentTimeHours < 18.0f)
	{
		NewPhase = ESpiderTimeOfDay::GoldenHour;
	}
	else if (CurrentTimeHours >= 18.0f && CurrentTimeHours < 19.5f)
	{
		NewPhase = ESpiderTimeOfDay::Sunset;
	}
	else if (CurrentTimeHours >= 19.5f && CurrentTimeHours < 21.0f)
	{
		NewPhase = ESpiderTimeOfDay::Dusk;
		bNewNight = true;
	}
	else
	{
		NewPhase = ESpiderTimeOfDay::Night;
		bNewNight = true;
	}

	if (NewPhase != CurrentTimePhase)
	{
		CurrentTimePhase = NewPhase;
		OnTimeOfDayChanged.Broadcast(CurrentTimePhase, CurrentTimeHours);
	}

	if (bNewNight != bIsNight)
	{
		bIsNight = bNewNight;
		OnNightStateToggled.Broadcast(bIsNight);
		UE_LOG(LogSpiderWorld, Log, TEXT("Night state changed: %s (Hour: %0.2f)"), bIsNight ? TEXT("NIGHT") : TEXT("DAY"), CurrentTimeHours);
	}
}

void ASpiderDayNightManager::UpdateCelestialBodies()
{
	// Solar elevation calculations: 6:00 is sunrise, 12:00 is zenith, 18:00 is sunset
	const float SolarProgress = (CurrentTimeHours - 6.0f) / 12.0f; // 0 at sunrise, 1 at sunset
	const float SunPitch = FMath::Sin(SolarProgress * PI) * 75.0f;
	const float SunYaw = (CurrentTimeHours / 24.0f) * 360.0f - 90.0f;

	const FRotator SunRotation(SunPitch, SunYaw, 0.0f);
	const FRotator MoonRotation(-SunPitch, SunYaw + 180.0f, 0.0f);

	// Update Sun
	if (SunLightActor && SunLightActor->GetLightComponent())
	{
		UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(SunLightActor->GetLightComponent());
		SunLightActor->SetActorRotation(SunRotation);

		const bool bSunVisible = (CurrentTimeHours >= 5.5f && CurrentTimeHours <= 19.5f);
		SunComp->SetVisibility(bSunVisible);

		if (bSunVisible)
		{
			const float SunAltitudeFactor = FMath::Clamp(FMath::Sin(SolarProgress * PI), 0.0f, 1.0f);
			SunComp->SetIntensity(SunMaxIntensity * SunAltitudeFactor);

			// Golden warm hues at dawn and sunset
			if (CurrentTimePhase == ESpiderTimeOfDay::Dawn || CurrentTimePhase == ESpiderTimeOfDay::Sunset)
			{
				SunComp->SetLightColor(FLinearColor(1.0f, 0.65f, 0.35f));
			}
			else if (CurrentTimePhase == ESpiderTimeOfDay::GoldenHour)
			{
				SunComp->SetLightColor(FLinearColor(1.0f, 0.85f, 0.6f));
			}
			else
			{
				SunComp->SetLightColor(FLinearColor(1.0f, 0.98f, 0.92f));
			}
		}
	}

	// Update Moon
	if (MoonLightActor && MoonLightActor->GetLightComponent())
	{
		UDirectionalLightComponent* MoonComp = Cast<UDirectionalLightComponent>(MoonLightActor->GetLightComponent());
		MoonLightActor->SetActorRotation(MoonRotation);

		const bool bMoonVisible = (CurrentTimeHours < 6.0f || CurrentTimeHours > 19.0f);
		MoonComp->SetVisibility(bMoonVisible);

		if (bMoonVisible)
		{
			MoonComp->SetIntensity(MoonMaxIntensity);
			MoonComp->SetLightColor(FLinearColor(0.6f, 0.75f, 1.0f)); // Cool moonlight
		}
	}

	// Recapture SkyLight periodically
	if (SkyLightActor && SkyLightActor->GetLightComponent())
	{
		SkyLightActor->GetLightComponent()->SetRealTimeCapture(true);
	}
}

void ASpiderDayNightManager::UpdateMaterialParameters()
{
	if (!CityMaterialParameters)
	{
		return;
	}

	UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(CityMaterialParameters);
	if (!MPCInstance)
	{
		return;
	}

	// Window Emissive scalar: Fades in between 18.0 and 20.0, stays on until 06.0
	float WindowEmissive = 0.0f;
	float StreetLight = 0.0f;

	if (CurrentTimeHours >= 18.0f && CurrentTimeHours < 20.0f)
	{
		const float Alpha = (CurrentTimeHours - 18.0f) / 2.0f;
		WindowEmissive = FMath::Lerp(0.0f, 2.5f, Alpha);
		StreetLight = FMath::Lerp(0.0f, 1.0f, Alpha);
	}
	else if (CurrentTimeHours >= 20.0f || CurrentTimeHours < 5.5f)
	{
		WindowEmissive = 2.5f;
		StreetLight = 1.0f;
	}
	else if (CurrentTimeHours >= 5.5f && CurrentTimeHours < 6.5f)
	{
		const float Alpha = 1.0f - ((CurrentTimeHours - 5.5f) / 1.0f);
		WindowEmissive = FMath::Lerp(0.0f, 2.5f, Alpha);
		StreetLight = FMath::Lerp(0.0f, 1.0f, Alpha);
	}

	MPCInstance->SetScalarParameterValue(TEXT("WindowEmissiveIntensity"), WindowEmissive);
	MPCInstance->SetScalarParameterValue(TEXT("StreetLightIntensity"), StreetLight);
	MPCInstance->SetScalarParameterValue(TEXT("TimeOfDayNormalized"), CurrentTimeHours / 24.0f);
}

void ASpiderDayNightManager::FindSceneReferences()
{
	if (!SunLightActor)
	{
		TArray<AActor*> FoundLights;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
		if (FoundLights.Num() > 0)
		{
			SunLightActor = Cast<ADirectionalLight>(FoundLights[0]);
		}
		if (FoundLights.Num() > 1)
		{
			MoonLightActor = Cast<ADirectionalLight>(FoundLights[1]);
		}
	}

	if (!SkyLightActor)
	{
		SkyLightActor = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ASkyLight::StaticClass()));
	}

	if (!PostProcessVolumeActor)
	{
		PostProcessVolumeActor = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass()));
	}
}
