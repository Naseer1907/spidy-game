// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpiderDayNightManager.generated.h"

class ADirectionalLight;
class ASkyLight;
class ASkyAtmosphere;
class APostProcessVolume;
class UMaterialParameterCollection;

/**
 * Categorized Time of Day Phases
 */
UENUM(BlueprintType)
enum class ESpiderTimeOfDay : uint8
{
	Dawn        = 0 UMETA(DisplayName = "Dawn / Early Sunrise"),
	Morning     = 1 UMETA(DisplayName = "Morning Daylight"),
	Noon        = 2 UMETA(DisplayName = "High Noon"),
	GoldenHour  = 3 UMETA(DisplayName = "Golden Hour Afternoon"),
	Sunset      = 4 UMETA(DisplayName = "Sunset / Twilight"),
	Dusk        = 5 UMETA(DisplayName = "Dusk"),
	Night       = 6 UMETA(DisplayName = "Midnight / Starry Night")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChangedSignature, ESpiderTimeOfDay, NewPhase, float, CurrentTimeHours);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNightStateToggledSignature, bool, bIsNight);

/**
 * ASpiderDayNightManager
 * Dynamic 24-hour Celestial Day/Night Simulator:
 * - Dynamic Sun and Moon solar azimuth and inclination calculation
 * - Real-time SkyLight, SkyAtmosphere, and PostProcess adjustments
 * - Night window emissive material parameter driving and street lamp automation
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderDayNightManager : public AActor
{
	GENERATED_BODY()

public:
	ASpiderDayNightManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Sets time of day in 24-hour format (e.g. 14.5 = 2:30 PM) */
	UFUNCTION(BlueprintCallable, Category = "Spider|World|Time")
	void SetTimeOfDay(float NewTimeInHours);

	/** Sets time multiplier (e.g. 1.0 = real-time, 60.0 = 24 mins full day) */
	UFUNCTION(BlueprintCallable, Category = "Spider|World|Time")
	void SetTimeSpeedMultiplier(float NewMultiplier) { TimeSpeedMultiplier = NewMultiplier; }

	/** Returns current time in 24h format */
	UFUNCTION(BlueprintPure, Category = "Spider|World|Time")
	float GetCurrentTimeHours() const { return CurrentTimeHours; }

	/** Returns current time of day phase */
	UFUNCTION(BlueprintPure, Category = "Spider|World|Time")
	ESpiderTimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

	/** Returns true if current hour is night */
	UFUNCTION(BlueprintPure, Category = "Spider|World|Time")
	bool IsNightTime() const { return bIsNight; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|World|Events")
	FOnTimeOfDayChangedSignature OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|World|Events")
	FOnNightStateToggledSignature OnNightStateToggled;

protected:
	// Cycle Tunables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float InitialTimeHours;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Time")
	float TimeSpeedMultiplier; // 60.0f = 24 min full cycle

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Time")
	bool bPauseTimeProgression;

	// Sun and Moon Solar Alignment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Sun")
	float SunLatitudeDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Sun")
	float SunMaxIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Moon")
	float MoonMaxIntensity;

	// Material Parameter Collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Materials")
	TObjectPtr<UMaterialParameterCollection> CityMaterialParameters;

	// World Scene Actor References
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|References")
	TObjectPtr<ADirectionalLight> SunLightActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|References")
	TObjectPtr<ADirectionalLight> MoonLightActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|References")
	TObjectPtr<ASkyLight> SkyLightActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|References")
	TObjectPtr<ASkyAtmosphere> SkyAtmosphereActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|References")
	TObjectPtr<APostProcessVolume> PostProcessVolumeActor;

private:
	float CurrentTimeHours;
	ESpiderTimeOfDay CurrentTimePhase;
	bool bIsNight;

	// Subroutines
	void UpdateCelestialBodies();
	void UpdateMaterialParameters();
	void EvaluateTimePhase();
	void FindSceneReferences();
};
