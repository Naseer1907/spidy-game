// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "Combat/SpiderCombatDataAsset.h"
#include "SpiderCombatComponent.generated.h"

class ASpiderHeroCharacter;
class USpiderCameraComponent;
class USpiderStaminaComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatComboChangedSignature, int32, CurrentCombo, int32, MaxCombo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerfectDodgeTriggeredSignature, AActor*, EvadedThreat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttackHitSignature, AActor*, Target, float, Damage, bool, bIsFinisher);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatTargetChangedSignature, AActor*, NewTarget);

/**
 * USpiderCombatComponent
 * Primary combat engine implementing:
 * - 3-Hit Light Combo Chain with responsive buffer window
 * - Heavy Launcher uppercut launching enemies airborne
 * - Aerial Juggle Combo & Ground Slam Finisher
 * - Directional Dodge & Frame-Perfect Dodge with Slow-Mo Time Dilation
 * - Micro-freeze Hit Pause on impact
 * - Soft-Lock Targeting and Motion Warping Assistance
 */
UCLASS(ClassGroup = (SpiderCombat), meta = (BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Input Triggers
	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Input")
	bool TryLightAttack();

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Input")
	bool TryHeavyAttack();

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Input")
	bool TryDodge();

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Input")
	bool TryFinisher();

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Input")
	bool TryGroundSlam();

	// Target Lock & Assisted Warping
	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Targeting")
	AActor* FindBestCombatTarget(float MaxRange = 800.0f, float MaxAngleDegrees = 75.0f);

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Targeting")
	void SetCurrentTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Spider|Combat|Targeting")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	// Animation Notify / Window Callbacks
	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|AnimNotify")
	void OpenComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|AnimNotify")
	void CloseComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|AnimNotify")
	void ExecuteHitTrace();

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|AnimNotify")
	void OnAttackFinished();

	// Hit Pause & Slow Motion
	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Feel")
	void ApplyHitPause(float Duration = 0.06f, float TimeDilation = 0.05f);

	UFUNCTION(BlueprintCallable, Category = "Spider|Combat|Feel")
	void TriggerPerfectDodge(AActor* ThreatActor);

	// Getters
	UFUNCTION(BlueprintPure, Category = "Spider|Combat|State")
	int32 GetCurrentComboStep() const { return CurrentComboStep; }

	UFUNCTION(BlueprintPure, Category = "Spider|Combat|State")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "Spider|Combat|State")
	bool IsInAirCombat() const { return bIsInAirCombat; }

	UFUNCTION(BlueprintPure, Category = "Spider|Combat|State")
	bool IsPerfectDodgeActive() const { return bPerfectDodgeActive; }

	// Event Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|Combat|Events")
	FOnCombatComboChangedSignature OnComboChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Combat|Events")
	FOnPerfectDodgeTriggeredSignature OnPerfectDodgeTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Combat|Events")
	FOnAttackHitSignature OnAttackHit;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Combat|Events")
	FOnCombatTargetChangedSignature OnTargetChanged;

protected:
	// Combat Data Assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Data")
	TArray<TObjectPtr<USpiderCombatDataAsset>> LightComboChain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Data")
	TObjectPtr<USpiderCombatDataAsset> HeavyLauncherAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Data")
	TArray<TObjectPtr<USpiderCombatDataAsset>> AirComboChain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Data")
	TObjectPtr<USpiderCombatDataAsset> GroundSlamAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Data")
	TObjectPtr<USpiderCombatDataAsset> FinisherAttack;

	// Dodge Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Dodge")
	float NormalDodgeDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Dodge")
	float PerfectDodgeWindowDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Dodge")
	float PerfectDodgeSlowMoDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Dodge")
	float PerfectDodgeTimeDilation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Dodge")
	float DodgeImpulse;

	// Targeting & Warping Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Warp")
	float MaxWarpDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Warp")
	float WarpTargetPadding;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|Warp")
	float AutoAimAngle;

	// Hit Pause Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|HitPause")
	float DefaultHitPauseDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|HitPause")
	float DefaultHitPauseDilation;

	// Ground Slam AOE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|GroundSlam")
	float GroundSlamDamageRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Combat|GroundSlam")
	float GroundSlamKnockback;

private:
	UPROPERTY()
	TWeakObjectPtr<ASpiderHeroCharacter> OwnerCharacter;

	UPROPERTY()
	TWeakObjectPtr<USpiderCameraComponent> CameraComponent;

	UPROPERTY()
	TWeakObjectPtr<USpiderStaminaComponent> StaminaComponent;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY()
	TObjectPtr<USpiderCombatDataAsset> ActiveAttackData;

	// State
	int32 CurrentComboStep;
	bool bIsAttacking;
	bool bIsInAirCombat;
	bool bComboWindowOpen;
	bool bBufferQueued;
	ESpiderAttackType QueuedAttackType;

	// Warping
	bool bIsWarping;
	FVector WarpTargetLocation;
	FRotator WarpTargetRotation;
	float WarpSpeed;

	// Dodge
	bool bIsDodging;
	bool bPerfectDodgeActive;
	FTimerHandle PerfectDodgeTimerHandle;
	FTimerHandle HitPauseTimerHandle;
	FTimerHandle ComboResetTimerHandle;

	// Internal Methods
	void StartAttack(USpiderCombatDataAsset* AttackData);
	void ResetCombo();
	void EndHitPause();
	void EndPerfectDodgeSlowMo();
	void UpdateMotionWarping(float DeltaTime);
	void PerformHitSweep(const USpiderCombatDataAsset* AttackData);
	void ApplyAttackEffects(AActor* HitActor, const FHitResult& HitInfo, const USpiderCombatDataAsset* AttackData);
	bool IsTargetWithinAttackCone(const AActor* TargetCandidate, float MaxDistance, float MaxAngleDegrees) const;
};
