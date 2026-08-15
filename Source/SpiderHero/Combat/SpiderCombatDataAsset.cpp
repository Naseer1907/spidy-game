// Copyright SpiderHero Team. All Rights Reserved.

#include "Combat/SpiderCombatDataAsset.h"

USpiderCombatDataAsset::USpiderCombatDataAsset()
{
	AttackID = TEXT("DefaultAttack");
	AttackDisplayName = FText::FromString(TEXT("Attack"));
	AttackType = ESpiderAttackType::LightGround;
	ComboStepIndex = 0;

	BaseDamage = 25.0f;
	StaminaCost = 8.0f;
	KnockbackForce = 600.0f;
	KnockbackDirectionOffset = FVector(1.0f, 0.0f, 0.2f);
	StunDuration = 0.4f;

	bCausesLauncher = false;
	LauncherVerticalImpulse = 900.0f;
	bCausesGroundSlam = false;
	GroundSlamRadius = 450.0f;
	bCausesKnockdown = false;

	AttackRange = 280.0f;
	AttackHitRadius = 70.0f;
	MaxWarpDistance = 450.0f;
	WarpInterpSpeed = 12.0f;
	AutoTargetAngleThreshold = 65.0f;

	ComboWindowStart = 0.25f;
	ComboWindowEnd = 0.75f;
	HitCancelWindowStart = 0.35f;
	RecoveryDuration = 0.2f;

	PlayRate = 1.0f;
	MontageSectionName = NAME_None;

	TrailSocketStart = TEXT("hand_r_socket");
	TrailSocketEnd = TEXT("hand_l_socket");

	HitPauseDuration = 0.06f;
	HitPauseTimeDilation = 0.05f;
	CameraImpactType = ESpiderCameraImpactType::LightHit;
	CameraImpactIntensity = 1.0f;
}

FPrimaryAssetId USpiderCombatDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("SpiderAttack"), GetFName());
}
