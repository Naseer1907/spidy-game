// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderWebTargetingComponent.generated.h"

class ASpiderHeroCharacter;
class ASpiderHeroPlayerController;
class UCameraComponent;

/**
 * Candidate web anchor target with scoring metadata
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderWebTargetCandidate
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	FVector SurfaceNormal = FVector::UpVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	float Score = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	float Distance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	float AngleDegrees = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	TWeakObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	TWeakObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	bool bIsOnScreen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	bool bIsSwingAnchor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	bool bIsZipAnchor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting")
	bool bIsValid = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebTargetUpdatedSignature, const FSpiderWebTargetCandidate&, BestTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebTargetLostSignature);

/**
 * USpiderWebTargetingComponent
 * Intelligent raycast and frustum querying subsystem finding optimal web anchor points for swinging and point zips.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderWebTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderWebTargetingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Finds the best swing anchor point in current view cone */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Targeting")
	bool FindBestSwingAnchor(FSpiderWebAnchorInfo& OutAnchorInfo);

	/** Finds the best point zip target */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Targeting")
	bool FindBestZipTarget(FSpiderWebTargetCandidate& OutZipTarget);

	/** Get current cached best swing target */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Targeting")
	const FSpiderWebTargetCandidate& GetCurrentSwingTarget() const { return CurrentBestSwingTarget; }

	/** Get current cached best zip target */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Targeting")
	const FSpiderWebTargetCandidate& GetCurrentZipTarget() const { return CurrentBestZipTarget; }

	/** Projects world location to screen space */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Targeting")
	bool ProjectWorldToScreen(const FVector& WorldLoc, FVector2D& OutScreenPos) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Targeting|Events")
	FOnWebTargetUpdatedSignature OnBestSwingTargetUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Targeting|Events")
	FOnWebTargetUpdatedSignature OnBestZipTargetUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Targeting|Events")
	FOnWebTargetLostSignature OnSwingTargetLost;

protected:
	// Frustum & Raycast Cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Config")
	float MaxSwingDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Config")
	float MinSwingDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Config")
	float MaxZipDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Config")
	float MaxTargetingAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Config")
	int32 FrustumRaycastCount;

	// Weightings for scoring algorithm
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Weights")
	float AngleWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Weights")
	float DistanceWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Targeting|Weights")
	float HeightBonusWeight;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting|State")
	FSpiderWebTargetCandidate CurrentBestSwingTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Targeting|State")
	FSpiderWebTargetCandidate CurrentBestZipTarget;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;
	TWeakObjectPtr<ASpiderHeroPlayerController> PlayerControllerOwner;

	float EvaluateAnchorScore(const FVector& PlayerLoc, const FVector& ViewForward, const FHitResult& Hit, bool bForSwinging) const;
	void ScanEnvironmentForTargets();
};
