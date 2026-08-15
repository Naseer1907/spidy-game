// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpiderPauseMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameResumedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuitToMainMenuSignature);

/**
 * USpiderPauseMenuWidget
 * Pause menu controlling game flow, controls display, audio/graphics settings, and quit operations.
 */
UCLASS(Abstract, Blueprintable)
class SPIDERHERO_API USpiderPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Resumes the game and closes pause menu */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Pause")
	void ResumeGame();

	/** Opens Settings submenu */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Pause")
	void OpenSettings();

	/** Opens Controls reference guide */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Pause")
	void OpenControlsGuide();

	/** Quit to Main Menu */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Pause")
	void QuitToMainMenu();

	/** Quit to Desktop */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI|Pause")
	void QuitGame();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|UI|Pause|Events")
	FOnGameResumedSignature OnGameResumed;

	UPROPERTY(BlueprintAssignable, Category = "Spider|UI|Pause|Events")
	FOnQuitToMainMenuSignature OnQuitToMainMenu;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|Pause|State")
	bool bIsSettingsOpen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|UI|Pause|State")
	bool bIsControlsOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|UI|Pause|Settings")
	float MasterVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|UI|Pause|Settings")
	float SFXVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|UI|Pause|Settings")
	float MusicVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|UI|Pause|Settings")
	int32 GraphicsQualityPreset;
};
