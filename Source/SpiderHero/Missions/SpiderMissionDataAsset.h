// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpiderMissionDataAsset.generated.h"

UENUM(BlueprintType)
enum class ESpiderMissionType : uint8
{
	Main                = 0 UMETA(DisplayName = "Main Story"),
	Side                = 1 UMETA(DisplayName = "Side Quest"),
	DynamicCrime        = 2 UMETA(DisplayName = "Dynamic Crime"),
	TraversalChallenge  = 3 UMETA(DisplayName = "Traversal Challenge"),
	Tutorial            = 4 UMETA(DisplayName = "Tutorial")
};

UENUM(BlueprintType)
enum class ESpiderObjectiveType : uint8
{
	ReachLocation           = 0 UMETA(DisplayName = "Reach Location"),
	DefeatEnemies           = 1 UMETA(DisplayName = "Defeat Enemies"),
	WebShootingPractice     = 2 UMETA(DisplayName = "Web Shooting Practice"),
	PerformSwings           = 3 UMETA(DisplayName = "Perform Swings"),
	NeutralizeHostageThreat = 4 UMETA(DisplayName = "Neutralize Hostage Threat"),
	CollectItem             = 5 UMETA(DisplayName = "Collect Item"),
	TimedCheckpointRace     = 6 UMETA(DisplayName = "Timed Checkpoint Race")
};

UENUM(BlueprintType)
enum class ESpiderObjectiveStatus : uint8
{
	NotStarted = 0 UMETA(DisplayName = "Not Started"),
	InProgress = 1 UMETA(DisplayName = "In Progress"),
	Completed  = 2 UMETA(DisplayName = "Completed"),
	Failed     = 3 UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderMissionObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Missions")
	FName ObjectiveID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Missions")
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Missions")
	ESpiderObjectiveType Type = ESpiderObjectiveType::ReachLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Missions")
	int32 RequiredCount = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Missions")
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Missions")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Missions")
	FName TargetTag = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Missions")
	ESpiderObjectiveStatus Status = ESpiderObjectiveStatus::NotStarted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Missions")
	bool bIsOptional = false;

	bool IsComplete() const { return Status == ESpiderObjectiveStatus::Completed; }
};

/**
 * USpiderMissionDataAsset
 * Primary data asset defining mission objectives, rewards, and unlock rules.
 */
UCLASS(BlueprintType)
class SPIDERHERO_API USpiderMissionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USpiderMissionDataAsset();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions")
	FName MissionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions")
	ESpiderMissionType MissionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions")
	TArray<FSpiderMissionObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions|Rewards")
	int32 XPReward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions|Rewards")
	int32 SkillPointReward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions|Rewards")
	int32 ReputationReward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions|Config")
	bool bIsRepeatable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider|Missions|Config")
	TArray<FName> PrerequisiteMissionIDs;
};
