// Copyright SpiderHero Team. All Rights Reserved.

#include "Civilians/SpiderCivilianManager.h"
#include "SpiderHero.h"
#include "Character/SpiderHeroCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ASpiderCivilianManager::ASpiderCivilianManager()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxCivilians = 60;
	CivilianWalkSpeed = 150.0f;
	CivilianFleeSpeed = 450.0f;
	HeroReactionDistance = 1200.0f;
	HeroPhotoDistance = 700.0f;
	TickCullingDistance = 18000.0f;

	NextCivilianId = 1;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
}

void ASpiderCivilianManager::BeginPlay()
{
	Super::BeginPlay();

	InitializePedestrians(GetActorLocation(), 4, 9400.0f);
	UE_LOG(LogSpiderCivilian, Log, TEXT("SpiderCivilianManager initialized with %d pedestrians."), CivilianPool.Num());
}

void ASpiderCivilianManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	const FVector PlayerLoc = PlayerActor ? PlayerActor->GetActorLocation() : GetActorLocation();

	UpdateCivilianAgents(DeltaSeconds, PlayerLoc);
}

void ASpiderCivilianManager::InitializePedestrians(const FVector& Center, int32 GridDimension, float BlockStride)
{
	SidewalkWaypoints.Empty();
	CivilianPool.Empty();

	const float HalfExtent = (GridDimension * BlockStride) * 0.5f;

	// Create grid of sidewalk perimeter waypoints
	for (int32 X = 0; X < GridDimension; ++X)
	{
		for (int32 Y = 0; Y < GridDimension; ++Y)
		{
			const float MinX = (X * BlockStride) - HalfExtent + 600.0f;
			const float MaxX = ((X + 1) * BlockStride) - HalfExtent - 600.0f;
			const float MinY = (Y * BlockStride) - HalfExtent + 600.0f;
			const float MaxY = ((Y + 1) * BlockStride) - HalfExtent - 600.0f;

			SidewalkWaypoints.Add(Center + FVector(MinX, MinY, 10.0f));
			SidewalkWaypoints.Add(Center + FVector(MaxX, MinY, 10.0f));
			SidewalkWaypoints.Add(Center + FVector(MaxX, MaxY, 10.0f));
			SidewalkWaypoints.Add(Center + FVector(MinX, MaxY, 10.0f));
		}
	}

	SpawnInitialCivilians();
}

void ASpiderCivilianManager::SpawnInitialCivilians()
{
	if (SidewalkWaypoints.Num() < 2)
	{
		return;
	}

	for (int32 i = 0; i < MaxCivilians; ++i)
	{
		FSpiderCivilianAgent Agent;
		Agent.CivilianId = NextCivilianId++;
		Agent.State = ESpiderCivilianState::Walking;
		Agent.WorldPosition = GetRandomSidewalkPoint();
		Agent.TargetPosition = GetRandomSidewalkPoint();
		Agent.WorldRotation = (Agent.TargetPosition - Agent.WorldPosition).Rotation();
		Agent.MoveSpeed = CivilianWalkSpeed;
		Agent.StateTimer = 0.0f;

		CivilianPool.Add(Agent);
	}
}

void ASpiderCivilianManager::UpdateCivilianAgents(float DeltaSeconds, const FVector& PlayerLocation)
{
	for (FSpiderCivilianAgent& Agent : CivilianPool)
	{
		const float DistToPlayer = FVector::Distance(Agent.WorldPosition, PlayerLocation);

		// Distance culling & throttling
		if (DistToPlayer > TickCullingDistance)
		{
			continue;
		}

		UpdateAgentState(Agent, DeltaSeconds, PlayerLocation);
	}
}

void ASpiderCivilianManager::UpdateAgentState(FSpiderCivilianAgent& Agent, float DeltaSeconds, const FVector& PlayerLocation)
{
	Agent.StateTimer += DeltaSeconds;

	// Handle hero presence reactions if not in panic
	if (Agent.State != ESpiderCivilianState::FleeingDanger && Agent.State != ESpiderCivilianState::CoweringInPanic)
	{
		HandleHeroReactions(Agent, PlayerLocation);
	}

	switch (Agent.State)
	{
	case ESpiderCivilianState::Walking:
	{
		const FVector ToTarget = (Agent.TargetPosition - Agent.WorldPosition);
		const float Dist = ToTarget.Size();

		if (Dist < 80.0f)
		{
			// Pick new destination or pause at corner
			if (FMath::FRand() < 0.25f)
			{
				Agent.State = ESpiderCivilianState::WaitingAtCrosswalk;
				Agent.StateTimer = 0.0f;
			}
			else
			{
				Agent.TargetPosition = GetRandomSidewalkPoint();
			}
		}
		else
		{
			const FVector MoveDir = ToTarget.GetSafeNormal();
			Agent.WorldPosition += MoveDir * Agent.MoveSpeed * DeltaSeconds;
			Agent.WorldRotation = FMath::RInterpTo(Agent.WorldRotation, MoveDir.Rotation(), DeltaSeconds, 8.0f);
		}
		break;
	}

	case ESpiderCivilianState::WaitingAtCrosswalk:
	{
		if (Agent.StateTimer >= 4.0f)
		{
			Agent.State = ESpiderCivilianState::Walking;
			Agent.TargetPosition = GetRandomSidewalkPoint();
			Agent.StateTimer = 0.0f;
		}
		break;
	}

	case ESpiderCivilianState::IdleChatter:
	{
		if (Agent.StateTimer >= 6.0f)
		{
			Agent.State = ESpiderCivilianState::Walking;
			Agent.TargetPosition = GetRandomSidewalkPoint();
			Agent.StateTimer = 0.0f;
		}
		break;
	}

	case ESpiderCivilianState::TakingPhotoOfHero:
	{
		const FVector FaceHero = (PlayerLocation - Agent.WorldPosition).GetSafeNormal2D();
		Agent.WorldRotation = FMath::RInterpTo(Agent.WorldRotation, FaceHero.Rotation(), DeltaSeconds, 10.0f);

		if (Agent.StateTimer >= 5.0f || FVector::Distance(Agent.WorldPosition, PlayerLocation) > HeroReactionDistance)
		{
			Agent.State = ESpiderCivilianState::Walking;
			Agent.TargetPosition = GetRandomSidewalkPoint();
			Agent.StateTimer = 0.0f;
		}
		break;
	}

	case ESpiderCivilianState::Cheering:
	{
		const FVector FaceHero = (PlayerLocation - Agent.WorldPosition).GetSafeNormal2D();
		Agent.WorldRotation = FMath::RInterpTo(Agent.WorldRotation, FaceHero.Rotation(), DeltaSeconds, 8.0f);

		if (Agent.StateTimer >= 4.5f)
		{
			Agent.State = ESpiderCivilianState::Walking;
			Agent.TargetPosition = GetRandomSidewalkPoint();
			Agent.StateTimer = 0.0f;
		}
		break;
	}

	case ESpiderCivilianState::FleeingDanger:
	{
		const FVector ToTarget = (Agent.TargetPosition - Agent.WorldPosition);
		const float Dist = ToTarget.Size();

		if (Dist < 120.0f || Agent.StateTimer >= Agent.PanicDuration)
		{
			Agent.State = ESpiderCivilianState::CoweringInPanic;
			Agent.StateTimer = 0.0f;
		}
		else
		{
			const FVector MoveDir = ToTarget.GetSafeNormal();
			Agent.WorldPosition += MoveDir * CivilianFleeSpeed * DeltaSeconds;
			Agent.WorldRotation = MoveDir.Rotation();
		}
		break;
	}

	case ESpiderCivilianState::CoweringInPanic:
	{
		if (Agent.StateTimer >= 8.0f)
		{
			Agent.State = ESpiderCivilianState::Walking;
			Agent.TargetPosition = GetRandomSidewalkPoint();
			Agent.StateTimer = 0.0f;
		}
		break;
	}
	}
}

void ASpiderCivilianManager::HandleHeroReactions(FSpiderCivilianAgent& Agent, const FVector& PlayerLocation)
{
	const float Dist = FVector::Distance(Agent.WorldPosition, PlayerLocation);

	if (Dist < HeroPhotoDistance && Agent.State == ESpiderCivilianState::Walking && FMath::FRand() < 0.05f)
	{
		Agent.State = ESpiderCivilianState::TakingPhotoOfHero;
		Agent.StateTimer = 0.0f;
	}
	else if (Dist < HeroReactionDistance && Agent.State == ESpiderCivilianState::Walking && FMath::FRand() < 0.03f)
	{
		Agent.State = ESpiderCivilianState::Cheering;
		Agent.StateTimer = 0.0f;
	}
}

void ASpiderCivilianManager::TriggerPanicAtLocation(const FVector& ThreatLocation, float Radius)
{
	int32 PanickedCount = 0;

	for (FSpiderCivilianAgent& Agent : CivilianPool)
	{
		const float Dist = FVector::Distance(Agent.WorldPosition, ThreatLocation);
		if (Dist <= Radius)
		{
			Agent.State = ESpiderCivilianState::FleeingDanger;
			Agent.PanicDuration = FMath::FRandRange(4.0f, 8.0f);
			Agent.StateTimer = 0.0f;

			// Flee directly away from threat
			const FVector FleeDir = (Agent.WorldPosition - ThreatLocation).GetSafeNormal2D();
			Agent.TargetPosition = Agent.WorldPosition + (FleeDir * 2000.0f);
			PanickedCount++;
		}
	}

	OnCrowdPanicTriggered.Broadcast(ThreatLocation, Radius);
	UE_LOG(LogSpiderCivilian, Warning, TEXT("Crowd panic triggered! %d civilians fleeing threat at %s"), PanickedCount, *ThreatLocation.ToString());
}

FVector ASpiderCivilianManager::GetRandomSidewalkPoint() const
{
	if (SidewalkWaypoints.Num() == 0)
	{
		return GetActorLocation();
	}

	const int32 Index = FMath::RandRange(0, SidewalkWaypoints.Num() - 1);
	return SidewalkWaypoints[Index] + FVector(FMath::FRandRange(-100.0f, 100.0f), FMath::FRandRange(-100.0f, 100.0f), 0.0f);
}
