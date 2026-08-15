// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/SpiderEnemyBase.h"
#include "SpiderEnemyMelee.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTelegraphAttackStartedSignature, float, WindupDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeleeAttackExecutedSignature);

/**
 * ASpiderEnemyMelee
 * Concrete melee brawler / grunt archetype.
 * Features:
 * - Telegraph attack wind-up with Spider-Sense warning trigger
 * - Melee punch combo & heavy swing strike traces
 * - Circling / strafing spacing around combat slots
 * - Stagger recovery and vulnerability windows
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderEnemyMelee : public ASpiderEnemyBase
{
	GENERATED_BODY()

public:
	ASpiderEnemyMelee(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Initiates attack wind-up / telegraph phase */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Combat")
	virtual bool StartAttackTelegraph(float CustomWindup = -1.0f);

	/** Executes actual melee strike hitbox */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Combat")
	virtual void ExecuteMeleeStrike();

	/** Aborts attack cleanly (e.g. if interrupted by player strike or web) */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Combat")
	virtual void AbortAttack();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|AI|Events")
	FOnTelegraphAttackStartedSignature OnTelegraphAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|AI|Events")
	FOnMeleeAttackExecutedSignature OnMeleeAttackExecuted;

protected:
	// Melee Attack Config
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Combat")
	float AttackDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Combat")
	float AttackRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Combat")
	float AttackRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Combat")
	float TelegraphWindupDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Combat")
	float AttackCooldownDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Combat")
	float KnockbackForce;

	// Animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Animation")
	TSoftObjectPtr<UAnimMontage> MeleeAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Animation")
	FName AttackSectionName;

	// VFX / SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Effects")
	TSoftObjectPtr<UNiagaraSystem> TelegraphIndicatorVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Effects")
	TSoftObjectPtr<USoundBase> TelegraphSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Effects")
	TSoftObjectPtr<USoundBase> StrikeHitSFX;

private:
	FTimerHandle TelegraphTimerHandle;
	FTimerHandle AttackCooldownTimerHandle;

	void FinishAttackCooldown();
};
