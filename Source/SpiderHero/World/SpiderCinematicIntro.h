// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpiderCinematicIntro.generated.h"

class UCameraComponent;
class ASpiderHeroCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCinematicIntroCompleteSignature);

/**
 * ASpiderCinematicIntro
 * Orchestrates a seamless cinematic intro camera sequence:
 * high skyline panorama -> skyscraper descent -> hero rooftop framing -> gameplay handoff.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderCinematicIntro : public AActor
{
	GENERATED_BODY()

public:
	ASpiderCinematicIntro();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Starts the cinematic sequence */
	UFUNCTION(BlueprintCallable, Category = "Spider|Cinematic")
	void PlayIntroSequence();

	/** Skips the cinematic and immediately hands control to player */
	UFUNCTION(BlueprintCallable, Category = "Spider|Cinematic")
	void SkipIntro();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Cinematic|Events")
	FOnCinematicIntroCompleteSignature OnCinematicIntroComplete;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Cinematic")
	TObjectPtr<UCameraComponent> CinematicCamera;

	// Cinematic Waypoints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Cinematic|Waypoints")
	FTransform SkylineStartTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Cinematic|Waypoints")
	FTransform CanyonMidTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Cinematic|Waypoints")
	FTransform RooftopHeroTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Cinematic|Config")
	float TotalIntroDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Cinematic|Config")
	float HandoffBlendDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Cinematic|Config")
	bool bAutoPlayOnStart;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Cinematic|State")
	bool bIsPlayingIntro;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Cinematic|State")
	float IntroElapsedTime;

private:
	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<ASpiderHeroCharacter> TargetHero;

	void CompleteIntroSequence();
};
