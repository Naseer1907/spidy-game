// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UCanvasPanel;

/**
 * USpiderHUDWidget
 * Modern Minimalist HUD: Health, Stamina, XP, Combo counter, Web cartridges, Reticle, Spider-Sense alert, Objectives.
 */
UCLASS(Abstract, Blueprintable)
class SPIDERHERO_API USpiderHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// UI Updaters
	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void UpdateHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void UpdateStamina(float CurrentStamina, float MaxStamina, bool bIsFatigued);

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void UpdateXP(int32 CurrentXP, int32 XPForNextLevel, int32 Level);

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void UpdateCombo(int32 ComboCount, float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void UpdateWebAmmo(int32 CurrentShots, int32 MaxShots);

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void UpdateReticle(const FVector2D& ScreenPosition, bool bHasTarget);

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void ShowSpiderSenseAlert(ESpiderSenseAlertLevel AlertLevel, float TimeToImpact);

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void HideSpiderSenseAlert();

	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void UpdateObjectivePrompt(const FText& ObjectiveText, int32 Current, int32 Required);

protected:
	// State for HUD rendering
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	float HealthPercent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	float StaminaPercent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	float XPPercent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	int32 CurrentLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	int32 ComboCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	float ComboMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	int32 WebShotsRemaining;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	int32 MaxWebShotsCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	FVector2D ReticleScreenPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	bool bReticleLocked;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	ESpiderSenseAlertLevel ActiveSpiderSenseLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	FText ActiveObjectiveText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	bool bShowObjective;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|State")
	bool bIsFatiguedAlert;
};
