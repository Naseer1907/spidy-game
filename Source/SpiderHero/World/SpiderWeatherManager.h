// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpiderWeatherManager.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UAudioComponent;
class USoundBase;
class UMaterialParameterCollection;

/**
 * Dynamic Weather Presets
 */
UENUM(BlueprintType)
enum class ESpiderWeatherState : uint8
{
	Clear               = 0 UMETA(DisplayName = "Clear Skies"),
	PartlyCloudy        = 1 UMETA(DisplayName = "Partly Cloudy"),
	Overcast            = 2 UMETA(DisplayName = "Overcast Gray"),
	LightRain           = 3 UMETA(DisplayName = "Light Drizzle / Rain"),
	HeavyThunderstorm   = 4 UMETA(DisplayName = "Heavy Thunderstorm"),
	DenseFog            = 5 UMETA(DisplayName = "Dense Morning / Sea Fog")
};

/**
 * Weather State Physical Parameters
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderWeatherParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather")
	float CloudDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather")
	float RainIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather")
	float WindSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather")
	float SurfaceWetnessTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather")
	float PuddleAccumulationTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather")
	float FogDensity = 0.005f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather")
	float ThunderFrequency = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherStateChangedSignature, ESpiderWeatherState, PreviousState, ESpiderWeatherState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThunderStrikeTriggeredSignature, const FVector&, StrikeLocation);

/**
 * ASpiderWeatherManager
 * Comprehensive atmospheric weather simulation managing:
 * - Dynamic weather state transitions (Clear, Overcast, Rain, Thunderstorm, Fog)
 * - Material Parameter Collection (MPC) driving surface wetness, puddles, and roughness
 * - Niagara weather particle scaling and spatial rain positioning
 * - Dynamic ambient rain audio, wind loops, and thunder strikes
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderWeatherManager : public AActor
{
	GENERATED_BODY()

public:
	ASpiderWeatherManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Transitions smoothly to a new target weather state */
	UFUNCTION(BlueprintCallable, Category = "Spider|Weather")
	void SetWeatherState(ESpiderWeatherState NewState, float TransitionDuration = 8.0f);

	/** Returns current weather state */
	UFUNCTION(BlueprintPure, Category = "Spider|Weather")
	ESpiderWeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

	/** Returns current wetness factor (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Spider|Weather")
	float GetCurrentWetness() const { return CurrentWetness; }

	/** Manually triggers thunder strike and flash */
	UFUNCTION(BlueprintCallable, Category = "Spider|Weather")
	void TriggerThunderStrike();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|Weather|Events")
	FOnWeatherStateChangedSignature OnWeatherStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Weather|Events")
	FOnThunderStrikeTriggeredSignature OnThunderStrikeTriggered;

protected:
	// Weather State Presets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather|Config")
	ESpiderWeatherState InitialWeatherState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather|Config")
	bool bEnableRandomWeatherCycle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather|Config")
	float MinWeatherDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather|Config")
	float MaxWeatherDuration;

	// Material Parameter Collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather|Materials")
	TObjectPtr<UMaterialParameterCollection> WeatherMaterialParameters;

	// Niagara FX
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Weather|FX")
	TObjectPtr<UNiagaraComponent> RainNiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather|FX")
	TSoftObjectPtr<UNiagaraSystem> RainSystemAsset;

	// Ambient Audio
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Weather|Audio")
	TObjectPtr<UAudioComponent> RainAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Weather|Audio")
	TObjectPtr<UAudioComponent> WindAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Weather|Audio")
	TSoftObjectPtr<USoundBase> ThunderSFX;

private:
	ESpiderWeatherState CurrentWeatherState;
	ESpiderWeatherState TargetWeatherState;

	FSpiderWeatherParameters CurrentParams;
	FSpiderWeatherParameters TargetParams;

	float TransitionProgress;
	float TotalTransitionTime;
	bool bIsTransitioning;

	float CurrentWetness;
	float CurrentPuddleLevel;
	float TimeInCurrentWeather;
	float WeatherStateDuration;
	float TimeSinceLastThunder;

	// Subroutines
	FSpiderWeatherParameters GetPresetParameters(ESpiderWeatherState State) const;
	void UpdateTransitions(float DeltaSeconds);
	void UpdateMaterialParameters();
	void UpdateAudioAndFX();
	void UpdateRandomCycle(float DeltaSeconds);
	void PositionRainAbovePlayer();
};
