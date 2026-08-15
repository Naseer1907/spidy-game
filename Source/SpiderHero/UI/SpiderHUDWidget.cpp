// Copyright SpiderHero Team. All Rights Reserved.

#include "UI/SpiderHUDWidget.h"
#include "SpiderHero.h"

void USpiderHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HealthPercent = 1.0f;
	StaminaPercent = 1.0f;
	XPPercent = 0.0f;
	CurrentLevel = 1;
	ComboCount = 0;
	ComboMultiplier = 1.0f;
	WebShotsRemaining = 8;
	MaxWebShotsCount = 8;
	ReticleScreenPos = FVector2D(0.5f, 0.5f);
	bReticleLocked = false;
	ActiveSpiderSenseLevel = ESpiderSenseAlertLevel::None;
	bShowObjective = false;
	bIsFatiguedAlert = false;
}

void USpiderHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void USpiderHUDWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

void USpiderHUDWidget::UpdateStamina(float CurrentStamina, float MaxStamina, bool bIsFatigued)
{
	StaminaPercent = MaxStamina > 0.0f ? FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f) : 0.0f;
	bIsFatiguedAlert = bIsFatigued;
}

void USpiderHUDWidget::UpdateXP(int32 CurrentXP, int32 XPForNextLevel, int32 Level)
{
	XPPercent = XPForNextLevel > 0 ? FMath::Clamp(static_cast<float>(CurrentXP) / static_cast<float>(XPForNextLevel), 0.0f, 1.0f) : 0.0f;
	CurrentLevel = Level;
}

void USpiderHUDWidget::UpdateCombo(int32 InComboCount, float InMultiplier)
{
	ComboCount = InComboCount;
	ComboMultiplier = InMultiplier;
}

void USpiderHUDWidget::UpdateWebAmmo(int32 CurrentShots, int32 MaxShots)
{
	WebShotsRemaining = CurrentShots;
	MaxWebShotsCount = MaxShots;
}

void USpiderHUDWidget::UpdateReticle(const FVector2D& ScreenPosition, bool bHasTarget)
{
	ReticleScreenPos = ScreenPosition;
	bReticleLocked = bHasTarget;
}

void USpiderHUDWidget::ShowSpiderSenseAlert(ESpiderSenseAlertLevel AlertLevel, float TimeToImpact)
{
	ActiveSpiderSenseLevel = AlertLevel;
}

void USpiderHUDWidget::HideSpiderSenseAlert()
{
	ActiveSpiderSenseLevel = ESpiderSenseAlertLevel::None;
}

void USpiderHUDWidget::UpdateObjectivePrompt(const FText& ObjectiveText, int32 Current, int32 Required)
{
	if (ObjectiveText.IsEmpty())
	{
		bShowObjective = false;
		ActiveObjectiveText = FText::GetEmpty();
	}
	else
	{
		bShowObjective = true;
		if (Required > 1)
		{
			ActiveObjectiveText = FText::Format(FText::FromString(TEXT("{0} ({1}/{2})")), ObjectiveText, FText::AsNumber(Current), FText::AsNumber(Required));
		}
		else
		{
			ActiveObjectiveText = ObjectiveText;
		}
	}
}
