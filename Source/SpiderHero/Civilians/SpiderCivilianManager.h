// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderCivilianManager.generated.h"

class USkeletalMesh;
class UAnimSequence;
class USoundBase;

/**
 * Pedestrian Behavior State Machine
 */
UENUM(BlueprintType)
enum class ESpiderCivilianState : uint8
{
	Walking             = 0 UMETA(DisplayName = "Walking Sidewalk"),
	WaitingAtCrosswalk  = 1 UMETA(DisplayName = "Waiting at Crosswalk"),
	IdleChatter         = 2 UMETA(DisplayName = "Idle Conversation"),
	TakingPhotoOfHero   = 3 UMETA(DisplayName = "Taking Photo of Hero"),
	Cheering            = 4 UMETA(DisplayName = "Cheering Spider-Hero"),
	FleeingDanger       = 5 UMETA(DisplayName = "Fleeing from Combat / Threat"),
	CoweringInPanic     = 6 UMETA(DisplayName = "Cowering in Panic")
};

/**
 * Lightweight Pooled Civilian Agent Data
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderCivilianAgent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	int32 CivilianId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	ESpiderCivilianState State = ESpiderCivilianState::Walking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	FVector WorldPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	FRotator WorldRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	FVector TargetPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	float MoveSpeed = 160.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	float StateTimer = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	float PanicDuration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Civilian")
	bool bIsHeroNearby = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdPanicTriggeredSignature, const FVector&, ThreatOrigin, float, PanicRadius);

/**
 * ASpiderCivilianManager
 * High-performance city pedestrian crowd simulation featuring:
 * - Dynamic sidewalk & crosswalk navigation
 * - Contextual superhero reactions (taking photos, cheering, fleeing, cowering)
 * - Distance-based tick throttling and visibility culling
 * - Panic propagation across crowd agents during villain/combat events
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderCivilianManager : public AActor
{
	GENERATED_BODY()

public:
	ASpiderCivilianManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Initializes pedestrian nodes across city district */
	UFUNCTION(BlueprintCallable, Category = "Spider|Civilian")
	void InitializePedestrians(const FVector& Center, int32 GridDimension, float BlockStride);

	/** Triggers crowd panic in a radius (e.g. from explosions, villain appearances, gunshots) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Civilian")
	void TriggerPanicAtLocation(const FVector& ThreatLocation, float Radius = 2500.0f);

	/** Returns active civilian count */
	UFUNCTION(BlueprintPure, Category = "Spider|Civilian")
	int32 GetActiveCivilianCount() const { return CivilianPool.Num(); }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|Civilian|Events")
	FOnCrowdPanicTriggeredSignature OnCrowdPanicTriggered;

protected:
	// Crowd Tunables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Config")
	int32 MaxCivilians;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Config")
	float CivilianWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Config")
	float CivilianFleeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Config")
	float HeroReactionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Config")
	float HeroPhotoDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Config")
	float TickCullingDistance;

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Audio")
	TSoftObjectPtr<USoundBase> CrowdCheerSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Civilian|Audio")
	TSoftObjectPtr<USoundBase> CrowdPanicSFX;

private:
	UPROPERTY()
	TArray<FSpiderCivilianAgent> CivilianPool;

	UPROPERTY()
	TArray<FVector> SidewalkWaypoints;

	int32 NextCivilianId;

	// Simulation Subroutines
	void UpdateCivilianAgents(float DeltaSeconds, const FVector& PlayerLocation);
	void UpdateAgentState(FSpiderCivilianAgent& Agent, float DeltaSeconds, const FVector& PlayerLocation);
	void HandleHeroReactions(FSpiderCivilianAgent& Agent, const FVector& PlayerLocation);
	void SpawnInitialCivilians();
	FVector GetRandomSidewalkPoint() const;
};
