// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/SpiderHeroTypes.h"
#include "ICombatTarget.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCombatTarget : public UInterface
{
	GENERATED_BODY()
};

/**
 * ICombatTarget
 * Interface for entities that can be targeted by Spider-Hero's auto-targeting,
 * lock-on camera, combat strike distance warping, and web strike systems.
 */
class SPIDERHERO_API ICombatTarget
{
	GENERATED_BODY()

public:
	/** Returns world location of the primary target point */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Target")
	FVector GetTargetLocation() const;

	/** Returns specific socket location for strikes (e.g. chest, head) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Target")
	FVector GetCombatSocketLocation(FName SocketName = NAME_None) const;

	/** Returns true if this target can currently be locked onto */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Target")
	bool CanBeLockedOn() const;

	/** Returns threat level integer (0 = None, 1 = Grunt, 2 = Elite/Brute, 3 = Boss) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Target")
	int32 GetTargetThreatLevel() const;

	/** Called when targeted by player camera or combat system */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Target")
	void OnTargeted(bool bIsTargeted, AActor* TargetingActor);

	/** Returns true if target is currently alive and active */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Target")
	bool IsCombatTargetValid() const;

	/** Returns bounding radius for combat spacing */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spider|Combat|Target")
	float GetTargetRadius() const;
};
