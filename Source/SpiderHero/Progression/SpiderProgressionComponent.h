// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpiderProgressionComponent.generated.h"

class ASpiderHeroCharacter;

UENUM(BlueprintType)
enum class ESpiderSkillTree : uint8
{
	Traversal = 0 UMETA(DisplayName = "Traversal & Swinging"),
	Combat    = 1 UMETA(DisplayName = "Combat & Melee"),
	Web       = 2 UMETA(DisplayName = "Web Gadgets"),
	Survival  = 3 UMETA(DisplayName = "Survival & Defense")
};

USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderSkillNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|SkillTree")
	FName SkillID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|SkillTree")
	FText DisplayName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|SkillTree")
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|SkillTree")
	ESpiderSkillTree SkillTree = ESpiderSkillTree::Traversal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|SkillTree")
	int32 SkillPointCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|SkillTree")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|SkillTree")
	FName PrerequisiteSkillID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|SkillTree")
	bool bIsUnlocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUpSignature, int32, NewLevel, int32, SkillPointsAwarded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExperienceGainedSignature, int32, XPGained, int32, CurrentXP, int32, XPForNextLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUnlockedSignature, const FSpiderSkillNode&, UnlockedSkill);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillPointsChangedSignature, int32, CurrentSkillPoints);

/**
 * USpiderProgressionComponent
 * Manages player experience, leveling curve, skill point allocation,
 * and 4 distinct skill trees (Traversal, Combat, Web, Survival).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderProgressionComponent();

	virtual void BeginPlay() override;

	/** Award experience points and process potential level ups */
	UFUNCTION(BlueprintCallable, Category = "Spider|Progression")
	void AddExperience(int32 Amount);

	/** Add skill points */
	UFUNCTION(BlueprintCallable, Category = "Spider|Progression")
	void AddSkillPoints(int32 Amount);

	/** Attempt to unlock a skill */
	UFUNCTION(BlueprintCallable, Category = "Spider|Progression")
	bool UnlockSkill(FName SkillID);

	/** Check if skill can be unlocked */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	bool CanUnlockSkill(FName SkillID) const;

	/** Check if skill is already unlocked */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	bool IsSkillUnlocked(FName SkillID) const;

	/** Get current level */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	int32 GetPlayerLevel() const { return PlayerLevel; }

	/** Get current XP */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	int32 GetCurrentXP() const { return CurrentXP; }

	/** Get XP needed for next level */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	int32 GetXPForNextLevel() const { return XPForNextLevel; }

	/** Get XP percentage to next level (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	float GetXPPercent() const { return XPForNextLevel > 0 ? static_cast<float>(CurrentXP) / static_cast<float>(XPForNextLevel) : 0.0f; }

	/** Get available skill points */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	int32 GetAvailableSkillPoints() const { return SkillPoints; }

	/** Retrieve all skills in a specific tree */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	TArray<FSpiderSkillNode> GetSkillsInTree(ESpiderSkillTree Tree) const;

	/** Get node data by ID */
	UFUNCTION(BlueprintPure, Category = "Spider|Progression")
	bool GetSkillNode(FName SkillID, FSpiderSkillNode& OutNode) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Progression|Events")
	FOnLevelUpSignature OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Progression|Events")
	FOnExperienceGainedSignature OnExperienceGained;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Progression|Events")
	FOnSkillUnlockedSignature OnSkillUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Progression|Events")
	FOnSkillPointsChangedSignature OnSkillPointsChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Progression|State")
	int32 PlayerLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Progression|State")
	int32 CurrentXP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Progression|State")
	int32 XPForNextLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Progression|State")
	int32 SkillPoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Progression|SkillTrees")
	TArray<FSpiderSkillNode> SkillNodes;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;

	void InitializeSkillTree();
	void ApplySkillBuffs(FName SkillID);
	int32 CalculateXPRequiredForLevel(int32 Level) const;
};
