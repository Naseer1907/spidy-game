// Copyright SpiderHero Team. All Rights Reserved.

#include "UI/SpiderPauseMenuWidget.h"
#include "SpiderHero.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void USpiderPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsSettingsOpen = false;
	bIsControlsOpen = false;
	MasterVolume = 1.0f;
	SFXVolume = 1.0f;
	MusicVolume = 0.8f;
	GraphicsQualityPreset = 3; // Epic / High default
}

void USpiderPauseMenuWidget::ResumeGame()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, false);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	RemoveFromParent();
	OnGameResumed.Broadcast();
	UE_LOG(LogSpiderHero, Log, TEXT("Game Resumed."));
}

void USpiderPauseMenuWidget::OpenSettings()
{
	bIsSettingsOpen = true;
	bIsControlsOpen = false;
}

void USpiderPauseMenuWidget::OpenControlsGuide()
{
	bIsControlsOpen = true;
	bIsSettingsOpen = false;
}

void USpiderPauseMenuWidget::QuitToMainMenu()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, false);
		UGameplayStatics::OpenLevel(World, TEXT("Entry"));
	}
	OnQuitToMainMenu.Broadcast();
}

void USpiderPauseMenuWidget::QuitGame()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}
