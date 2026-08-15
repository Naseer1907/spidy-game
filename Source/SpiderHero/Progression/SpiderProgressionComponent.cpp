// Copyright SpiderHero Team. All Rights Reserved.

#include "Progression/SpiderProgressionComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "Abilities/SpiderHealthComponent.h"
#include "Abilities/SpiderStaminaComponent.h"
#include "SpiderHero.h"

USpiderProgressionComponent::USpiderProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PlayerLevel = 1;
	CurrentXP = 0;
	XPForNextLevel = 1000;
	SkillPoints = 2; // Initial starting skill points
}

void USpiderProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());
	InitializeSkillTree();
}

int32 USpiderProgressionComponent::CalculateXPRequiredForLevel(int32 Level) const
{
	return FMath::RoundToInt(1000.0f * FMath::Pow(static_cast<float>(Level), 1.45f));
}

void USpiderProgressionComponent::InitializeSkillTree()
{
	if (SkillNodes.Num() > 0)
	{
		return;
	}

	// 1. TRAVERSAL TREE
	FSpiderSkillNode T1;
	T1.SkillID = TEXT("Skill_FasterSwing");
	T1.DisplayName = FText::FromString(TEXT("Aerodynamic Web Swing"));
	T1.Description = FText::FromString(TEXT("Increases maximum swinging velocity by 25%."));
	T1.SkillTree = ESpiderSkillTree::Traversal;
	T1.SkillPointCost = 1;
	T1.RequiredLevel = 1;
	SkillNodes.Add(T1);

	FSpiderSkillNode T2;
	T2.SkillID = TEXT("Skill_WebLaunchBoost");
	T2.DisplayName = FText::FromString(TEXT("Apex Kinetic Launch"));
	T2.Description = FText::FromString(TEXT("Releasing at the apex of a swing grants an explosive 40% forward & upward boost."));
	T2.SkillTree = ESpiderSkillTree::Traversal;
	T2.SkillPointCost = 2;
	T2.RequiredLevel = 2;
	T2.PrerequisiteSkillID = TEXT("Skill_FasterSwing");
	SkillNodes.Add(T2);

	FSpiderSkillNode T3;
	T3.SkillID = TEXT("Skill_WallSprint");
	T3.DisplayName = FText::FromString(TEXT("Frictionless Wall Sprint"));
	T3.Description = FText::FromString(TEXT("Increases wall-running duration by 50% and allows upward vertical sprint chaining."));
	T3.SkillTree = ESpiderSkillTree::Traversal;
	T3.SkillPointCost = 2;
	T3.RequiredLevel = 3;
	T3.PrerequisiteSkillID = TEXT("Skill_WebLaunchBoost");
	SkillNodes.Add(T3);

	// 2. COMBAT TREE
	FSpiderSkillNode C1;
	C1.SkillID = TEXT("Skill_AirJuggleMaster");
	C1.DisplayName = FText::FromString(TEXT("Air Juggle Virtuoso"));
	C1.Description = FText::FromString(TEXT("Suspends air combat gravity by 40%, enabling extended 6-hit aerial combo chains."));
	C1.SkillTree = ESpiderSkillTree::Combat;
	C1.SkillPointCost = 1;
	C1.RequiredLevel = 1;
	SkillNodes.Add(C1);

	FSpiderSkillNode C2;
	C2.SkillID = TEXT("Skill_PerfectCounter");
	C2.DisplayName = FText::FromString(TEXT("Spider-Sense Counter-Strike"));
	C2.Description = FText::FromString(TEXT("Dodging in the red critical window triggers instant counter-attack web strike."));
	C2.SkillTree = ESpiderSkillTree::Combat;
	C2.SkillPointCost = 2;
	C2.RequiredLevel = 2;
	C2.PrerequisiteSkillID = TEXT("Skill_AirJuggleMaster");
	SkillNodes.Add(C2);

	FSpiderSkillNode C3;
	C3.SkillID = TEXT("Skill_HeavyFinisher");
	C3.DisplayName = FText::FromString(TEXT("Overwhelming Finisher"));
	C3.Description = FText::FromString(TEXT("Finishers deal AOE shockwave damage and build double combo multiplier."));
	C3.SkillTree = ESpiderSkillTree::Combat;
	C3.SkillPointCost = 3;
	C3.RequiredLevel = 4;
	C3.PrerequisiteSkillID = TEXT("Skill_PerfectCounter");
	SkillNodes.Add(C3);

	// 3. WEB TREE
	FSpiderSkillNode W1;
	W1.SkillID = TEXT("Skill_WebBurstAOE");
	W1.DisplayName = FText::FromString(TEXT("Web Splatter Burst"));
	W1.Description = FText::FromString(TEXT("Web shots explode on impact, webbing all enemies within 3 meters."));
	W1.SkillTree = ESpiderSkillTree::Web;
	W1.SkillPointCost = 1;
	W1.RequiredLevel = 1;
	SkillNodes.Add(W1);

	FSpiderSkillNode W2;
	W2.SkillID = TEXT("Skill_MultiWebShot");
	W2.DisplayName = FText::FromString(TEXT("Triple Web Salvo"));
	W2.Description = FText::FromString(TEXT("Fires a spread of 3 web projectiles simultaneously with single cartridge consumption."));
	W2.SkillTree = ESpiderSkillTree::Web;
	W2.SkillPointCost = 2;
	W2.RequiredLevel = 2;
	W2.PrerequisiteSkillID = TEXT("Skill_WebBurstAOE");
	SkillNodes.Add(W2);

	// 4. SURVIVAL TREE
	FSpiderSkillNode S1;
	S1.SkillID = TEXT("Skill_MaxHealth");
	S1.DisplayName = FText::FromString(TEXT("Bio-Enhanced Toughness"));
	S1.Description = FText::FromString(TEXT("Permanently increases Max Health by +50 HP."));
	S1.SkillTree = ESpiderSkillTree::Survival;
	S1.SkillPointCost = 1;
	S1.RequiredLevel = 1;
	SkillNodes.Add(S1);

	FSpiderSkillNode S2;
	S2.SkillID = TEXT("Skill_MaxStamina");
	S2.DisplayName = FText::FromString(TEXT("Metabolic Endurance"));
	S2.Description = FText::FromString(TEXT("Permanently increases Max Stamina by +30 points."));
	S2.SkillTree = ESpiderSkillTree::Survival;
	S2.SkillPointCost = 1;
	S2.RequiredLevel = 1;
	SkillNodes.Add(S2);

	FSpiderSkillNode S3;
	S3.SkillID = TEXT("Skill_RapidRegen");
	S3.DisplayName = FText::FromString(TEXT("Accelerated Cellular Recovery"));
	S3.Description = FText::FromString(TEXT("Doubles health regeneration rate and cuts regen delay in half."));
	S3.SkillTree = ESpiderSkillTree::Survival;
	S3.SkillPointCost = 2;
	S3.RequiredLevel = 3;
	S3.PrerequisiteSkillID = TEXT("Skill_MaxHealth");
	SkillNodes.Add(S3);
}

void USpiderProgressionComponent::AddExperience(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	CurrentXP += Amount;
	UE_LOG(LogSpiderHero, Log, TEXT("Gained %d XP (Current: %d / %d)"), Amount, CurrentXP, XPForNextLevel);

	while (CurrentXP >= XPForNextLevel)
	{
		CurrentXP -= XPForNextLevel;
		PlayerLevel++;
		XPForNextLevel = CalculateXPRequiredForLevel(PlayerLevel);

		int32 AwardedPoints = 1;
		if (PlayerLevel % 5 == 0)
		{
			AwardedPoints = 2; // Bonus points every 5 levels
		}

		SkillPoints += AwardedPoints;

		OnLevelUp.Broadcast(PlayerLevel, AwardedPoints);
		OnSkillPointsChanged.Broadcast(SkillPoints);
		UE_LOG(LogSpiderHero, Warning, TEXT("LEVEL UP! Reached Level %d! (+%d Skill Points)"), PlayerLevel, AwardedPoints);
	}

	OnExperienceGained.Broadcast(Amount, CurrentXP, XPForNextLevel);
}

void USpiderProgressionComponent::AddSkillPoints(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	SkillPoints += Amount;
	OnSkillPointsChanged.Broadcast(SkillPoints);
}

bool USpiderProgressionComponent::CanUnlockSkill(FName SkillID) const
{
	const FSpiderSkillNode* Node = SkillNodes.FindByPredicate([SkillID](const FSpiderSkillNode& N) {
		return N.SkillID == SkillID;
	});

	if (!Node || Node->bIsUnlocked)
	{
		return false;
	}

	if (SkillPoints < Node->SkillPointCost || PlayerLevel < Node->RequiredLevel)
	{
		return false;
	}

	// Check Prerequisite
	if (Node->PrerequisiteSkillID != NAME_None)
	{
		if (!IsSkillUnlocked(Node->PrerequisiteSkillID))
		{
			return false;
		}
	}

	return true;
}

bool USpiderProgressionComponent::IsSkillUnlocked(FName SkillID) const
{
	const FSpiderSkillNode* Node = SkillNodes.FindByPredicate([SkillID](const FSpiderSkillNode& N) {
		return N.SkillID == SkillID;
	});

	return Node ? Node->bIsUnlocked : false;
}

bool USpiderProgressionComponent::UnlockSkill(FName SkillID)
{
	FSpiderSkillNode* Node = SkillNodes.FindByPredicate([SkillID](const FSpiderSkillNode& N) {
		return N.SkillID == SkillID;
	});

	if (!Node || !CanUnlockSkill(SkillID))
	{
		return false;
	}

	SkillPoints -= Node->SkillPointCost;
	Node->bIsUnlocked = true;

	ApplySkillBuffs(SkillID);

	OnSkillUnlocked.Broadcast(*Node);
	OnSkillPointsChanged.Broadcast(SkillPoints);
	UE_LOG(LogSpiderHero, Log, TEXT("Unlocked Skill: %s (%s)"), *Node->DisplayName.ToString(), *SkillID.ToString());

	return true;
}

void USpiderProgressionComponent::ApplySkillBuffs(FName SkillID)
{
	if (!CharacterOwner.IsValid())
	{
		return;
	}

	if (SkillID == TEXT("Skill_MaxHealth"))
	{
		if (USpiderHealthComponent* Health = CharacterOwner->FindComponentByClass<USpiderHealthComponent>())
		{
			Health->SetMaxHealth(Health->GetMaxHealth() + 50.0f, true);
		}
	}
	else if (SkillID == TEXT("Skill_MaxStamina"))
	{
		if (USpiderStaminaComponent* Stamina = CharacterOwner->FindComponentByClass<USpiderStaminaComponent>())
		{
			Stamina->SetMaxStamina(Stamina->GetMaxStamina() + 30.0f, true);
		}
	}
}

TArray<FSpiderSkillNode> USpiderProgressionComponent::GetSkillsInTree(ESpiderSkillTree Tree) const
{
	TArray<FSpiderSkillNode> Results;
	for (const FSpiderSkillNode& Node : SkillNodes)
	{
		if (Node.SkillTree == Tree)
		{
			Results.Add(Node);
		}
	}
	return Results;
}

bool USpiderProgressionComponent::GetSkillNode(FName SkillID, FSpiderSkillNode& OutNode) const
{
	const FSpiderSkillNode* Found = SkillNodes.FindByPredicate([SkillID](const FSpiderSkillNode& N) {
		return N.SkillID == SkillID;
	});

	if (Found)
	{
		OutNode = *Found;
		return true;
	}

	return false;
}
