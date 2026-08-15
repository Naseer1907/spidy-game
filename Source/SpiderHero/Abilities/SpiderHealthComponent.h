// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth, float, HealthDelta, AActor*, InstigatedBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AActor*, KillerActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInvulnerabilityChangedSignature, bool, bIsInvulnerable, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageBlockedSignature, float, BlockedDamage);

/**
 * USpiderHealthComponent
 * Handles character health, damage calculations, invulnerability frames, and health regeneration.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderHealthComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Apply damage with mitigation, i-frame check, and death evaluation */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Health")
	float ApplyDamage(float Amount, AActor* DamageCauser, const FHitResult& HitInfo);

	/** Restore health clamped to MaxHealth */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Health")
	float Heal(float Amount);

	/** Grant temporary invulnerability frames */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Health")
	void GrantInvulnerability(float Duration);

	/** Set or clear invulnerability explicitly */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Health")
	void SetInvulnerable(bool bInvulnerable, float Duration = 0.0f);

	/** Check if owner is dead */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Health")
	bool IsDead() const { return bIsDead; }

	/** Check if owner is currently invulnerable */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Health")
	bool IsInvulnerable() const { return bIsInvulnerable; }

	/** Get current health points */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	/** Get maximum health points */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Health")
	float GetMaxHealth() const { return MaxHealth; }

	/** Get health as percentage between 0.0 and 1.0 */
	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	/** Set maximum health */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Health")
	void SetMaxHealth(float NewMaxHealth, bool bHealToMax = false);

	/** Set damage mitigation percentage (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Health")
	void SetDamageMitigationPercent(float NewMitigation) { DamageMitigationPercent = FMath::Clamp(NewMitigation, 0.0f, 1.0f); }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Health|Events")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Health|Events")
	FOnDeathSignature OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Health|Events")
	FOnInvulnerabilityChangedSignature OnInvulnerabilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Health|Events")
	FOnDamageBlockedSignature OnDamageBlocked;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Health")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Abilities|Health")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Health", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageMitigationPercent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Abilities|Health")
	bool bIsInvulnerable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Abilities|Health")
	bool bIsDead;

	// Regeneration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Health|Regen")
	bool bEnableRegeneration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Health|Regen")
	float RegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Abilities|Health|Regen")
	float RegenDelay;

private:
	FTimerHandle IFrameTimerHandle;
	float TimeSinceLastDamage;

	void ClearInvulnerability();
};
