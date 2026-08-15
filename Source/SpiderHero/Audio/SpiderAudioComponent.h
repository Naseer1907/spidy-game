// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpiderAudioComponent.generated.h"

class ASpiderHeroCharacter;
class USoundBase;
class UAudioComponent;

/**
 * USpiderAudioComponent
 * Manages speed-reactive wind whoosh, procedural web-swing rope tension sounds,
 * combat impact layers, and Spider-Sense acoustic chimes.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderAudioComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Plays web shooter discharge sound */
	UFUNCTION(BlueprintCallable, Category = "Spider|Audio")
	void PlayWebShootSound();

	/** Plays web anchor impact/attach sound at location */
	UFUNCTION(BlueprintCallable, Category = "Spider|Audio")
	void PlayWebAttachSound(const FVector& Location);

	/** Plays web swing release whoosh */
	UFUNCTION(BlueprintCallable, Category = "Spider|Audio")
	void PlayWebReleaseSound(float BoostFactor);

	/** Plays melee combat impact sound */
	UFUNCTION(BlueprintCallable, Category = "Spider|Audio")
	void PlayCombatHitSound(const FVector& Location, bool bIsHeavy);

	/** Plays Spider-Sense warning acoustic cue */
	UFUNCTION(BlueprintCallable, Category = "Spider|Audio")
	void PlaySpiderSenseWarning(int32 AlertLevel);

	/** Plays Point Launch sonic burst */
	UFUNCTION(BlueprintCallable, Category = "Spider|Audio")
	void PlayPointLaunchSound();

protected:
	// Audio Assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> WebShootSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> WebAttachSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> WebReleaseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> LightHitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> HeavyHitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> SpiderSenseChime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> PointLaunchSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Audio|Assets")
	TObjectPtr<USoundBase> WindLoopSound;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Audio|State")
	float CurrentWindVolume;

private:
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;
	TObjectPtr<UAudioComponent> WindAudioComponent;

	void UpdateWindWhoosh(float DeltaTime);
};
