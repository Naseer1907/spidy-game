// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderVFXSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

/**
 * USpiderVFXSubsystem
 * World subsystem managing runtime Niagara particle dispatching for web ribbons,
 * impact shockwaves, Spider-Sense halos, and dynamic ground slams.
 */
UCLASS()
class SPIDERHERO_API USpiderVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USpiderVFXSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Spawns a web line / ribbon between two world points */
	UFUNCTION(BlueprintCallable, Category = "Spider|VFX")
	void SpawnWebBeamRibbon(const FVector& StartLocation, const FVector& EndLocation, float Duration = 0.8f);

	/** Spawns web impact splatter burst at hit location */
	UFUNCTION(BlueprintCallable, Category = "Spider|VFX")
	void SpawnWebImpactBurst(const FVector& Location, const FVector& Normal);

	/** Spawns a ground slam shockwave ring */
	UFUNCTION(BlueprintCallable, Category = "Spider|VFX")
	void SpawnGroundSlamShockwave(const FVector& Location, float Radius = 400.0f);

	/** Spawns Spider-Sense halo visual warning around hero head */
	UFUNCTION(BlueprintCallable, Category = "Spider|VFX")
	void SpawnSpiderSenseHalo(const FVector& HeadLocation, ESpiderSenseAlertLevel AlertLevel);

	/** Spawns combat finisher impact sparks & distortion */
	UFUNCTION(BlueprintCallable, Category = "Spider|VFX")
	void SpawnFinisherImpact(const FVector& Location, const FVector& Direction);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Spider|VFX|Systems")
	TObjectPtr<UNiagaraSystem> WebBeamSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Spider|VFX|Systems")
	TObjectPtr<UNiagaraSystem> WebImpactSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Spider|VFX|Systems")
	TObjectPtr<UNiagaraSystem> ShockwaveSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Spider|VFX|Systems")
	TObjectPtr<UNiagaraSystem> SpiderSenseSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Spider|VFX|Systems")
	TObjectPtr<UNiagaraSystem> FinisherImpactSystem;
};
