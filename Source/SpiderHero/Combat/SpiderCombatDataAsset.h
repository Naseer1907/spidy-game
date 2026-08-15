// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/SpiderHeroTypes.h"
#include "Camera/SpiderCameraComponent.h"
#include "SpiderCombatDataAsset.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class USoundBase;

/**
 * USpiderCombatDataAsset
 * Primary Data Asset defining modular combat attack moves, combo windows, hit reactions,
 * VFX, SFX, Hit Pause, and Distance Warping parameters.
 */
UCLASS(BlueprintType)
class SPIDERHERO_API USpiderCombatDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USpiderCombatDataAsset();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// Attack Identification
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Identity")
	FName AttackID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Identity")
	FText AttackDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Identity")
	ESpiderAttackType AttackType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Identity")
	int32 ComboStepIndex;

	// Damage & Combat Stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	float StaminaCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	float KnockbackForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	FVector KnockbackDirectionOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	float StunDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	bool bCausesLauncher;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	float LauncherVerticalImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	bool bCausesGroundSlam;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	float GroundSlamRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Damage")
	bool bCausesKnockdown;

	// Range & Distance Warping Assistance
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Targeting")
	float AttackRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Targeting")
	float AttackHitRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Targeting")
	float MaxWarpDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Targeting")
	float WarpInterpSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Targeting")
	float AutoTargetAngleThreshold;

	// Combo Windows & Timing
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Timing")
	float ComboWindowStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Timing")
	float ComboWindowEnd;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Timing")
	float HitCancelWindowStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Timing")
	float RecoveryDuration;

	// Animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Animation")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Animation")
	FName MontageSectionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Animation")
	float PlayRate;

	// Audio & Visual Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Effects")
	TSoftObjectPtr<UNiagaraSystem> TrailVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Effects")
	FName TrailSocketStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Effects")
	FName TrailSocketEnd;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Effects")
	TSoftObjectPtr<UNiagaraSystem> HitImpactVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Effects")
	TSoftObjectPtr<USoundBase> SwingSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Effects")
	TSoftObjectPtr<USoundBase> ImpactSFX;

	// Game Feel & Camera
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Feel")
	float HitPauseDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Feel")
	float HitPauseTimeDilation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Feel")
	ESpiderCameraImpactType CameraImpactType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Feel")
	float CameraImpactIntensity;
};
