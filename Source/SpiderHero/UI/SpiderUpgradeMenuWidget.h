// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Progression/SpiderProgressionComponent.h"
#include "SpiderUpgradeMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeMenuClosedSignature);

/**
 * USpiderUpgradeMenuWidget
 * Interactive skill tree menu allowing tree tab switching, node inspection, and skill point allocation.
 */
UCLASS(Abstract, Blueprintable)
class SPIDERHERO_API USpiderUpgradeMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Selects a skill tree tab */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Skills")
	void SelectTreeTab(ESpiderSkillTree Tree);

	/** Selects a specific skill node to inspect */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Skills")
	void SelectSkillNode(FName SkillID);

	/** Attempts to unlock currently selected skill */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Skills")
	bool UnlockSelectedSkill();

	/** Closes the upgrade menu */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Skills")
	void CloseMenu();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|UI|Skills|Events")
	FOnUpgradeMenuClosedSignature OnUpgradeMenuClosed;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|Skills|State")
	ESpiderSkillTree ActiveTreeTab;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|Skills|State")
	FSpiderSkillNode SelectedSkillNode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|Skills|State")
	bool bHasSelection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|Skills|State")
	bool bCanUnlockSelected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|Skills|State")
	int32 AvailablePoints;

private:
	TWeakObjectPtr<USpiderProgressionComponent> ProgressionComp;
	void RefreshSelectionState();
};
