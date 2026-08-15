// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderTrafficManager.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class USoundBase;

/**
 * Traffic Light States
 */
UENUM(BlueprintType)
enum class ESpiderTrafficLightState : uint8
{
	Green   = 0 UMETA(DisplayName = "Green (Go)"),
	Yellow  = 1 UMETA(DisplayName = "Yellow (Prepare to Stop)"),
	Red     = 2 UMETA(DisplayName = "Red (Stop)")
};

/**
 * Vehicle Archetypes
 */
UENUM(BlueprintType)
enum class ESpiderVehicleType : uint8
{
	Sedan   = 0 UMETA(DisplayName = "Civilian Sedan"),
	Taxi    = 1 UMETA(DisplayName = "City Yellow Cab"),
	SUV     = 2 UMETA(DisplayName = "SUV"),
	Bus     = 3 UMETA(DisplayName = "City Transit Bus"),
	Police  = 4 UMETA(DisplayName = "Police Cruiser")
};

/**
 * Traffic Lane Segment
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderTrafficLane
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	int32 LaneId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	FVector StartPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	FVector EndPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	float SpeedLimit = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	int32 TargetIntersectionId = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	int32 NextLaneId = -1;
};

/**
 * Traffic Intersection
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderTrafficIntersection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	int32 IntersectionId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic")
	FVector CenterLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	ESpiderTrafficLightState NorthSouthLight = ESpiderTrafficLightState::Green;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	ESpiderTrafficLightState EastWestLight = ESpiderTrafficLightState::Red;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	float StateTimer = 0.0f;
};

/**
 * Lightweight Pooled Vehicle Record
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderSimulatedVehicle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	int32 VehicleId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	ESpiderVehicleType VehicleType = ESpiderVehicleType::Sedan;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	int32 InstanceIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	float CurrentSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	int32 CurrentLaneId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	float LaneDistanceRatio = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	bool bIsStopped = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	bool bIsHonking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Traffic")
	bool bIsWrecked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleImpactSignature, int32, VehicleId, const FVector&, ImpactForce);

/**
 * ASpiderTrafficManager
 * High-performance city traffic simulation managing:
 * - Instanced vehicle pooling along grid lane networks
 * - Synchronized 4-way traffic intersections with signal cycles
 * - Obstacle detection, pedestrian/hero avoidance, and honking
 * - Physical collision and wreck reactions when impacted by superhero combat
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderTrafficManager : public AActor
{
	GENERATED_BODY()

public:
	ASpiderTrafficManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Initializes lane network and populates initial vehicle pool */
	UFUNCTION(BlueprintCallable, Category = "Spider|Traffic")
	void InitializeTraffic(const FVector& CityCenter, int32 GridDimension, float BlockStride);

	/** Applies impact force to a vehicle (e.g. from combat or web throws) */
	UFUNCTION(BlueprintCallable, Category = "Spider|Traffic")
	void DamageVehicle(int32 VehicleId, const FVector& ImpactForce);

	/** Returns total active vehicles */
	UFUNCTION(BlueprintPure, Category = "Spider|Traffic")
	int32 GetActiveVehicleCount() const { return ActiveVehicles.Num(); }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|Traffic|Events")
	FOnVehicleImpactSignature OnVehicleImpact;

protected:
	// Traffic Tunables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Config")
	int32 MaxVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Config")
	float VehicleDespawnDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Config")
	float SafeFollowingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Config")
	float GreenLightDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Config")
	float YellowLightDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Config")
	float RedLightDuration;

	// Vehicle Meshes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Meshes")
	TSoftObjectPtr<UStaticMesh> SedanMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Meshes")
	TSoftObjectPtr<UStaticMesh> TaxiMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Meshes")
	TSoftObjectPtr<UStaticMesh> SuvMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Meshes")
	TSoftObjectPtr<UStaticMesh> BusMesh;

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Audio")
	TSoftObjectPtr<USoundBase> CarHonkSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Traffic|Audio")
	TSoftObjectPtr<USoundBase> CrashSFX;

private:
	UPROPERTY()
	TMap<ESpiderVehicleType, TObjectPtr<UInstancedStaticMeshComponent>> VehicleIsmMap;

	UPROPERTY()
	TArray<FSpiderTrafficLane> TrafficLanes;

	UPROPERTY()
	TArray<FSpiderTrafficIntersection> Intersections;

	UPROPERTY()
	TArray<FSpiderSimulatedVehicle> ActiveVehicles;

	int32 NextVehicleId;

	// Simulation Subroutines
	void UpdateIntersections(float DeltaSeconds);
	void UpdateVehicles(float DeltaSeconds);
	void CheckObstaclesAndLeadVehicles(FSpiderSimulatedVehicle& Vehicle, float DeltaSeconds);
	void RecycleVehiclesAroundPlayer();
	void SpawnVehicleOnLane(int32 LaneId);
	UInstancedStaticMeshComponent* GetIsmForType(ESpiderVehicleType Type);
};
