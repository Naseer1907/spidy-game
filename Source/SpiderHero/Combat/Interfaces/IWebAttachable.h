// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IWebAttachable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UWebAttachable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IWebAttachable
 * Interface for actors and components that can have web lines attached, be pulled,
 * accumulate web gauge, become web-bound, or get pinned to walls/surfaces.
 */
class SPIDERHERO_API IWebAttachable
{
	GENERATED_BODY()

public:
	/** Returns true if this entity currently allows web attachment */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	bool CanAttachWeb() const;

	/** Called when a web projectile or line attaches to this actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	void OnWebAttached(AActor* AttachingActor, const FVector& AttachPoint);

	/** Called when pulled by a web line (yank or strike) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	void OnWebPulled(AActor* PullingActor, const FVector& PullDirection, float Force);

	/** Called when web gauge fills up and entity becomes immobilized/bound */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	void OnWebBound(float Duration, AActor* InstigatorActor);

	/** Adds web accumulation gauge (0.0 to 100.0) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	void ApplyWebGauge(float Amount, AActor* InstigatorActor);

	/** Returns current web accumulation percentage (0.0 to 1.0) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	float GetWebGaugePercent() const;

	/** Returns true if currently web bound / incapacitated */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	bool IsWebBound() const;

	/** Returns preferred web attachment socket or world point */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	FVector GetWebAnchorPoint() const;

	/** Attempts to pin the web-bound entity to a nearby surface / wall */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Web")
	bool TryPinToSurface(const FVector& SurfaceLocation, const FVector& SurfaceNormal);
};
