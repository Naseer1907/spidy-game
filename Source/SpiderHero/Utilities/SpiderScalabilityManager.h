// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "SpiderScalabilityManager.generated.h"

UENUM(BlueprintType)
enum class ESpiderScalabilityLevel : uint8
{
	Low     = 0 UMETA(DisplayName = "Low"),
	Medium  = 1 UMETA(DisplayName = "Medium"),
	High    = 2 UMETA(DisplayName = "High"),
	Ultra   = 3 UMETA(DisplayName = "Ultra / Cinematic")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScalabilityPresetAppliedSignature, ESpiderScalabilityLevel, NewLevel);

/**
 * USpiderScalabilityManager
 * Engine subsystem managing runtime graphics scalability, Lumen/Nanite controls,
 * shadow fidelity, and open-world crowd budgets.
 */
UCLASS()
class SPIDERHERO_API USpiderScalabilityManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	USpiderScalabilityManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Applies global preset: 0=Low, 1=Med, 2=High, 3=Ultra */
	UFUNCTION(BlueprintCallable, Category = "Spider|Scalability")
	void ApplyScalabilityPreset(ESpiderScalabilityLevel Level);

	/** Sets dynamic resolution scaling target */
	UFUNCTION(BlueprintCallable, Category = "Spider|Scalability")
	void SetDynamicResolutionEnabled(bool bEnabled, float TargetFPS = 60.0f);

	/** Get current active level */
	UFUNCTION(BlueprintPure, Category = "Spider|Scalability")
	ESpiderScalabilityLevel GetCurrentScalabilityLevel() const { return CurrentLevel; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Scalability|Events")
	FOnScalabilityPresetAppliedSignature OnScalabilityPresetApplied;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Scalability")
	ESpiderScalabilityLevel CurrentLevel;

private:
	void ExecuteConsoleCommand(const FString& Command);
};
