// Copyright SpiderHero Team. All Rights Reserved.

#include "World/SpiderCityGenerator.h"
#include "SpiderHero.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

ASpiderCityGenerator::ASpiderCityGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	DistrictBlocksX = 4;
	DistrictBlocksY = 4;
	BlockSize = 8000.0f;
	StreetWidth = 1400.0f;
	SidewalkWidth = 400.0f;
	RandomSeed = 1337;
	ParkPlazaProbability = 0.15f;

	HismCullingStartDistance = 45000.0f;
	HismCullingEndDistance = 65000.0f;
	bEnableNanite = true;
	bGenerateOnBeginPlay = true;

	TotalBuildingsGenerated = 0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	// Setup default fallback archetypes
	FSpiderBuildingArchetype ModernArchetype;
	ModernArchetype.Style = ESpiderBuildingStyle::ModernGlass;
	ModernArchetype.MinFloors = 15;
	ModernArchetype.MaxFloors = 40;
	ModernArchetype.FloorHeight = 400.0f;
	ModernArchetype.FootprintSize = FVector(2400.0f, 2400.0f, 400.0f);
	BuildingArchetypes.Add(ModernArchetype);

	FSpiderBuildingArchetype ArtDecoArchetype;
	ArtDecoArchetype.Style = ESpiderBuildingStyle::ArtDeco;
	ArtDecoArchetype.MinFloors = 10;
	ArtDecoArchetype.MaxFloors = 30;
	ArtDecoArchetype.FloorHeight = 380.0f;
	ArtDecoArchetype.FootprintSize = FVector(2000.0f, 2000.0f, 380.0f);
	BuildingArchetypes.Add(ArtDecoArchetype);

	FSpiderBuildingArchetype NeoGothicArchetype;
	NeoGothicArchetype.Style = ESpiderBuildingStyle::NeoGothic;
	NeoGothicArchetype.MinFloors = 12;
	NeoGothicArchetype.MaxFloors = 35;
	NeoGothicArchetype.FloorHeight = 420.0f;
	NeoGothicArchetype.FootprintSize = FVector(2200.0f, 2200.0f, 420.0f);
	BuildingArchetypes.Add(NeoGothicArchetype);
}

void ASpiderCityGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		GenerateCityDistrict();
	}
}

void ASpiderCityGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ASpiderCityGenerator::ClearCity()
{
	for (auto& Pair : HismComponentMap)
	{
		if (Pair.Value)
		{
			Pair.Value->ClearInstances();
		}
	}

	CityBlocks.Empty();
	TotalBuildingsGenerated = 0;
	UE_LOG(LogSpiderWorld, Log, TEXT("SpiderCityGenerator cleared all instances."));
}

void ASpiderCityGenerator::GenerateCityDistrict()
{
	ClearCity();
	RandomStream.Initialize(RandomSeed);

	UE_LOG(LogSpiderWorld, Log, TEXT("Generating SpiderHero City District: %dx%d blocks..."), DistrictBlocksX, DistrictBlocksY);

	// 1. Generate Road Grid
	GenerateRoadNetwork();

	// 2. Generate Blocks, Sidewalks, Plazas, and Skyscrapers
	const float HalfDistrictWidth = (DistrictBlocksX * (BlockSize + StreetWidth)) * 0.5f;
	const float HalfDistrictDepth = (DistrictBlocksY * (BlockSize + StreetWidth)) * 0.5f;

	for (int32 X = 0; X < DistrictBlocksX; ++X)
	{
		for (int32 Y = 0; Y < DistrictBlocksY; ++Y)
		{
			FSpiderCityBlock Block;
			Block.GridX = X;
			Block.GridY = Y;

			const float CenterX = (X * (BlockSize + StreetWidth)) - HalfDistrictWidth + (BlockSize * 0.5f);
			const float CenterY = (Y * (BlockSize + StreetWidth)) - HalfDistrictDepth + (BlockSize * 0.5f);
			Block.CenterLocation = GetActorLocation() + FVector(CenterX, CenterY, 0.0f);

			// Decide if this block is a park plaza
			const float ParkRoll = RandomStream.FRand();
			Block.bIsParkPlaza = (ParkRoll < ParkPlazaProbability);

			GenerateBlockGround(Block);

			if (!Block.bIsParkPlaza)
			{
				GenerateBlockBuildings(Block);
			}

			CityBlocks.Add(Block);
		}
	}

	UE_LOG(LogSpiderWorld, Log, TEXT("City District Generation Complete! Total Buildings: %d, Total Blocks: %d"), TotalBuildingsGenerated, CityBlocks.Num());
	OnCityGenerationCompleted.Broadcast(TotalBuildingsGenerated);
}

void ASpiderCityGenerator::GenerateRoadNetwork()
{
	UStaticMesh* StraightRoad = RoadStraightMesh.Get();
	UStaticMesh* Crossroad = RoadIntersectionMesh.Get();

	if (!StraightRoad && !Crossroad)
	{
		return;
	}

	const float TotalStride = BlockSize + StreetWidth;
	const float HalfDistrictWidth = (DistrictBlocksX * TotalStride) * 0.5f;
	const float HalfDistrictDepth = (DistrictBlocksY * TotalStride) * 0.5f;

	// Intersections
	if (Crossroad)
	{
		UHierarchicalInstancedStaticMeshComponent* HismCross = GetOrCreateHism(Crossroad, TEXT("Road_Intersection"));
		for (int32 X = 0; X <= DistrictBlocksX; ++X)
		{
			for (int32 Y = 0; Y <= DistrictBlocksY; ++Y)
			{
				const float PosX = (X * TotalStride) - HalfDistrictWidth - (StreetWidth * 0.5f);
				const float PosY = (Y * TotalStride) - HalfDistrictDepth - (StreetWidth * 0.5f);
				const FTransform IntersectionTransform(FRotator::ZeroRotator, GetActorLocation() + FVector(PosX, PosY, 0.0f), FVector::OneVector);
				HismCross->AddInstance(IntersectionTransform);
			}
		}
	}

	// Straight Roads
	if (StraightRoad)
	{
		UHierarchicalInstancedStaticMeshComponent* HismStraight = GetOrCreateHism(StraightRoad, TEXT("Road_Straight"));
		for (int32 X = 0; X < DistrictBlocksX; ++X)
		{
			for (int32 Y = 0; Y <= DistrictBlocksY; ++Y)
			{
				const float PosX = (X * TotalStride) - HalfDistrictWidth + (BlockSize * 0.5f);
				const float PosY = (Y * TotalStride) - HalfDistrictDepth - (StreetWidth * 0.5f);
				const FTransform RoadTransform(FRotator(0.0f, 0.0f, 0.0f), GetActorLocation() + FVector(PosX, PosY, 0.0f), FVector(BlockSize / 1000.0f, StreetWidth / 1000.0f, 1.0f));
				HismStraight->AddInstance(RoadTransform);
			}
		}

		for (int32 X = 0; X <= DistrictBlocksX; ++X)
		{
			for (int32 Y = 0; Y < DistrictBlocksY; ++Y)
			{
				const float PosX = (X * TotalStride) - HalfDistrictWidth - (StreetWidth * 0.5f);
				const float PosY = (Y * TotalStride) - HalfDistrictDepth + (BlockSize * 0.5f);
				const FTransform RoadTransform(FRotator(0.0f, 90.0f, 0.0f), GetActorLocation() + FVector(PosX, PosY, 0.0f), FVector(BlockSize / 1000.0f, StreetWidth / 1000.0f, 1.0f));
				HismStraight->AddInstance(RoadTransform);
			}
		}
	}
}

void ASpiderCityGenerator::GenerateBlockGround(const FSpiderCityBlock& Block)
{
	UStaticMesh* GroundMesh = Block.bIsParkPlaza ? ParkGrassMesh.Get() : SidewalkMesh.Get();
	if (!GroundMesh)
	{
		return;
	}

	const FString Key = Block.bIsParkPlaza ? TEXT("Park_Ground") : TEXT("Sidewalk_Ground");
	UHierarchicalInstancedStaticMeshComponent* HismGround = GetOrCreateHism(GroundMesh, Key);

	const FTransform GroundTransform(FRotator::ZeroRotator, Block.CenterLocation, FVector(BlockSize / 1000.0f, BlockSize / 1000.0f, 0.2f));
	HismGround->AddInstance(GroundTransform);
}

void ASpiderCityGenerator::GenerateBlockBuildings(FSpiderCityBlock& Block)
{
	if (BuildingArchetypes.Num() == 0)
	{
		return;
	}

	// 2x2 Sub-lot grid per city block
	const float SubLotSize = (BlockSize - SidewalkWidth * 2.0f) * 0.5f;
	const float Offsets[2] = { -SubLotSize * 0.5f, SubLotSize * 0.5f };

	for (int32 SubX = 0; SubX < 2; ++SubX)
	{
		for (int32 SubY = 0; SubY < 2; ++SubY)
		{
			// Chance of alleyway opening or open courtyard
			if (RandomStream.FRand() < 0.15f)
			{
				continue;
			}

			const FVector BuildingPos = Block.CenterLocation + FVector(Offsets[SubX], Offsets[SubY], 0.0f);
			const int32 ArchetypeIndex = RandomStream.RandRange(0, BuildingArchetypes.Num() - 1);
			const float BuildingYaw = RandomStream.RandRange(0, 3) * 90.0f;

			GenerateSkyscraper(BuildingPos, BuildingArchetypes[ArchetypeIndex], BuildingYaw);
			Block.BuildingTransforms.Add(FTransform(FRotator(0.0f, BuildingYaw, 0.0f), BuildingPos, FVector::OneVector));
			TotalBuildingsGenerated++;
		}
	}
}

void ASpiderCityGenerator::GenerateSkyscraper(const FVector& BaseLocation, const FSpiderBuildingArchetype& Archetype, float YawRotation)
{
	const int32 FloorCount = RandomStream.RandRange(Archetype.MinFloors, Archetype.MaxFloors);
	const FRotator BuildingRot(0.0f, YawRotation, 0.0f);

	UStaticMesh* GroundMesh = Archetype.GroundFloorMesh.Get();
	UStaticMesh* MidMesh = Archetype.MidFloorMesh.Get();
	UStaticMesh* TopMesh = Archetype.TopFloorMesh.Get();
	UStaticMesh* RoofMesh = Archetype.RoofCapMesh.Get();

	float CurrentHeight = 0.0f;

	// Ground Floor
	if (GroundMesh)
	{
		UHierarchicalInstancedStaticMeshComponent* HismGround = GetOrCreateHism(GroundMesh, TEXT("Building_Ground"));
		HismGround->AddInstance(FTransform(BuildingRot, BaseLocation + FVector(0.0f, 0.0f, CurrentHeight), FVector::OneVector));
	}
	CurrentHeight += Archetype.FloorHeight;

	// Mid Floors
	if (MidMesh)
	{
		UHierarchicalInstancedStaticMeshComponent* HismMid = GetOrCreateHism(MidMesh, TEXT("Building_Mid"));
		for (int32 f = 1; f < FloorCount - 1; ++f)
		{
			HismMid->AddInstance(FTransform(BuildingRot, BaseLocation + FVector(0.0f, 0.0f, CurrentHeight), FVector::OneVector));
			CurrentHeight += Archetype.FloorHeight;
		}
	}
	else
	{
		CurrentHeight += (FloorCount - 2) * Archetype.FloorHeight;
	}

	// Top Floor
	if (TopMesh)
	{
		UHierarchicalInstancedStaticMeshComponent* HismTop = GetOrCreateHism(TopMesh, TEXT("Building_Top"));
		HismTop->AddInstance(FTransform(BuildingRot, BaseLocation + FVector(0.0f, 0.0f, CurrentHeight), FVector::OneVector));
	}
	CurrentHeight += Archetype.FloorHeight;

	// Roof Cap
	if (RoofMesh)
	{
		UHierarchicalInstancedStaticMeshComponent* HismRoof = GetOrCreateHism(RoofMesh, TEXT("Building_Roof"));
		HismRoof->AddInstance(FTransform(BuildingRot, BaseLocation + FVector(0.0f, 0.0f, CurrentHeight), FVector::OneVector));
	}

	// Rooftop Details & Props
	GenerateRooftopDetails(BaseLocation + FVector(0.0f, 0.0f, CurrentHeight), Archetype.FootprintSize.X, Archetype.FootprintSize.Y);
}

void ASpiderCityGenerator::GenerateRooftopDetails(const FVector& RoofCenter, float BuildingWidth, float BuildingDepth)
{
	// 1. HVAC Units
	if (HvacPropMeshes.Num() > 0)
	{
		const int32 HvacCount = RandomStream.RandRange(1, 3);
		for (int32 i = 0; i < HvacCount; ++i)
		{
			const int32 Index = RandomStream.RandRange(0, HvacPropMeshes.Num() - 1);
			if (UStaticMesh* Mesh = HvacPropMeshes[Index].Get())
			{
				UHierarchicalInstancedStaticMeshComponent* HismHvac = GetOrCreateHism(Mesh, TEXT("Roof_HVAC"));
				const float OffsetX = RandomStream.FRandRange(-BuildingWidth * 0.3f, BuildingWidth * 0.3f);
				const float OffsetY = RandomStream.FRandRange(-BuildingDepth * 0.3f, BuildingDepth * 0.3f);
				const FRotator Rot(0.0f, RandomStream.RandRange(0, 3) * 90.0f, 0.0f);
				HismHvac->AddInstance(FTransform(Rot, RoofCenter + FVector(OffsetX, OffsetY, 0.0f), FVector::OneVector));
			}
		}
	}

	// 2. Water Tower
	if (WaterTowerMeshes.Num() > 0 && RandomStream.FRand() < 0.45f)
	{
		const int32 Index = RandomStream.RandRange(0, WaterTowerMeshes.Num() - 1);
		if (UStaticMesh* Mesh = WaterTowerMeshes[Index].Get())
		{
			UHierarchicalInstancedStaticMeshComponent* HismTower = GetOrCreateHism(Mesh, TEXT("Roof_WaterTower"));
			const float OffsetX = RandomStream.FRandRange(-BuildingWidth * 0.25f, BuildingWidth * 0.25f);
			const float OffsetY = RandomStream.FRandRange(-BuildingDepth * 0.25f, BuildingDepth * 0.25f);
			HismTower->AddInstance(FTransform(FRotator(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f), RoofCenter + FVector(OffsetX, OffsetY, 0.0f), FVector::OneVector));
		}
	}

	// 3. Spire / Antenna on Tall Skyscraper
	if (AntennaMeshes.Num() > 0 && RandomStream.FRand() < 0.5f)
	{
		const int32 Index = RandomStream.RandRange(0, AntennaMeshes.Num() - 1);
		if (UStaticMesh* Mesh = AntennaMeshes[Index].Get())
		{
			UHierarchicalInstancedStaticMeshComponent* HismAntenna = GetOrCreateHism(Mesh, TEXT("Roof_Antenna"));
			HismAntenna->AddInstance(FTransform(FRotator::ZeroRotator, RoofCenter, FVector::OneVector));
		}
	}
}

UHierarchicalInstancedStaticMeshComponent* ASpiderCityGenerator::GetOrCreateHism(UStaticMesh* Mesh, const FString& KeyPrefix)
{
	if (!Mesh)
	{
		return nullptr;
	}

	const FString Key = FString::Printf(TEXT("%s_%s"), *KeyPrefix, *Mesh->GetName());
	if (TObjectPtr<UHierarchicalInstancedStaticMeshComponent>* FoundComp = HismComponentMap.Find(Key))
	{
		if (*FoundComp)
		{
			return *FoundComp;
		}
	}

	UHierarchicalInstancedStaticMeshComponent* NewHism = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, *Key);
	NewHism->SetStaticMesh(Mesh);
	NewHism->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewHism->RegisterComponent();

	SetupHismComponent(NewHism);
	ConfigureTraversalCollisions(NewHism);

	HismComponentMap.Add(Key, NewHism);
	return NewHism;
}

void ASpiderCityGenerator::SetupHismComponent(UHierarchicalInstancedStaticMeshComponent* HismComp)
{
	if (!HismComp)
	{
		return;
	}

	HismComp->SetCullDistances(HismCullingStartDistance, HismCullingEndDistance);
	HismComp->bEnableDensityScaling = true;
	HismComp->SetMobility(EComponentMobility::Static);
	HismComp->SetGenerateOverlapEvents(false);
}

void ASpiderCityGenerator::ConfigureTraversalCollisions(UHierarchicalInstancedStaticMeshComponent* HismComp)
{
	if (!HismComp)
	{
		return;
	}

	HismComp->SetCollisionProfileName(TEXT("BlockAll"));
	HismComp->SetCollisionResponseToChannel(SPIDER_TRACE_WALL_SURFACE, ECR_Block);
	HismComp->SetCollisionResponseToChannel(SPIDER_TRACE_WEB_ANCHOR, ECR_Block);
	HismComp->SetCollisionResponseToChannel(SPIDER_TRACE_ZIPLINE_ANCHOR, ECR_Block);
	HismComp->SetCollisionResponseToChannel(SPIDER_TRACE_PARKOUR_OBSTACLE, ECR_Block);
}
