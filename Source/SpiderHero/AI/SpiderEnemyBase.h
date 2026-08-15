// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/SpiderHeroTypes.h"
#include "Combat/Interfaces/IDamageable.h"
#include "Combat/Interfaces/ICombatTarget.h"
#include "Combat/Interfaces/IWebAttachable.h"
#include "SpiderEnemyBase.generated.h"

class USpiderHealthComponent;
class ASpiderEnemyDirector;
class UNiagaraSystem;
class USoundBase;

/**
 * AI state machine for combat and crowd coordination
 */
UENUM(BlueprintType)
enum class ESpiderEnemyAIState : uint8
{
	Idle            = 0 UMETA(DisplayName = "Idle"),
	Patrol          = 1 UMETA(DisplayName = "Patrol"),
	Approaching     = 2 UMETA(DisplayName = "Approaching Target"),
	Circling        = 3 UMETA(DisplayName = "Circling / Spacing"),
	PreparingAttack = 4 UMETA(DisplayName = "Preparing Attack (Telegraph)"),
	Attacking       = 5 UMETA(DisplayName = "Attacking"),
	Staggered       = 6 UMETA(DisplayName = "Staggered"),
	KnockedDown     = 7 UMETA(DisplayName = "Knocked Down"),
	AirLaunched     = 8 UMETA(DisplayName = "Air Launched (Juggle)"),
	WebBound        = 9 UMETA(DisplayName = "Web Bound"),
	WallPinned      = 10 UMETA(DisplayName = "Wall Pinned"),
	Dead            = 11 UMETA(DisplayName = "Dead")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyAIStateChangedSignature, ESpiderEnemyAIState, PreviousState, ESpiderEnemyAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyWebGaugeChangedSignature, float, CurrentGauge, float, MaxGauge);

/**
 * ASpiderEnemyBase
 * Base Enemy Character class implementing IDamageable, ICombatTarget, and IWebAttachable.
 * Provides hit reactions, ragdoll physics, web gauge accumulation, air juggle states,
 * and integration with the Attack Token Director.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderEnemyBase : public ACharacter, public IDamageable, public ICombatTarget, public IWebAttachable
{
	GENERATED_BODY()

public:
	ASpiderEnemyBase(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Landed(const FHitResult& Hit) override;

	// IDamageable Interface Implementation
	virtual float TakeDamageCustom_Implementation(float DamageAmount, const FSpiderHitReaction& HitReaction) override;
	virtual bool IsAlive_Implementation() const override { return CurrentAIState != ESpiderEnemyAIState::Dead; }
	virtual float GetHealthPercent_Implementation() const override;
	virtual void ApplyStun_Implementation(float Duration) override;
	virtual void ApplyKnockdown_Implementation(const FVector& LaunchVelocity) override;
	virtual void ApplyHitReaction_Implementation(const FSpiderHitReaction& HitReaction) override;
	virtual bool CanBeDamaged_Implementation() const override { return CurrentAIState != ESpiderEnemyAIState::Dead; }

	// ICombatTarget Interface Implementation
	virtual FVector GetTargetLocation_Implementation() const override;
	virtual FVector GetCombatSocketLocation_Implementation(FName SocketName) const override;
	virtual bool CanBeLockedOn_Implementation() const override { return CurrentAIState != ESpiderEnemyAIState::Dead; }
	virtual int32 GetTargetThreatLevel_Implementation() const override { return ThreatLevel; }
	virtual void OnTargeted_Implementation(bool bIsTargeted, AActor* TargetingActor) override;
	virtual bool IsCombatTargetValid_Implementation() const override { return CurrentAIState != ESpiderEnemyAIState::Dead; }
	virtual float GetTargetRadius_Implementation() const override;

	// IWebAttachable Interface Implementation
	virtual bool CanAttachWeb_Implementation() const override { return CurrentAIState != ESpiderEnemyAIState::Dead; }
	virtual void OnWebAttached_Implementation(AActor* AttachingActor, const FVector& AttachPoint) override;
	virtual void OnWebPulled_Implementation(AActor* PullingActor, const FVector& PullDirection, float Force) override;
	virtual void OnWebBound_Implementation(float Duration, AActor* InstigatorActor) override;
	virtual void ApplyWebGauge_Implementation(float Amount, AActor* InstigatorActor) override;
	virtual float GetWebGaugePercent_Implementation() const override { return WebGauge / MaxWebGauge; }
	virtual bool IsWebBound_Implementation() const override { return CurrentAIState == ESpiderEnemyAIState::WebBound || CurrentAIState == ESpiderEnemyAIState::WallPinned; }
	virtual FVector GetWebAnchorPoint_Implementation() const override;
	virtual bool TryPinToSurface_Implementation(const FVector& SurfaceLocation, const FVector& SurfaceNormal) override;

	// State Control
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|State")
	void SetAIState(ESpiderEnemyAIState NewState);

	UFUNCTION(BlueprintPure, Category = "Spider|AI|State")
	ESpiderEnemyAIState GetAIState() const { return CurrentAIState; }

	// Attack Token Coordination
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Token")
	void SetHasAttackToken(bool bHasToken) { bHasAttackToken = bHasToken; }

	UFUNCTION(BlueprintPure, Category = "Spider|AI|Token")
	bool HasAttackToken() const { return bHasAttackToken; }

	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Slot")
	void SetAssignedSlotIndex(int32 SlotIndex) { AssignedSlotIndex = SlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Spider|AI|Slot")
	int32 GetAssignedSlotIndex() const { return AssignedSlotIndex; }

	// State Recovery Handlers
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Combat")
	virtual void RecoverFromStagger();

	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Combat")
	virtual void RecoverFromKnockdown();

	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Combat")
	virtual void RecoverFromWebBound();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|AI|Events")
	FOnEnemyAIStateChangedSignature OnAIStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|AI|Events")
	FOnEnemyWebGaugeChangedSignature OnWebGaugeChanged;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Components")
	TObjectPtr<USpiderHealthComponent> HealthComponent;

	// Enemy Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Config")
	int32 ThreatLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Config")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Config")
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Config")
	float MaxWebGauge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Config")
	float WebGauge;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Config")
	float WebGaugeDecayRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Config")
	float DefaultWebBoundDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Config")
	float StaggerDurationDefault;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Config")
	float KnockdownRecoveryDelay;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|State")
	ESpiderEnemyAIState CurrentAIState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|State")
	bool bHasAttackToken;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|State")
	int32 AssignedSlotIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|State")
	bool bIsTargetedByPlayer;

	// Timers
	FTimerHandle StateRecoveryTimerHandle;
	FTimerHandle RagdollCleanupTimerHandle;

	// Virtual Handlers
	virtual void HandleDeath(AActor* Killer);
	virtual void EnterRagdoll();
	virtual void CleanupCorpse();
};
