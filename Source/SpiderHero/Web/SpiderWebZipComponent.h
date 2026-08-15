// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderWebZipComponent.generated.h"

class ASpiderHeroCharacter;
class USpiderMovementComponent;
class USpiderWebTargetingComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZipStartedSignature, const FVector&, TargetLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZipCompletedSignature, bool, bIsPerched);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPointLaunchExecutedSignature, const FVector&, LaunchVelocity);

/**
 * USpiderWebZipComponent
 * High-speed Point Zip, Perch Zip, and Point Launch ballistic traversal system.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderWebZipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderWebZipComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initiates Point Zip towards targeted point or best candidate */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Zip")
	bool StartPointZip();

	/** Executes Point Launch (jump release during zip) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Zip")
	bool TryTriggerPointLaunch();

	/** Cancels zip travel */
	UFUNCTION(BlueprintCallable, Category = "Spider|Web|Zip")
	void CancelZip();

	/** Check if currently in zip transit */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Zip")
	bool IsZipping() const { return bIsZipping; }

	/** Check if Point Launch jump window is currently open */
	UFUNCTION(BlueprintPure, Category = "Spider|Web|Zip")
	bool IsPointLaunchWindowActive() const { return bPointLaunchWindowActive; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Zip|Events")
	FOnZipStartedSignature OnZipStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Zip|Events")
	FOnZipCompletedSignature OnZipCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Web|Zip|Events")
	FOnPointLaunchExecutedSignature OnPointLaunchExecuted;

protected:
	// Tuning
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Zip|Config")
	float ZipSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Zip|Config")
	float DecelerationDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Zip|Config")
	float PointLaunchWindowPercent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Zip|Launch")
	float PointLaunchForwardImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Web|Zip|Launch")
	float PointLaunchUpwardImpulse;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Zip|State")
	bool bIsZipping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Zip|State")
	bool bPointLaunchWindowActive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Zip|State")
	FVector ZipStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Zip|State")
	FVector ZipTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Zip|State")
	FVector ZipTargetNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Web|Zip|State")
	float TotalZipDistance;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;
	TWeakObjectPtr<USpiderMovementComponent> MovementComponent;
	TWeakObjectPtr<USpiderWebTargetingComponent> TargetingComponent;

	void UpdateZipTravel(float DeltaTime);
	void ArriveAtZipTarget();
};
