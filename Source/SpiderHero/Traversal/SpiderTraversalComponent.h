// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderTraversalComponent.generated.h"

class ASpiderHeroCharacter;
class USpiderMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTraversalActionStartedSignature, ESpiderParkourAction, Action, const FVector&, TargetLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTraversalActionEndedSignature, ESpiderParkourAction, CompletedAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParkourObstacleDetectedSignature, ESpiderParkourAction, DetectedAction);

/**
 * Traversal ledge hit information
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderLedgeInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	FVector WallHitLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	FVector WallNormal = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	FVector LedgeTopLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	FVector LedgeNormal = FVector::UpVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	float ObstacleHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	float ObstacleDepth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	bool bHasClearance = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal")
	bool bIsValid = false;
};

/**
 * USpiderTraversalComponent
 * Handles intelligent environment scanning, multi-stage ledge raycasting,
 * obstacle classification (Vault, Mantle, Climb, Perch), and smooth procedural traversal execution.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderTraversalComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Scans environment ahead and returns detected ledge / obstacle info */
	UFUNCTION(BlueprintCallable, Category = "Spider|Traversal")
	bool DetectLedge(FSpiderLedgeInfo& OutLedgeInfo) const;

	/** Classifies detected ledge into a parkour action */
	UFUNCTION(BlueprintPure, Category = "Spider|Traversal")
	ESpiderParkourAction ClassifyObstacle(const FSpiderLedgeInfo& LedgeInfo) const;

	/** Attempts to initiate traversal on detected obstacle */
	UFUNCTION(BlueprintCallable, Category = "Spider|Traversal")
	bool TryPerformTraversal();

	/** Cancels ongoing traversal action */
	UFUNCTION(BlueprintCallable, Category = "Spider|Traversal")
	void CancelTraversal();

	/** Returns true if actively executing a traversal action */
	UFUNCTION(BlueprintPure, Category = "Spider|Traversal")
	bool IsTraversing() const { return bIsExecutingTraversal; }

	/** Get current active parkour action */
	UFUNCTION(BlueprintPure, Category = "Spider|Traversal")
	ESpiderParkourAction GetCurrentAction() const { return CurrentParkourAction; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Traversal|Events")
	FOnTraversalActionStartedSignature OnTraversalActionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Traversal|Events")
	FOnTraversalActionEndedSignature OnTraversalActionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Traversal|Events")
	FOnParkourObstacleDetectedSignature OnParkourObstacleDetected;

protected:
	// Trace Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Traces")
	float ForwardTraceDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Traces")
	float MaxLedgeDetectionHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Traces")
	float MinLedgeDetectionHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Traces")
	float SphereTraceRadius;

	// Obstacle Height & Depth Thresholds
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Thresholds")
	float LowVaultMaxHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Thresholds")
	float HighVaultMaxHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Thresholds")
	float MantleMaxHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Thresholds")
	float VaultMaxDepth;

	// Traversal Timing & Interpolation
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Config")
	float VaultDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Config")
	float MantleDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Traversal|Config")
	float ClimbUpDuration;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal|State")
	bool bIsExecutingTraversal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal|State")
	ESpiderParkourAction CurrentParkourAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal|State")
	FTransform TraversalStartTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal|State")
	FTransform TraversalTargetTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal|State")
	float TraversalProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traversal|State")
	float CurrentActionDuration;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;
	TWeakObjectPtr<USpiderMovementComponent> SpiderMovementComp;

	void UpdateTraversalExecution(float DeltaTime);
	void CompleteTraversal();
	bool CheckCapsuleClearance(const FVector& Location) const;
};
