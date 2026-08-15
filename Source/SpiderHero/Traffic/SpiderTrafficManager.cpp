// Copyright SpiderHero Team. All Rights Reserved.

#include "Traffic/SpiderTrafficManager.h"
#include "SpiderHero.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

ASpiderTrafficManager::ASpiderTrafficManager()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxVehicles = 40;
	VehicleDespawnDistance = 25000.0f;
	SafeFollowingDistance = 650.0f;

	GreenLightDuration = 12.0f;
	YellowLightDuration = 3.0f;
	RedLightDuration = 15.0f;

	NextVehicleId = 1;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
}

void ASpiderTrafficManager::BeginPlay()
{
	Super::BeginPlay();

	// Initialize default traffic grid if not called externally
	InitializeTraffic(GetActorLocation(), 4, 9400.0f);
	UE_LOG(LogSpiderTraffic, Log, TEXT("SpiderTrafficManager initialized with %d lanes."), TrafficLanes.Num());
}

void ASpiderTrafficManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateIntersections(DeltaSeconds);
	UpdateVehicles(DeltaSeconds);
	RecycleVehiclesAroundPlayer();
}

void ASpiderTrafficManager::InitializeTraffic(const FVector& CityCenter, int32 GridDimension, float BlockStride)
{
	TrafficLanes.Empty();
	Intersections.Empty();
	ActiveVehicles.Empty();

	const float HalfExtent = (GridDimension * BlockStride) * 0.5f;
	int32 LaneIdCounter = 0;
	int32 IntersectIdCounter = 0;

	// Generate Intersections
	for (int32 X = 0; X <= GridDimension; ++X)
	{
		for (int32 Y = 0; Y <= GridDimension; ++Y)
		{
			FSpiderTrafficIntersection Intersect;
			Intersect.IntersectionId = IntersectIdCounter++;
			Intersect.CenterLocation = CityCenter + FVector((X * BlockStride) - HalfExtent, (Y * BlockStride) - HalfExtent, 0.0f);
			Intersect.NorthSouthLight = (X % 2 == 0) ? ESpiderTrafficLightState::Green : ESpiderTrafficLightState::Red;
			Intersect.EastWestLight = (Intersect.NorthSouthLight == ESpiderTrafficLightState::Green) ? ESpiderTrafficLightState::Red : ESpiderTrafficLightState::Green;
			Intersect.StateTimer = 0.0f;
			Intersections.Add(Intersect);
		}
	}

	// Generate Lanes connecting intersections
	for (int32 X = 0; X < GridDimension; ++X)
	{
		for (int32 Y = 0; Y <= GridDimension; ++Y)
		{
			// Eastbound Lane
			FSpiderTrafficLane LaneE;
			LaneE.LaneId = LaneIdCounter++;
			LaneE.StartPoint = CityCenter + FVector((X * BlockStride) - HalfExtent + 400.0f, (Y * BlockStride) - HalfExtent + 250.0f, 0.0f);
			LaneE.EndPoint = CityCenter + FVector(((X + 1) * BlockStride) - HalfExtent - 400.0f, (Y * BlockStride) - HalfExtent + 250.0f, 0.0f);
			TrafficLanes.Add(LaneE);

			// Westbound Lane
			FSpiderTrafficLane LaneW;
			LaneW.LaneId = LaneIdCounter++;
			LaneW.StartPoint = CityCenter + FVector(((X + 1) * BlockStride) - HalfExtent - 400.0f, (Y * BlockStride) - HalfExtent - 250.0f, 0.0f);
			LaneW.EndPoint = CityCenter + FVector((X * BlockStride) - HalfExtent + 400.0f, (Y * BlockStride) - HalfExtent - 250.0f, 0.0f);
			TrafficLanes.Add(LaneW);
		}
	}

	for (int32 X = 0; X <= GridDimension; ++X)
	{
		for (int32 Y = 0; Y < GridDimension; ++Y)
		{
			// Northbound Lane
			FSpiderTrafficLane LaneN;
			LaneN.LaneId = LaneIdCounter++;
			LaneN.StartPoint = CityCenter + FVector((X * BlockStride) - HalfExtent - 250.0f, (Y * BlockStride) - HalfExtent + 400.0f, 0.0f);
			LaneN.EndPoint = CityCenter + FVector((X * BlockStride) - HalfExtent - 250.0f, ((Y + 1) * BlockStride) - HalfExtent - 400.0f, 0.0f);
			TrafficLanes.Add(LaneN);

			// Southbound Lane
			FSpiderTrafficLane LaneS;
			LaneS.LaneId = LaneIdCounter++;
			LaneS.StartPoint = CityCenter + FVector((X * BlockStride) - HalfExtent + 250.0f, ((Y + 1) * BlockStride) - HalfExtent - 400.0f, 0.0f);
			LaneS.EndPoint = CityCenter + FVector((X * BlockStride) - HalfExtent + 250.0f, (Y * BlockStride) - HalfExtent + 400.0f, 0.0f);
			TrafficLanes.Add(LaneS);
		}
	}

	// Spawn Initial Vehicles
	const int32 InitialCount = FMath::Min(MaxVehicles, TrafficLanes.Num());
	for (int32 i = 0; i < InitialCount; ++i)
	{
		SpawnVehicleOnLane(i % TrafficLanes.Num());
	}
}

void ASpiderTrafficManager::SpawnVehicleOnLane(int32 LaneId)
{
	if (!TrafficLanes.IsValidIndex(LaneId) || ActiveVehicles.Num() >= MaxVehicles)
	{
		return;
	}

	const FSpiderTrafficLane& Lane = TrafficLanes[LaneId];
	const ESpiderVehicleType VType = static_cast<ESpiderVehicleType>(FMath::RandRange(0, 3));
	UInstancedStaticMeshComponent* Ism = GetIsmForType(VType);

	if (!Ism)
	{
		return;
	}

	FSpiderSimulatedVehicle Vehicle;
	Vehicle.VehicleId = NextVehicleId++;
	Vehicle.VehicleType = VType;
	Vehicle.CurrentLaneId = LaneId;
	Vehicle.LaneDistanceRatio = FMath::FRandRange(0.05f, 0.9f);
	Vehicle.CurrentSpeed = FMath::FRandRange(600.0f, Lane.SpeedLimit);

	const FVector Pos = FMath::Lerp(Lane.StartPoint, Lane.EndPoint, Vehicle.LaneDistanceRatio);
	const FRotator Rot = (Lane.EndPoint - Lane.StartPoint).Rotation();
	Vehicle.Transform = FTransform(Rot, Pos, FVector::OneVector);

	Vehicle.InstanceIndex = Ism->AddInstance(Vehicle.Transform);
	ActiveVehicles.Add(Vehicle);
}

void ASpiderTrafficManager::UpdateIntersections(float DeltaSeconds)
{
	for (FSpiderTrafficIntersection& Intersect : Intersections)
	{
		Intersect.StateTimer += DeltaSeconds;

		if (Intersect.NorthSouthLight == ESpiderTrafficLightState::Green && Intersect.StateTimer >= GreenLightDuration)
		{
			Intersect.NorthSouthLight = ESpiderTrafficLightState::Yellow;
			Intersect.StateTimer = 0.0f;
		}
		else if (Intersect.NorthSouthLight == ESpiderTrafficLightState::Yellow && Intersect.StateTimer >= YellowLightDuration)
		{
			Intersect.NorthSouthLight = ESpiderTrafficLightState::Red;
			Intersect.EastWestLight = ESpiderTrafficLightState::Green;
			Intersect.StateTimer = 0.0f;
		}
		else if (Intersect.EastWestLight == ESpiderTrafficLightState::Green && Intersect.StateTimer >= GreenLightDuration)
		{
			Intersect.EastWestLight = ESpiderTrafficLightState::Yellow;
			Intersect.StateTimer = 0.0f;
		}
		else if (Intersect.EastWestLight == ESpiderTrafficLightState::Yellow && Intersect.StateTimer >= YellowLightDuration)
		{
			Intersect.EastWestLight = ESpiderTrafficLightState::Red;
			Intersect.NorthSouthLight = ESpiderTrafficLightState::Green;
			Intersect.StateTimer = 0.0f;
		}
	}
}

void ASpiderTrafficManager::UpdateVehicles(float DeltaSeconds)
{
	for (int32 i = 0; i < ActiveVehicles.Num(); ++i)
	{
		FSpiderSimulatedVehicle& Vehicle = ActiveVehicles[i];
		if (Vehicle.bIsWrecked || !TrafficLanes.IsValidIndex(Vehicle.CurrentLaneId))
		{
			continue;
		}

		const FSpiderTrafficLane& Lane = TrafficLanes[Vehicle.CurrentLaneId];
		const float LaneLength = FVector::Distance(Lane.StartPoint, Lane.EndPoint);

		CheckObstaclesAndLeadVehicles(Vehicle, DeltaSeconds);

		if (!Vehicle.bIsStopped)
		{
			const float MoveDelta = (Vehicle.CurrentSpeed * DeltaSeconds) / FMath::Max(LaneLength, 1.0f);
			Vehicle.LaneDistanceRatio += MoveDelta;

			if (Vehicle.LaneDistanceRatio >= 1.0f)
			{
				// Transition to random next lane
				Vehicle.LaneDistanceRatio = 0.0f;
				Vehicle.CurrentLaneId = FMath::RandRange(0, TrafficLanes.Num() - 1);
			}
		}

		// Update World Position
		const FSpiderTrafficLane& CurrentLane = TrafficLanes[Vehicle.CurrentLaneId];
		const FVector NewPos = FMath::Lerp(CurrentLane.StartPoint, CurrentLane.EndPoint, Vehicle.LaneDistanceRatio);
		const FRotator NewRot = (CurrentLane.EndPoint - CurrentLane.StartPoint).Rotation();
		Vehicle.Transform = FTransform(NewRot, NewPos, FVector::OneVector);

		UInstancedStaticMeshComponent* Ism = GetIsmForType(Vehicle.VehicleType);
		if (Ism && Ism->IsValidInstance(Vehicle.InstanceIndex))
		{
			Ism->UpdateInstanceTransform(Vehicle.InstanceIndex, Vehicle.Transform, true, false, true);
		}
	}
}

void ASpiderTrafficManager::CheckObstaclesAndLeadVehicles(FSpiderSimulatedVehicle& Vehicle, float DeltaSeconds)
{
	Vehicle.bIsStopped = false;

	// Check ahead along forward vector
	const FVector RayStart = Vehicle.Transform.GetLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector RayEnd = RayStart + (Vehicle.Transform.GetRotation().GetForwardVector() * SafeFollowingDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_Pawn, Params))
	{
		Vehicle.bIsStopped = true;
		Vehicle.bIsHonking = true;
	}
	else
	{
		Vehicle.bIsHonking = false;
	}
}

void ASpiderTrafficManager::DamageVehicle(int32 VehicleId, const FVector& ImpactForce)
{
	for (FSpiderSimulatedVehicle& Vehicle : ActiveVehicles)
	{
		if (Vehicle.VehicleId == VehicleId)
		{
			Vehicle.bIsWrecked = true;
			Vehicle.bIsStopped = true;

			// Add rotational tilt to wrecked instance
			FRotator WreckedRot = Vehicle.Transform.Rotator();
			WreckedRot.Pitch += FMath::RandRange(-25.0f, 25.0f);
			WreckedRot.Roll += FMath::RandRange(-35.0f, 35.0f);
			Vehicle.Transform.SetRotation(WreckedRot.Quaternion());
			Vehicle.Transform.AddToTranslation(ImpactForce * 0.1f);

			UInstancedStaticMeshComponent* Ism = GetIsmForType(Vehicle.VehicleType);
			if (Ism && Ism->IsValidInstance(Vehicle.InstanceIndex))
			{
				Ism->UpdateInstanceTransform(Vehicle.InstanceIndex, Vehicle.Transform, true, true, true);
			}

			OnVehicleImpact.Broadcast(VehicleId, ImpactForce);
			UE_LOG(LogSpiderTraffic, Log, TEXT("Vehicle %d wrecked by superhero force!"), VehicleId);
			break;
		}
	}
}

void ASpiderTrafficManager::RecycleVehiclesAroundPlayer()
{
	AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerActor)
	{
		return;
	}

	const FVector PlayerLoc = PlayerActor->GetActorLocation();

	for (int32 i = ActiveVehicles.Num() - 1; i >= 0; --i)
	{
		FSpiderSimulatedVehicle& Vehicle = ActiveVehicles[i];
		const float Dist = FVector::Distance(PlayerLoc, Vehicle.Transform.GetLocation());

		if (Dist > VehicleDespawnDistance)
		{
			// Respawn closer to player
			int32 NearestLane = FMath::RandRange(0, TrafficLanes.Num() - 1);
			Vehicle.CurrentLaneId = NearestLane;
			Vehicle.LaneDistanceRatio = FMath::FRandRange(0.1f, 0.9f);
			Vehicle.bIsWrecked = false;
			Vehicle.bIsStopped = false;
		}
	}
}

UInstancedStaticMeshComponent* ASpiderTrafficManager::GetIsmForType(ESpiderVehicleType Type)
{
	if (TObjectPtr<UInstancedStaticMeshComponent>* Found = VehicleIsmMap.Find(Type))
	{
		return *Found;
	}

	UStaticMesh* MeshToUse = nullptr;
	switch (Type)
	{
	case ESpiderVehicleType::Taxi:   MeshToUse = TaxiMesh.Get(); break;
	case ESpiderVehicleType::SUV:    MeshToUse = SuvMesh.Get(); break;
	case ESpiderVehicleType::Bus:    MeshToUse = BusMesh.Get(); break;
	case ESpiderVehicleType::Sedan:
	default:                         MeshToUse = SedanMesh.Get(); break;
	}

	if (!MeshToUse)
	{
		return nullptr;
	}

	UInstancedStaticMeshComponent* NewIsm = NewObject<UInstancedStaticMeshComponent>(this);
	NewIsm->SetStaticMesh(MeshToUse);
	NewIsm->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewIsm->SetCollisionProfileName(TEXT("Vehicle"));
	NewIsm->RegisterComponent();

	VehicleIsmMap.Add(Type, NewIsm);
	return NewIsm;
}
