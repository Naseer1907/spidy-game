// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderEnemyDirector.generated.h"

class ASpiderEnemyBase;
class ASpiderHeroCharacter;

/**
 * Combat ring slot struct
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderCombatSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Slots")
	int32 SlotIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Slots")
	float AngleDegrees = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Slots")
	float Radius = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Slots")
	FVector WorldPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Slots")
	TWeakObjectPtr<ASpiderEnemyBase> Occupant = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Slots")
	bool bIsOccupied = false;
};

/**
 * Active token holder tracking
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderAttackTokenHolder
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<ASpiderEnemyBase> Enemy = nullptr;

	UPROPERTY()
	float TimeTokenGranted = 0.0f;

	UPROPERTY()
	float MaxTokenHoldDuration = 4.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackTokenGrantedSignature, ASpiderEnemyBase*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackTokenReleasedSignature, ASpiderEnemyBase*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatEscalationChangedSignature, int32, NewEscalationLevel);

/**
 * ASpiderEnemyDirector
 * Central coordinator for enemy crowd AI and attack choreography:
 * - Prevents dogpiling by limiting concurrent attack tokens (1-2 attackers max)
 * - Computes dynamic circular combat slots around player for cinematic spacing
 * - Manages flanking, circling, and combat escalation
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderEnemyDirector : public AActor
{
	GENERATED_BODY()

public:
	ASpiderEnemyDirector();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Enrolls an enemy into the combat director */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Director")
	void RegisterEnemy(ASpiderEnemyBase* Enemy);

	/** Unregisters an enemy (on death or despawn) */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Director")
	void UnregisterEnemy(ASpiderEnemyBase* Enemy);

	/** Requests an attack token */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Director")
	bool RequestAttackToken(ASpiderEnemyBase* RequestingEnemy);

	/** Releases an attack token */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Director")
	void ReleaseAttackToken(ASpiderEnemyBase* ReleasingEnemy);

	/** Gets assigned world position for an enemy's slot */
	UFUNCTION(BlueprintPure, Category = "Spider|AI|Director")
	FVector GetSlotLocationForEnemy(const ASpiderEnemyBase* Enemy) const;

	/** Sets combat escalation level (1: Calm, 2: Standard, 3: Aggressive / High Wave) */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Director")
	void SetEscalationLevel(int32 NewLevel);

	/** Gets active registered enemy count */
	UFUNCTION(BlueprintPure, Category = "Spider|AI|Director")
	int32 GetRegisteredEnemyCount() const { return RegisteredEnemies.Num(); }

	/** Delegates */
	UPROPERTY(BlueprintAssignable, Category = "Spider|AI|Events")
	FOnAttackTokenGrantedSignature OnAttackTokenGranted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|AI|Events")
	FOnAttackTokenReleasedSignature OnAttackTokenReleased;

	UPROPERTY(BlueprintAssignable, Category = "Spider|AI|Events")
	FOnCombatEscalationChangedSignature OnCombatEscalationChanged;

protected:
	// Token Tunables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Tokens")
	int32 MaxConcurrentAttackTokens;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Tokens")
	float MaxTokenDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Tokens")
	float MinDelayBetweenAttacks;

	// Slot Geometry
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Slots")
	int32 InnerSlotCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Slots")
	float InnerRingRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Slots")
	int32 OuterSlotCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Slots")
	float OuterRingRadius;

	// Escalation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Escalation")
	int32 CurrentEscalationLevel;

	// Slots Array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Slots")
	TArray<FSpiderCombatSlot> CombatSlots;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<ASpiderEnemyBase>> RegisteredEnemies;

	UPROPERTY()
	TArray<FSpiderAttackTokenHolder> ActiveTokenHolders;

	UPROPERTY()
	TWeakObjectPtr<ASpiderHeroCharacter> TargetPlayer;

	float TimeSinceLastTokenGrant;

	void InitializeSlots();
	void UpdateSlotPositions();
	void AssignSlotsToEnemies();
	void ValidateActiveTokens(float DeltaSeconds);
	void FindPlayer();
};
