// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderCityGenerator.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

/**
 * Architectural styles for city skyscrapers and structures
 */
UENUM(BlueprintType)
enum class ESpiderBuildingStyle : uint8
{
	ModernGlass      = 0 UMETA(DisplayName = "Modern Glass Skyscraper"),
	ArtDeco          = 1 UMETA(DisplayName = "Art Deco Classic"),
	NeoGothic        = 2 UMETA(DisplayName = "Neo-Gothic Spire"),
	BrickIndustrial  = 3 UMETA(DisplayName = "Brick / Brownstone"),
	HighTechPlaza    = 4 UMETA(DisplayName = "High-Tech Commercial")
};

/**
 * Skyscraper mesh archetype definition
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderBuildingArchetype
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	ESpiderBuildingStyle Style = ESpiderBuildingStyle::ModernGlass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	TSoftObjectPtr<UStaticMesh> GroundFloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	TSoftObjectPtr<UStaticMesh> MidFloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	TSoftObjectPtr<UStaticMesh> TopFloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	TSoftObjectPtr<UStaticMesh> RoofCapMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	int32 MinFloors = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	int32 MaxFloors = 45;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	float FloorHeight = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Building")
	FVector FootprintSize = FVector(2000.0f, 2000.0f, 400.0f);
};

/**
 * City block data
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderCityBlock
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|World|Block")
	int32 GridX = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|World|Block")
	int32 GridY = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|World|Block")
	FVector CenterLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|World|Block")
	bool bIsParkPlaza = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|World|Block")
	TArray<FTransform> BuildingTransforms;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCityGenerationCompletedSignature, int32, TotalBuildingsSpawned);

/**
 * ASpiderCityGenerator
 * High-performance procedural superhero city district generator.
 * Features:
 * - Hierarchical Instanced Static Mesh (HISM) clustering for massive draw-call efficiency
 * - Skyscraper archetypes (Art Deco, Modern Glass, Neo-Gothic) with configurable floors
 * - Rooftop props: HVAC units, water towers, antennas, helipads, and roof access doors
 * - Grid street networks with sidewalks, crosswalks, alleyways, and park plazas
 * - Traversal surface collision tagging (ECC_GameTraceChannel2 & ECC_GameTraceChannel1)
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderCityGenerator : public AActor
{
	GENERATED_BODY()

public:
	ASpiderCityGenerator();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Generates the entire city district based on configured parameters */
	UFUNCTION(BlueprintCallable, Category = "Spider|World|Generator")
	void GenerateCityDistrict();

	/** Clears all instanced meshes and generated data */
	UFUNCTION(BlueprintCallable, Category = "Spider|World|Generator")
	void ClearCity();

	/** Returns generated city blocks */
	UFUNCTION(BlueprintPure, Category = "Spider|World|Generator")
	const TArray<FSpiderCityBlock>& GetCityBlocks() const { return CityBlocks; }

	/** Returns total building count */
	UFUNCTION(BlueprintPure, Category = "Spider|World|Generator")
	int32 GetTotalBuildingCount() const { return TotalBuildingsGenerated; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|World|Events")
	FOnCityGenerationCompletedSignature OnCityGenerationCompleted;

protected:
	// District Dimensions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Grid")
	int32 DistrictBlocksX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Grid")
	int32 DistrictBlocksY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Grid")
	float BlockSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Grid")
	float StreetWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Grid")
	float SidewalkWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Grid")
	int32 RandomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Grid")
	float ParkPlazaProbability;

	// Building Archetypes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Archetypes")
	TArray<FSpiderBuildingArchetype> BuildingArchetypes;

	// Instanced Mesh Assets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Meshes")
	TSoftObjectPtr<UStaticMesh> RoadStraightMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Meshes")
	TSoftObjectPtr<UStaticMesh> RoadIntersectionMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Meshes")
	TSoftObjectPtr<UStaticMesh> SidewalkMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Meshes")
	TSoftObjectPtr<UStaticMesh> ParkGrassMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Rooftop")
	TArray<TSoftObjectPtr<UStaticMesh>> HvacPropMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Rooftop")
	TArray<TSoftObjectPtr<UStaticMesh>> WaterTowerMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Rooftop")
	TArray<TSoftObjectPtr<UStaticMesh>> AntennaMeshes;

	// Optimization & Culling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Optimization")
	float HismCullingStartDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Optimization")
	float HismCullingEndDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Optimization")
	bool bEnableNanite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|World|Config|Optimization")
	bool bGenerateOnBeginPlay;

private:
	// HISM Component Cache
	UPROPERTY()
	TMap<FString, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> HismComponentMap;

	UPROPERTY()
	TArray<FSpiderCityBlock> CityBlocks;

	int32 TotalBuildingsGenerated;
	FRandomStream RandomStream;

	// Generation Pipeline Subroutines
	void SetupHismComponent(UHierarchicalInstancedStaticMeshComponent* HismComp);
	UHierarchicalInstancedStaticMeshComponent* GetOrCreateHism(UStaticMesh* Mesh, const FString& KeyPrefix);
	void GenerateRoadNetwork();
	void GenerateBlockGround(const FSpiderCityBlock& Block);
	void GenerateBlockBuildings(FSpiderCityBlock& Block);
	void GenerateSkyscraper(const FVector& BaseLocation, const FSpiderBuildingArchetype& Archetype, float YawRotation);
	void GenerateRooftopDetails(const FVector& RoofCenter, float BuildingWidth, float BuildingDepth);
	void ConfigureTraversalCollisions(UHierarchicalInstancedStaticMeshComponent* HismComp);
};
