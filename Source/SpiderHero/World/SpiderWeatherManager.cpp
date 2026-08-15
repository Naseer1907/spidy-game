// Copyright SpiderHero Team. All Rights Reserved.

#include "World/SpiderWeatherManager.h"
#include "SpiderHero.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ASpiderWeatherManager::ASpiderWeatherManager()
{
	PrimaryActorTick.bCanEverTick = true;

	InitialWeatherState = ESpiderWeatherState::Clear;
	CurrentWeatherState = ESpiderWeatherState::Clear;
	TargetWeatherState = ESpiderWeatherState::Clear;

	bEnableRandomWeatherCycle = true;
	MinWeatherDuration = 300.0f; // 5 mins
	MaxWeatherDuration = 900.0f; // 15 mins

	TransitionProgress = 1.0f;
	TotalTransitionTime = 1.0f;
	bIsTransitioning = false;

	CurrentWetness = 0.0f;
	CurrentPuddleLevel = 0.0f;
	TimeInCurrentWeather = 0.0f;
	WeatherStateDuration = 600.0f;
	TimeSinceLastThunder = 0.0f;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	RainNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainNiagaraComponent"));
	RainNiagaraComponent->SetupAttachment(RootComponent);
	RainNiagaraComponent->bAutoActivate = false;

	RainAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RainAudioComponent"));
	RainAudioComponent->SetupAttachment(RootComponent);
	RainAudioComponent->bAutoActivate = false;

	WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudioComponent"));
	WindAudioComponent->SetupAttachment(RootComponent);
	WindAudioComponent->bAutoActivate = false;
}

void ASpiderWeatherManager::BeginPlay()
{
	Super::BeginPlay();

	CurrentParams = GetPresetParameters(InitialWeatherState);
	TargetParams = CurrentParams;
	CurrentWeatherState = InitialWeatherState;
	TargetWeatherState = InitialWeatherState;
	WeatherStateDuration = FMath::FRandRange(MinWeatherDuration, MaxWeatherDuration);

	UpdateMaterialParameters();
	UpdateAudioAndFX();

	UE_LOG(LogSpiderWorld, Log, TEXT("SpiderWeatherManager initialized with weather state: %d"), static_cast<int32>(CurrentWeatherState));
}

void ASpiderWeatherManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateTransitions(DeltaSeconds);
	UpdateMaterialParameters();
	UpdateAudioAndFX();
	PositionRainAbovePlayer();

	if (bEnableRandomWeatherCycle)
	{
		UpdateRandomCycle(DeltaSeconds);
	}

	// Thunder simulation during thunderstorm
	if (CurrentWeatherState == ESpiderWeatherState::HeavyThunderstorm)
	{
		TimeSinceLastThunder += DeltaSeconds;
		if (TimeSinceLastThunder >= CurrentParams.ThunderFrequency && CurrentParams.ThunderFrequency > 0.0f)
		{
			TriggerThunderStrike();
			TimeSinceLastThunder = 0.0f;
		}
	}
}

void ASpiderWeatherManager::SetWeatherState(ESpiderWeatherState NewState, float TransitionDuration)
{
	if (CurrentWeatherState == NewState && !bIsTransitioning)
	{
		return;
	}

	const ESpiderWeatherState PrevState = CurrentWeatherState;
	TargetWeatherState = NewState;
	TargetParams = GetPresetParameters(TargetWeatherState);

	TotalTransitionTime = FMath::Max(TransitionDuration, 0.1f);
	TransitionProgress = 0.0f;
	bIsTransitioning = true;

	OnWeatherStateChanged.Broadcast(PrevState, TargetWeatherState);
	UE_LOG(LogSpiderWorld, Log, TEXT("Weather transitioning from %d to %d (Duration: %0.1fs)"), static_cast<int32>(PrevState), static_cast<int32>(TargetWeatherState), TotalTransitionTime);
}

FSpiderWeatherParameters ASpiderWeatherManager::GetPresetParameters(ESpiderWeatherState State) const
{
	FSpiderWeatherParameters Params;

	switch (State)
	{
	case ESpiderWeatherState::PartlyCloudy:
		Params.CloudDensity = 0.35f;
		Params.RainIntensity = 0.0f;
		Params.WindSpeed = 300.0f;
		Params.SurfaceWetnessTarget = 0.0f;
		Params.PuddleAccumulationTarget = 0.0f;
		Params.FogDensity = 0.003f;
		Params.ThunderFrequency = 0.0f;
		break;

	case ESpiderWeatherState::Overcast:
		Params.CloudDensity = 0.85f;
		Params.RainIntensity = 0.0f;
		Params.WindSpeed = 450.0f;
		Params.SurfaceWetnessTarget = 0.1f;
		Params.PuddleAccumulationTarget = 0.0f;
		Params.FogDensity = 0.008f;
		Params.ThunderFrequency = 0.0f;
		break;

	case ESpiderWeatherState::LightRain:
		Params.CloudDensity = 0.9f;
		Params.RainIntensity = 0.45f;
		Params.WindSpeed = 600.0f;
		Params.SurfaceWetnessTarget = 0.75f;
		Params.PuddleAccumulationTarget = 0.4f;
		Params.FogDensity = 0.015f;
		Params.ThunderFrequency = 0.0f;
		break;

	case ESpiderWeatherState::HeavyThunderstorm:
		Params.CloudDensity = 1.0f;
		Params.RainIntensity = 1.0f;
		Params.WindSpeed = 1200.0f;
		Params.SurfaceWetnessTarget = 1.0f;
		Params.PuddleAccumulationTarget = 1.0f;
		Params.FogDensity = 0.035f;
		Params.ThunderFrequency = 14.0f;
		break;

	case ESpiderWeatherState::DenseFog:
		Params.CloudDensity = 0.7f;
		Params.RainIntensity = 0.05f;
		Params.WindSpeed = 100.0f;
		Params.SurfaceWetnessTarget = 0.4f;
		Params.PuddleAccumulationTarget = 0.1f;
		Params.FogDensity = 0.08f;
		Params.ThunderFrequency = 0.0f;
		break;

	case ESpiderWeatherState::Clear:
	default:
		Params.CloudDensity = 0.05f;
		Params.RainIntensity = 0.0f;
		Params.WindSpeed = 150.0f;
		Params.SurfaceWetnessTarget = 0.0f;
		Params.PuddleAccumulationTarget = 0.0f;
		Params.FogDensity = 0.001f;
		Params.ThunderFrequency = 0.0f;
		break;
	}

	return Params;
}

void ASpiderWeatherManager::UpdateTransitions(float DeltaSeconds)
{
	if (bIsTransitioning)
	{
		TransitionProgress += (DeltaSeconds / TotalTransitionTime);

		const float Alpha = FMath::Clamp(TransitionProgress, 0.0f, 1.0f);
		CurrentParams.CloudDensity = FMath::Lerp(CurrentParams.CloudDensity, TargetParams.CloudDensity, Alpha);
		CurrentParams.RainIntensity = FMath::Lerp(CurrentParams.RainIntensity, TargetParams.RainIntensity, Alpha);
		CurrentParams.WindSpeed = FMath::Lerp(CurrentParams.WindSpeed, TargetParams.WindSpeed, Alpha);
		CurrentParams.FogDensity = FMath::Lerp(CurrentParams.FogDensity, TargetParams.FogDensity, Alpha);
		CurrentParams.ThunderFrequency = TargetParams.ThunderFrequency;

		if (TransitionProgress >= 1.0f)
		{
			CurrentParams = TargetParams;
			CurrentWeatherState = TargetWeatherState;
			bIsTransitioning = false;
			TimeInCurrentWeather = 0.0f;
		}
	}

	// Surface wetness accumulation / drying rate
	const float WetnessSpeed = (CurrentParams.SurfaceWetnessTarget > CurrentWetness) ? 0.15f : 0.03f; // Dries slowly
	CurrentWetness = FMath::FInterpTo(CurrentWetness, CurrentParams.SurfaceWetnessTarget, DeltaSeconds, WetnessSpeed);

	const float PuddleSpeed = (CurrentParams.PuddleAccumulationTarget > CurrentPuddleLevel) ? 0.08f : 0.02f;
	CurrentPuddleLevel = FMath::FInterpTo(CurrentPuddleLevel, CurrentParams.PuddleAccumulationTarget, DeltaSeconds, PuddleSpeed);
}

void ASpiderWeatherManager::UpdateMaterialParameters()
{
	if (!WeatherMaterialParameters)
	{
		return;
	}

	UMaterialParameterCollectionInstance* MPC = GetWorld()->GetParameterCollectionInstance(WeatherMaterialParameters);
	if (!MPC)
	{
		return;
	}

	MPC->SetScalarParameterValue(TEXT("GlobalWetness"), CurrentWetness);
	MPC->SetScalarParameterValue(TEXT("GlobalPuddleAccumulation"), CurrentPuddleLevel);
	MPC->SetScalarParameterValue(TEXT("RainDropletIntensity"), CurrentParams.RainIntensity);
	MPC->SetScalarParameterValue(TEXT("WindSpeedNormalized"), CurrentParams.WindSpeed / 1500.0f);
	MPC->SetScalarParameterValue(TEXT("CloudCoverage"), CurrentParams.CloudDensity);
	MPC->SetScalarParameterValue(TEXT("VolumetricFogDensity"), CurrentParams.FogDensity);
}

void ASpiderWeatherManager::UpdateAudioAndFX()
{
	// Rain Niagara particle scaling
	if (RainNiagaraComponent)
	{
		if (CurrentParams.RainIntensity > 0.01f)
		{
			if (!RainNiagaraComponent->IsActive())
			{
				RainNiagaraComponent->Activate(true);
			}
			RainNiagaraComponent->SetFloatParameter(TEXT("SpawnRateMultiplier"), CurrentParams.RainIntensity);
		}
		else if (RainNiagaraComponent->IsActive())
		{
			RainNiagaraComponent->Deactivate();
		}
	}

	// Rain Audio
	if (RainAudioComponent)
	{
		if (CurrentParams.RainIntensity > 0.05f)
		{
			if (!RainAudioComponent->IsPlaying())
			{
				RainAudioComponent->Play();
			}
			RainAudioComponent->SetVolumeMultiplier(CurrentParams.RainIntensity);
		}
		else if (RainAudioComponent->IsPlaying())
		{
			RainAudioComponent->FadeOut(2.0f, 0.0f);
		}
	}

	// Wind Audio
	if (WindAudioComponent)
	{
		const float WindVolume = FMath::Clamp(CurrentParams.WindSpeed / 1200.0f, 0.0f, 1.0f);
		if (WindVolume > 0.1f)
		{
			if (!WindAudioComponent->IsPlaying())
			{
				WindAudioComponent->Play();
			}
			WindAudioComponent->SetVolumeMultiplier(WindVolume);
		}
		else if (WindAudioComponent->IsPlaying())
		{
			WindAudioComponent->FadeOut(3.0f, 0.0f);
		}
	}
}

void ASpiderWeatherManager::PositionRainAbovePlayer()
{
	AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Player && RainNiagaraComponent)
	{
		// Position rain volume directly above player
		const FVector PlayerLoc = Player->GetActorLocation();
		RainNiagaraComponent->SetWorldLocation(PlayerLoc + FVector(0.0f, 0.0f, 1200.0f));
	}
}

void ASpiderWeatherManager::TriggerThunderStrike()
{
	AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	const FVector Center = Player ? Player->GetActorLocation() : GetActorLocation();

	const float StrikeDist = FMath::FRandRange(1500.0f, 6000.0f);
	const float Angle = FMath::FRandRange(0.0f, 360.0f);
	const FVector Offset = FRotator(0.0f, Angle, 0.0f).RotateVector(FVector(StrikeDist, 0.0f, 0.0f));
	const FVector StrikeLoc = Center + Offset;

	if (USoundBase* SFX = ThunderSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(this, SFX, StrikeLoc, 1.5f);
	}

	OnThunderStrikeTriggered.Broadcast(StrikeLoc);
	UE_LOG(LogSpiderWorld, Log, TEXT("Thunder strike triggered at: %s"), *StrikeLoc.ToString());
}

void ASpiderWeatherManager::UpdateRandomCycle(float DeltaSeconds)
{
	TimeInCurrentWeather += DeltaSeconds;
	if (TimeInCurrentWeather >= WeatherStateDuration && !bIsTransitioning)
	{
		const int32 RandomStateIndex = FMath::RandRange(0, 5);
		SetWeatherState(static_cast<ESpiderWeatherState>(RandomStateIndex), 10.0f);
		WeatherStateDuration = FMath::FRandRange(MinWeatherDuration, MaxWeatherDuration);
	}
}
