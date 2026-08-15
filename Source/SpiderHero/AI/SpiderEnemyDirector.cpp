// Copyright SpiderHero Team. All Rights Reserved.

#include "AI/SpiderEnemyDirector.h"
#include "SpiderHero.h"
#include "AI/SpiderEnemyBase.h"
#include "AI/SpiderAIController.h"
#include "Character/SpiderHeroCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ASpiderEnemyDirector::ASpiderEnemyDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxConcurrentAttackTokens = 2;
	MaxTokenDuration = 3.5f;
	MinDelayBetweenAttacks = 1.0f;

	InnerSlotCount = 6;
	InnerRingRadius = 380.0f;

	OuterSlotCount = 8;
	OuterRingRadius = 700.0f;

	CurrentEscalationLevel = 1;
	TimeSinceLastTokenGrant = 0.0f;
}

void ASpiderEnemyDirector::BeginPlay()
{
	Super::BeginPlay();

	InitializeSlots();
	FindPlayer();

	UE_LOG(LogSpiderAI, Log, TEXT("SpiderEnemyDirector initialized with %d slots."), CombatSlots.Num());
}

void ASpiderEnemyDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TimeSinceLastTokenGrant += DeltaSeconds;

	if (!TargetPlayer.IsValid())
	{
		FindPlayer();
	}

	UpdateSlotPositions();
	AssignSlotsToEnemies();
	ValidateActiveTokens(DeltaSeconds);
}

void ASpiderEnemyDirector::InitializeSlots()
{
	CombatSlots.Empty();
	int32 SlotId = 0;

	// Inner Ring Slots
	const float InnerAngleStep = 360.0f / static_cast<float>(InnerSlotCount);
	for (int32 i = 0; i < InnerSlotCount; ++i)
	{
		FSpiderCombatSlot Slot;
		Slot.SlotIndex = SlotId++;
		Slot.AngleDegrees = i * InnerAngleStep;
		Slot.Radius = InnerRingRadius;
		CombatSlots.Add(Slot);
	}

	// Outer Ring Slots (Offset by half step for alternating coverage)
	const float OuterAngleStep = 360.0f / static_cast<float>(OuterSlotCount);
	const float AngleOffset = OuterAngleStep * 0.5f;
	for (int32 i = 0; i < OuterSlotCount; ++i)
	{
		FSpiderCombatSlot Slot;
		Slot.SlotIndex = SlotId++;
		Slot.AngleDegrees = (i * OuterAngleStep) + AngleOffset;
		Slot.Radius = OuterRingRadius;
		CombatSlots.Add(Slot);
	}
}

void ASpiderEnemyDirector::FindPlayer()
{
	AActor* FoundChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (FoundChar)
	{
		TargetPlayer = Cast<ASpiderHeroCharacter>(FoundChar);
	}
}

void ASpiderEnemyDirector::UpdateSlotPositions()
{
	if (!TargetPlayer.IsValid())
	{
		return;
	}

	const FVector PlayerLoc = TargetPlayer->GetActorLocation();
	const float PlayerYaw = TargetPlayer->GetActorRotation().Yaw;

	for (FSpiderCombatSlot& Slot : CombatSlots)
	{
		const float TotalAngle = PlayerYaw + Slot.AngleDegrees;
		const FRotator Rot(0.0f, TotalAngle, 0.0f);
		Slot.WorldPosition = PlayerLoc + Rot.RotateVector(FVector(Slot.Radius, 0.0f, 0.0f));

		// Update occupant's controller if assigned
		if (Slot.Occupant.IsValid())
		{
			if (ASpiderAIController* AIC = Cast<ASpiderAIController>(Slot.Occupant->GetController()))
			{
				AIC->SetSlotTargetLocation(Slot.WorldPosition);
			}
		}
	}
}

void ASpiderEnemyDirector::AssignSlotsToEnemies()
{
	for (int32 i = RegisteredEnemies.Num() - 1; i >= 0; --i)
	{
		if (!RegisteredEnemies[i].IsValid() || !RegisteredEnemies[i]->IsAlive_Implementation())
		{
			UnregisterEnemy(RegisteredEnemies[i].Get());
			continue;
		}

		ASpiderEnemyBase* Enemy = RegisteredEnemies[i].Get();
		if (Enemy->GetAssignedSlotIndex() >= 0)
		{
			continue;
		}

		// Find closest free slot
		int32 BestSlotIdx = -1;
		float BestDistSq = MAX_FLT;

		for (int32 s = 0; s < CombatSlots.Num(); ++s)
		{
			if (!CombatSlots[s].bIsOccupied)
			{
				const float DistSq = FVector::DistSquared(Enemy->GetActorLocation(), CombatSlots[s].WorldPosition);
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestSlotIdx = s;
				}
			}
		}

		if (BestSlotIdx >= 0)
		{
			CombatSlots[BestSlotIdx].bIsOccupied = true;
			CombatSlots[BestSlotIdx].Occupant = Enemy;
			Enemy->SetAssignedSlotIndex(BestSlotIdx);
		}
	}
}

void ASpiderEnemyDirector::RegisterEnemy(ASpiderEnemyBase* Enemy)
{
	if (!Enemy || RegisteredEnemies.Contains(Enemy))
	{
		return;
	}

	RegisteredEnemies.Add(Enemy);
	UE_LOG(LogSpiderAI, Log, TEXT("Enemy registered with Director: %s (Total: %d)"), *Enemy->GetName(), RegisteredEnemies.Num());
}

void ASpiderEnemyDirector::UnregisterEnemy(ASpiderEnemyBase* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	ReleaseAttackToken(Enemy);

	const int32 SlotIdx = Enemy->GetAssignedSlotIndex();
	if (SlotIdx >= 0 && SlotIdx < CombatSlots.Num())
	{
		CombatSlots[SlotIdx].bIsOccupied = false;
		CombatSlots[SlotIdx].Occupant = nullptr;
		Enemy->SetAssignedSlotIndex(-1);
	}

	RegisteredEnemies.Remove(Enemy);
}

bool ASpiderEnemyDirector::RequestAttackToken(ASpiderEnemyBase* RequestingEnemy)
{
	if (!RequestingEnemy || !RequestingEnemy->IsAlive_Implementation())
	{
		return false;
	}

	if (ActiveTokenHolders.Num() >= MaxConcurrentAttackTokens)
	{
		return false;
	}

	if (TimeSinceLastTokenGrant < MinDelayBetweenAttacks)
	{
		return false;
	}

	// Grant Token
	FSpiderAttackTokenHolder TokenHolder;
	TokenHolder.Enemy = RequestingEnemy;
	TokenHolder.TimeTokenGranted = GetWorld()->GetTimeSeconds();
	TokenHolder.MaxTokenHoldDuration = MaxTokenDuration;

	ActiveTokenHolders.Add(TokenHolder);
	RequestingEnemy->SetHasAttackToken(true);
	TimeSinceLastTokenGrant = 0.0f;

	OnAttackTokenGranted.Broadcast(RequestingEnemy);
	UE_LOG(LogSpiderAI, Verbose, TEXT("Attack token granted to %s"), *RequestingEnemy->GetName());
	return true;
}

void ASpiderEnemyDirector::ReleaseAttackToken(ASpiderEnemyBase* ReleasingEnemy)
{
	if (!ReleasingEnemy)
	{
		return;
	}

	for (int32 i = ActiveTokenHolders.Num() - 1; i >= 0; --i)
	{
		if (ActiveTokenHolders[i].Enemy.Get() == ReleasingEnemy)
		{
			ActiveTokenHolders.RemoveAt(i);
			ReleasingEnemy->SetHasAttackToken(false);
			OnAttackTokenReleased.Broadcast(ReleasingEnemy);
			UE_LOG(LogSpiderAI, Verbose, TEXT("Attack token released by %s"), *ReleasingEnemy->GetName());
			break;
		}
	}
}

FVector ASpiderEnemyDirector::GetSlotLocationForEnemy(const ASpiderEnemyBase* Enemy) const
{
	if (!Enemy)
	{
		return FVector::ZeroVector;
	}

	const int32 SlotIdx = Enemy->GetAssignedSlotIndex();
	if (SlotIdx >= 0 && SlotIdx < CombatSlots.Num())
	{
		return CombatSlots[SlotIdx].WorldPosition;
	}

	return TargetPlayer.IsValid() ? TargetPlayer->GetActorLocation() : FVector::ZeroVector;
}

void ASpiderEnemyDirector::SetEscalationLevel(int32 NewLevel)
{
	CurrentEscalationLevel = FMath::Clamp(NewLevel, 1, 5);

	switch (CurrentEscalationLevel)
	{
	case 1:
		MaxConcurrentAttackTokens = 1;
		MinDelayBetweenAttacks = 1.6f;
		break;
	case 2:
		MaxConcurrentAttackTokens = 2;
		MinDelayBetweenAttacks = 1.0f;
		break;
	case 3:
	default:
		MaxConcurrentAttackTokens = 2;
		MinDelayBetweenAttacks = 0.6f;
		break;
	}

	OnCombatEscalationChanged.Broadcast(CurrentEscalationLevel);
}

void ASpiderEnemyDirector::ValidateActiveTokens(float DeltaSeconds)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	for (int32 i = ActiveTokenHolders.Num() - 1; i >= 0; --i)
	{
		FSpiderAttackTokenHolder& Holder = ActiveTokenHolders[i];
		if (!Holder.Enemy.IsValid() || 
			!Holder.Enemy->IsAlive_Implementation() || 
			Holder.Enemy->GetAIState() == ESpiderEnemyAIState::Staggered ||
			Holder.Enemy->GetAIState() == ESpiderEnemyAIState::KnockedDown ||
			Holder.Enemy->GetAIState() == ESpiderEnemyAIState::WebBound ||
			Holder.Enemy->GetAIState() == ESpiderEnemyAIState::WallPinned ||
			(CurrentTime - Holder.TimeTokenGranted) > Holder.MaxTokenHoldDuration)
		{
			if (Holder.Enemy.IsValid())
			{
				Holder.Enemy->SetHasAttackToken(false);
				OnAttackTokenReleased.Broadcast(Holder.Enemy.Get());
			}
			ActiveTokenHolders.RemoveAt(i);
		}
	}
}
