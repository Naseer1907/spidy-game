// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/SpiderHeroTypes.h"
#include "IDamageable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IDamageable
 * Interface for actors and components that can receive damage, stagger, knockdown, and hit reactions.
 */
class SPIDERHERO_API IDamageable
{
	GENERATED_BODY()

public:
	/** Apply damage with hit reaction info */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Damage")
	float TakeDamageCustom(float DamageAmount, const FSpiderHitReaction& HitReaction);

	/** Returns true if the entity is currently alive */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Damage")
	bool IsAlive() const;

	/** Returns current health percentage from 0.0 to 1.0 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Damage")
	float GetHealthPercent() const;

	/** Applies stun / stagger duration */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Damage")
	void ApplyStun(float Duration);

	/** Applies knockdown launch physics */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Damage")
	void ApplyKnockdown(const FVector& LaunchVelocity);

	/** Applies directional hit reaction */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Damage")
	void ApplyHitReaction(const FSpiderHitReaction& HitReaction);

	/** Returns true if currently capable of receiving damage */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Damage")
	bool CanBeDamaged() const;
};
