// Copyright SpiderHero Team. All Rights Reserved.

#include "Save/SpiderSaveGame.h"

USpiderSaveGame::USpiderSaveGame()
{
	SaveSlotName = TEXT("SpiderHero_SaveSlot_0");
	UserIndex = 0;
	SaveVersion = 1;
	Timestamp = FDateTime::Now();

	PlayerLevel = 1;
	CurrentXP = 0;
	SkillPoints = 0;

	CurrentHealth = 200.0f;
	MaxHealth = 200.0f;
	CurrentStamina = 100.0f;
	MaxStamina = 100.0f;
	PlayerTransform = FTransform::Identity;

	ActiveMissionID = NAME_None;
	CityCrimeRate = 50.0f;
	HeroReputation = 100;
	HeatLevel = 0;

	MasterVolume = 1.0f;
	SFXVolume = 1.0f;
	MusicVolume = 0.8f;
	ScalabilityQualityPreset = 3;
}
