// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SpiderSaveGame.generated.h"

/**
 * USpiderSaveGame
 * Complete SaveGame serialization class storing progression, skills, player stats,
 * mission states, open world metrics, and user preferences.
 */
UCLASS()
class SPIDERHERO_API USpiderSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	USpiderSaveGame();

	// Meta
	UPROPERTY(VisibleAnywhere, Category = "Spider|Save")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save")
	int32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save")
	int32 SaveVersion;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save")
	FDateTime Timestamp;

	// Player Progression & Stats
	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Progression")
	int32 PlayerLevel;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Progression")
	int32 CurrentXP;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Progression")
	int32 SkillPoints;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Progression")
	TArray<FName> UnlockedSkillIDs;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Stats")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Stats")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Stats")
	float CurrentStamina;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Stats")
	float MaxStamina;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Transform")
	FTransform PlayerTransform;

	// Missions
	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Missions")
	FName ActiveMissionID;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Missions")
	TMap<FName, int32> ObjectiveProgressMap;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Missions")
	TArray<FName> CompletedMissionIDs;

	// Open World City State
	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|World")
	float CityCrimeRate;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|World")
	int32 HeroReputation;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|World")
	int32 HeatLevel;

	// Settings
	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Settings")
	float MasterVolume;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Settings")
	float SFXVolume;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Settings")
	float MusicVolume;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Save|Settings")
	int32 ScalabilityQualityPreset;
};
