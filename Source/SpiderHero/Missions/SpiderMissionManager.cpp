// Copyright SpiderHero Team. All Rights Reserved.

#include "Missions/SpiderMissionManager.h"
#include "Character/SpiderHeroCharacter.h"
#include "Core/SpiderHeroGameState.h"
#include "Progression/SpiderProgressionComponent.h"
#include "SpiderHero.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

USpiderMissionManager::USpiderMissionManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	ActiveMissionAsset = nullptr;
	bEnableDynamicCrimes = true;
	DynamicCrimeSpawnInterval = 120.0f; // 2 minutes
	DynamicCrimeMinRadius = 5000.0f;
	DynamicCrimeMaxRadius = 12000.0f;
	TimeSinceLastCrimeSpawn = 0.0f;
}

void USpiderMissionManager::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());
}

void USpiderMissionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnableDynamicCrimes && !HasActiveMission())
	{
		TimeSinceLastCrimeSpawn += DeltaTime;
		if (TimeSinceLastCrimeSpawn >= DynamicCrimeSpawnInterval)
		{
			TimeSinceLastCrimeSpawn = 0.0f;
			TriggerDynamicCrime();
		}
	}
}

bool USpiderMissionManager::StartMission(USpiderMissionDataAsset* MissionAsset)
{
	if (!MissionAsset)
	{
		return false;
	}

	if (ActiveMissionAsset)
	{
		AbandonActiveMission();
	}

	ActiveMissionAsset = MissionAsset;
	CurrentObjectives = MissionAsset->Objectives;

	for (int32 i = 0; i < CurrentObjectives.Num(); ++i)
	{
		CurrentObjectives[i].CurrentCount = 0;
		CurrentObjectives[i].Status = (i == 0) ? ESpiderObjectiveStatus::InProgress : ESpiderObjectiveStatus::NotStarted;
	}

	OnMissionStarted.Broadcast(ActiveMissionAsset);
	UE_LOG(LogSpiderHero, Log, TEXT("Started Mission: %s (%s)"), *MissionAsset->Title.ToString(), *MissionAsset->MissionID.ToString());

	return true;
}

bool USpiderMissionManager::AdvanceObjective(FName ObjectiveID, int32 Amount)
{
	if (!ActiveMissionAsset)
	{
		return false;
	}

	for (int32 i = 0; i < CurrentObjectives.Num(); ++i)
	{
		FSpiderMissionObjective& Obj = CurrentObjectives[i];
		if (Obj.ObjectiveID == ObjectiveID && Obj.Status == ESpiderObjectiveStatus::InProgress)
		{
			Obj.CurrentCount = FMath::Min(Obj.RequiredCount, Obj.CurrentCount + Amount);
			
			if (Obj.CurrentCount >= Obj.RequiredCount)
			{
				Obj.Status = ESpiderObjectiveStatus::Completed;
				UE_LOG(LogSpiderHero, Log, TEXT("Objective '%s' completed!"), *Obj.Description.ToString());

				// Unlock next objective if available
				if (i + 1 < CurrentObjectives.Num() && CurrentObjectives[i + 1].Status == ESpiderObjectiveStatus::NotStarted)
				{
					CurrentObjectives[i + 1].Status = ESpiderObjectiveStatus::InProgress;
				}
			}

			OnObjectiveUpdated.Broadcast(Obj, i);
			CheckAllObjectivesComplete();
			return true;
		}
	}

	return false;
}

void USpiderMissionManager::CheckAllObjectivesComplete()
{
	if (!ActiveMissionAsset)
	{
		return;
	}

	bool bAllDone = true;
	for (const FSpiderMissionObjective& Obj : CurrentObjectives)
	{
		if (!Obj.bIsOptional && Obj.Status != ESpiderObjectiveStatus::Completed)
		{
			bAllDone = false;
			break;
		}
	}

	if (bAllDone)
	{
		CompleteActiveMission();
	}
}

void USpiderMissionManager::CompleteActiveMission()
{
	if (!ActiveMissionAsset)
	{
		return;
	}

	USpiderMissionDataAsset* CompletedAsset = ActiveMissionAsset;
	CompletedMissionIDs.AddUnique(CompletedAsset->MissionID);

	// Dispatch XP & Skill Points to ProgressionComponent
	if (CharacterOwner.IsValid())
	{
		USpiderProgressionComponent* Progression = CharacterOwner->FindComponentByClass<USpiderProgressionComponent>();
		if (Progression)
		{
			Progression->AddExperience(CompletedAsset->XPReward);
			Progression->AddSkillPoints(CompletedAsset->SkillPointReward);
		}
	}

	// Update City Metrics on GameState
	if (UWorld* World = GetWorld())
	{
		ASpiderHeroGameState* GameState = Cast<ASpiderHeroGameState>(World->GetGameState());
		if (GameState)
		{
			GameState->AddReputation(CompletedAsset->ReputationReward);
			GameState->ModifyCrimeRate(-5.0f); // Reduce crime on completion
		}
	}

	OnMissionCompleted.Broadcast(CompletedAsset, CompletedAsset->XPReward);
	UE_LOG(LogSpiderHero, Log, TEXT("Mission Completed: %s! Awarded %d XP, %d Skill Points."), 
		*CompletedAsset->Title.ToString(), CompletedAsset->XPReward, CompletedAsset->SkillPointReward);

	ActiveMissionAsset = nullptr;
	CurrentObjectives.Empty();
}

void USpiderMissionManager::FailActiveMission()
{
	if (!ActiveMissionAsset)
	{
		return;
	}

	USpiderMissionDataAsset* FailedAsset = ActiveMissionAsset;
	OnMissionFailed.Broadcast(FailedAsset);
	UE_LOG(LogSpiderHero, Warning, TEXT("Mission Failed: %s"), *FailedAsset->Title.ToString());

	ActiveMissionAsset = nullptr;
	CurrentObjectives.Empty();
}

void USpiderMissionManager::AbandonActiveMission()
{
	if (!ActiveMissionAsset)
	{
		return;
	}

	UE_LOG(LogSpiderHero, Log, TEXT("Mission Abandoned: %s"), *ActiveMissionAsset->Title.ToString());
	ActiveMissionAsset = nullptr;
	CurrentObjectives.Empty();
}

void USpiderMissionManager::StartTutorialMission()
{
	USpiderMissionDataAsset* Tutorial = NewObject<USpiderMissionDataAsset>(this, TEXT("Mission_RooftopDisturbance"));
	Tutorial->MissionID = TEXT("Mission_Tutorial_Rooftop");
	Tutorial->Title = FText::FromString(TEXT("Rooftop Disturbance"));
	Tutorial->Description = FText::FromString(TEXT("Investigate suspicious activity on the financial district skyscraper."));
	Tutorial->MissionType = ESpiderMissionType::Tutorial;
	Tutorial->XPReward = 500;
	Tutorial->SkillPointReward = 2;
	Tutorial->ReputationReward = 100;

	// Objective 1: Vantage Point
	FSpiderMissionObjective Obj1;
	Obj1.ObjectiveID = TEXT("Obj_ReachRooftop");
	Obj1.Description = FText::FromString(TEXT("Web-Swing or Wall-Climb to the high vantage point."));
	Obj1.Type = ESpiderObjectiveType::ReachLocation;
	Obj1.RequiredCount = 1;
	Tutorial->Objectives.Add(Obj1);

	// Objective 2: Web Shoot Targets
	FSpiderMissionObjective Obj2;
	Obj2.ObjectiveID = TEXT("Obj_WebShootTargets");
	Obj2.Description = FText::FromString(TEXT("Practice Web Shooting: Web up 3 lookouts."));
	Obj2.Type = ESpiderObjectiveType::WebShootingPractice;
	Obj2.RequiredCount = 3;
	Tutorial->Objectives.Add(Obj2);

	// Objective 3: Defeat Street Thugs
	FSpiderMissionObjective Obj3;
	Obj3.ObjectiveID = TEXT("Obj_DefeatGang");
	Obj3.Description = FText::FromString(TEXT("Neutralize the rooftop gang members."));
	Obj3.Type = ESpiderObjectiveType::DefeatEnemies;
	Obj3.RequiredCount = 5;
	Tutorial->Objectives.Add(Obj3);

	// Objective 4: Squad Leader Finisher
	FSpiderMissionObjective Obj4;
	Obj4.ObjectiveID = TEXT("Obj_PerformFinisher");
	Obj4.Description = FText::FromString(TEXT("Perform a Finisher on the Brute leader."));
	Obj4.Type = ESpiderObjectiveType::DefeatEnemies;
	Obj4.RequiredCount = 1;
	Tutorial->Objectives.Add(Obj4);

	StartMission(Tutorial);
}

void USpiderMissionManager::TriggerDynamicCrime()
{
	if (!CharacterOwner.IsValid())
	{
		return;
	}

	const FVector PlayerLoc = CharacterOwner->GetActorLocation();
	const float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
	const float RandomDist = FMath::RandRange(DynamicCrimeMinRadius, DynamicCrimeMaxRadius);

	FVector CrimeLoc = PlayerLoc + FVector(FMath::Cos(RandomAngle) * RandomDist, FMath::Sin(RandomAngle) * RandomDist, 0.0f);

	static const FName CrimeTypes[] = { TEXT("Crime_RooftopAmbush"), TEXT("Crime_ArmedRobbery"), TEXT("Crime_HostageSituation") };
	FName ChosenCrime = CrimeTypes[FMath::RandRange(0, 2)];

	OnDynamicCrimeSpawned.Broadcast(ChosenCrime, CrimeLoc);
	UE_LOG(LogSpiderHero, Log, TEXT("Dynamic Crime triggered: %s at %s"), *ChosenCrime.ToString(), *CrimeLoc.ToString());
}
