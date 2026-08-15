// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UBehaviorTree;
class UBlackboardComponent;
class ASpiderEnemyBase;
class ASpiderEnemyDirector;

/**
 * ASpiderAIController
 * Perception-driven superhero AI Controller featuring:
 * - Sight and Hearing senses configuration
 * - Blackboard key synchronization (Target, Distance, SlotLocation, CombatState)
 * - Integration with Attack Token Director to avoid dogpiling
 */
UCLASS()
class SPIDERHERO_API ASpiderAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASpiderAIController(const FObjectInitializer& ObjectInitializer);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	/** Blackboard Keys */
	static const FName Key_TargetActor;
	static const FName Key_TargetLocation;
	static const FName Key_SlotLocation;
	static const FName Key_HasAttackToken;
	static const FName Key_CombatState;
	static const FName Key_DistanceToTarget;

	/** Perception update handler */
	UFUNCTION()
	void OnPerceptionTargetUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** Requests attack token from Enemy Director */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Token")
	bool RequestAttackToken();

	/** Releases held attack token back to Director */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Token")
	void ReleaseAttackToken();

	/** Sets designated slot location from director */
	UFUNCTION(BlueprintCallable, Category = "Spider|AI|Navigation")
	void SetSlotTargetLocation(const FVector& SlotWorldLocation);

protected:
	// AI Perception
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	// Behavior Tree
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|AI|Behavior")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree;

	// Perception Tunables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Perception")
	float SightRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Perception")
	float LoseSightRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Perception")
	float PeripheralVisionAngleDegrees;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|AI|Perception")
	float HearingRange;

private:
	UPROPERTY()
	TWeakObjectPtr<ASpiderEnemyBase> ControlledEnemy;

	UPROPERTY()
	TWeakObjectPtr<ASpiderEnemyDirector> EnemyDirector;

	UPROPERTY()
	TWeakObjectPtr<AActor> PerceivedTargetActor;

	void FindOrCreateDirector();
	void UpdateBlackboardState();
};
