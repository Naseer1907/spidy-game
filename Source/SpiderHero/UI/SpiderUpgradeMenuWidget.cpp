// Copyright SpiderHero Team. All Rights Reserved.

#include "UI/SpiderUpgradeMenuWidget.h"
#include "Character/SpiderHeroCharacter.h"
#include "SpiderHero.h"

void USpiderUpgradeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ActiveTreeTab = ESpiderSkillTree::Traversal;
	bHasSelection = false;
	bCanUnlockSelected = false;
	AvailablePoints = 0;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			ProgressionComp = Pawn->FindComponentByClass<USpiderProgressionComponent>();
			if (ProgressionComp.IsValid())
			{
				AvailablePoints = ProgressionComp->GetAvailableSkillPoints();
			}
		}
	}
}

void USpiderUpgradeMenuWidget::SelectTreeTab(ESpiderSkillTree Tree)
{
	ActiveTreeTab = Tree;
	bHasSelection = false;
	bCanUnlockSelected = false;
}

void USpiderUpgradeMenuWidget::SelectSkillNode(FName SkillID)
{
	if (!ProgressionComp.IsValid())
	{
		return;
	}

	if (ProgressionComp->GetSkillNode(SkillID, SelectedSkillNode))
	{
		bHasSelection = true;
		RefreshSelectionState();
	}
}

void USpiderUpgradeMenuWidget::RefreshSelectionState()
{
	if (!ProgressionComp.IsValid() || !bHasSelection)
	{
		bCanUnlockSelected = false;
		return;
	}

	AvailablePoints = ProgressionComp->GetAvailableSkillPoints();
	bCanUnlockSelected = ProgressionComp->CanUnlockSkill(SelectedSkillNode.SkillID);
}

bool USpiderUpgradeMenuWidget::UnlockSelectedSkill()
{
	if (!ProgressionComp.IsValid() || !bHasSelection)
	{
		return false;
	}

	if (ProgressionComp->UnlockSkill(SelectedSkillNode.SkillID))
	{
		ProgressionComp->GetSkillNode(SelectedSkillNode.SkillID, SelectedSkillNode);
		RefreshSelectionState();
		return true;
	}

	return false;
}

void USpiderUpgradeMenuWidget::CloseMenu()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	RemoveFromParent();
	OnUpgradeMenuClosed.Broadcast();
}
