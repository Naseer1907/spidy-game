// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpiderHeroGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrimeRateChangedSignature, float, NewCrimeRate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroReputationChangedSignature, int32, NewReputation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboScoreChangedSignature, int32, CurrentCombo, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeatLevelChangedSignature, int32, NewHeatLevel);

/**
 * ASpiderHeroGameState
 * Manages open world city metrics, hero reputation, combat combo state, and game progression.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderHeroGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASpiderHeroGameState();

	virtual void BeginPlay() override;

	/** Adjust city crime rate percentage (clamped between 0.0 and 100.0) */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameState|City")
	void ModifyCrimeRate(float Delta);

	/** Set absolute crime rate */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameState|City")
	void SetCrimeRate(float NewRate);

	/** Get current crime rate percentage */
	UFUNCTION(BlueprintPure, Category = "Spider|GameState|City")
	float GetCrimeRate() const { return CityCrimeRate; }

	/** Adjust Hero Reputation points */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameState|Progression")
	void AddReputation(int32 Amount);

	/** Get current Hero Reputation */
	UFUNCTION(BlueprintPure, Category = "Spider|GameState|Progression")
	int32 GetHeroReputation() const { return HeroReputation; }

	/** Register combo hit increment and update multiplier */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameState|Combat")
	void RegisterComboHit();

	/** Reset combat combo */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameState|Combat")
	void ResetCombo();

	/** Get current combo hit count */
	UFUNCTION(BlueprintPure, Category = "Spider|GameState|Combat")
	int32 GetCurrentComboCount() const { return CurrentComboCount; }

	/** Get current combo damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Spider|GameState|Combat")
	float GetCurrentComboMultiplier() const { return CurrentComboMultiplier; }

	/** Modify current Heat Level (0 to 5) */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameState|City")
	void SetHeatLevel(int32 InHeatLevel);

	/** Get current Heat Level */
	UFUNCTION(BlueprintPure, Category = "Spider|GameState|City")
	int32 GetHeatLevel() const { return HeatLevel; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|GameState|Events")
	FOnCrimeRateChangedSignature OnCrimeRateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|GameState|Events")
	FOnHeroReputationChangedSignature OnHeroReputationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|GameState|Events")
	FOnComboScoreChangedSignature OnComboScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|GameState|Events")
	FOnHeatLevelChangedSignature OnHeatLevelChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|GameState|City")
	float CityCrimeRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|GameState|Progression")
	int32 HeroReputation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|GameState|Combat")
	int32 CurrentComboCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|GameState|Combat")
	float CurrentComboMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|GameState|City")
	int32 HeatLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|GameState|Combat")
	float ComboTimeoutDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|GameState|Combat")
	float MaxComboMultiplier;

private:
	FTimerHandle ComboTimerHandle;
	void HandleComboTimeout();
};
