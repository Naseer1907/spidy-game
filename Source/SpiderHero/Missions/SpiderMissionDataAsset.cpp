// Copyright SpiderHero Team. All Rights Reserved.

#include "Missions/SpiderMissionDataAsset.h"

USpiderMissionDataAsset::USpiderMissionDataAsset()
{
	MissionID = TEXT("Mission_Default");
	Title = FText::FromString(TEXT("Default Mission"));
	Description = FText::FromString(TEXT("Default Mission Description"));
	MissionType = ESpiderMissionType::Side;
	XPReward = 250;
	SkillPointReward = 1;
	ReputationReward = 50;
	bIsRepeatable = false;
}
