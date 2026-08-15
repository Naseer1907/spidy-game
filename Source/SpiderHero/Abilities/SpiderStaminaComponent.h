// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpiderStaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnStaminaChangedSignature, float, CurrentStamina, float, MaxStamina, float, Delta, bool, bIsFatigued);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFatigueStateChangedSignature, bool, bIsFatigued);

/**
 * USpiderStaminaComponent
 * Manages player stamina consumption for web-swing boosts, dodging, heavy attacks, and parkour sprints.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderStaminaComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Attempt to consume stamina for an action. Returns true if consumption succeeded */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Stamina")
	bool ConsumeStamina(float Amount);

	/** Check if there is enough stamina available */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Stamina")
	bool HasEnoughStamina(float RequiredAmount) const;

	/** Restore a specific amount of stamina */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Stamina")
	void RestoreStamina(float Amount);

	/** Instantly reset stamina to maximum */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Stamina")
	void ResetStamina();

	/** Pause or resume stamina regeneration */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Stamina")
	void SetRegenPaused(bool bPaused) { bRegenPaused = bPaused; }

	/** Check if player is currently in fatigue state */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Stamina")
	bool IsFatigued() const { return bIsFatigued; }

	/** Get current stamina value */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	/** Get maximum stamina */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	/** Get stamina normalized (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Stamina")
	float GetStaminaPercent() const { return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f; }

	/** Set maximum stamina */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Stamina")
	void SetMaxStamina(float NewMaxStamina, bool bFillToMax = false);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Stamina|Events")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Stamina|Events")
	FOnFatigueStateChangedSignature OnFatigueStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Stamina")
	float MaxStamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Abilities|Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Stamina|Regen")
	float StaminaRegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Stamina|Regen")
	float StaminaRegenDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Stamina|Fatigue")
	float FatigueThreshold;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Abilities|Stamina")
	bool bIsFatigued;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Abilities|Stamina")
	bool bRegenPaused;

private:
	float TimeSinceLastConsumption;

	void UpdateFatigueState();
};
