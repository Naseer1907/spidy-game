// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderAbilityComponent.generated.h"

class ASpiderHeroCharacter;
class USpiderCameraComponent;
class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class ESpiderWebAbilityType : uint8
{
	WebShot      = 0 UMETA(DisplayName = "Web Shot"),
	WebBurst     = 1 UMETA(DisplayName = "Web Burst"),
	WebPull      = 2 UMETA(DisplayName = "Web Pull"),
	WebWallPin   = 3 UMETA(DisplayName = "Web Wall Pin")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebFluidChangedSignature, float, CurrentFluid, float, MaxFluid, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebAbilityFiredSignature, ESpiderWebAbilityType, AbilityType, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyWebBoundSignature, AActor*, BoundEnemy, bool, bIsPinnedToWall);

/**
 * USpiderAbilityComponent
 * Manages superhero web gadgetry and abilities:
 * - Web Shot: rapid dart applying web gauge
 * - Web Burst: radial explosion knocking back and binding enemies
 * - Web Pull: yank light enemies or launch toward heavy targets
 * - Web Wall Pin: incapacitating web-bound enemies against geometry
 * - Resource & cooldown management
 */
UCLASS(ClassGroup = (SpiderAbilities), meta = (BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderAbilityComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Ability Executions
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Web")
	bool FireWebShot(AActor* OptionalTarget = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Web")
	bool FireWebBurst();

	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Web")
	bool FireWebPull(AActor* OptionalTarget = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Web")
	bool TryPinTargetToWall(AActor* TargetActor);

	// Resource Management
	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Resource")
	bool ConsumeWebFluid(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Spider|Abilities|Resource")
	void RestoreWebFluid(float Amount);

	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Resource")
	float GetCurrentWebFluid() const { return CurrentWebFluid; }

	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Resource")
	float GetMaxWebFluid() const { return MaxWebFluid; }

	UFUNCTION(BlueprintPure, Category = "Spider|Abilities|Resource")
	float GetWebFluidPercent() const { return MaxWebFluid > 0.0f ? CurrentWebFluid / MaxWebFluid : 0.0f; }

	// Event Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Events")
	FOnWebFluidChangedSignature OnWebFluidChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Events")
	FOnWebAbilityFiredSignature OnWebAbilityFired;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Abilities|Events")
	FOnEnemyWebBoundSignature OnEnemyWebBound;

protected:
	// Resource Config
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|Config")
	float MaxWebFluid;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Abilities|Config")
	float CurrentWebFluid;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|Config")
	float WebFluidRegenRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|Config")
	float WebFluidRegenDelay;

	// Web Shot Config
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebShot")
	float WebShotFluidCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebShot")
	float WebShotRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebShot")
	float WebShotGaugeAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebShot")
	float WebShotDamage;

	// Web Burst Config
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebBurst")
	float WebBurstFluidCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebBurst")
	float WebBurstRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebBurst")
	float WebBurstKnockbackForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebBurst")
	float WebBurstGaugeAmount;

	// Web Pull Config
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebPull")
	float WebPullFluidCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebPull")
	float WebPullRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebPull")
	float WebPullYankForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WebPull")
	float WebStrikeLaunchSpeed;

	// Wall Pin Config
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WallPin")
	float WallPinTraceDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Abilities|WallPin")
	float WallPinDuration;

private:
	UPROPERTY()
	TWeakObjectPtr<ASpiderHeroCharacter> OwnerCharacter;

	UPROPERTY()
	TWeakObjectPtr<USpiderCameraComponent> CameraComponent;

	float TimeSinceLastFluidConsume;

	AActor* FindBestAbilityTarget(float MaxRange) const;
	bool CheckNearbyWall(const FVector& Location, FHitResult& OutWallHit) const;
};
