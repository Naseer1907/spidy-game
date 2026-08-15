// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpiderHeroGameMode.generated.h"

class ASpiderHeroCharacter;
class ASpiderHeroPlayerController;
class ASpiderHeroGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRespawned, ASpiderHeroCharacter*, RespawnedCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChangedSignature, FName, NewStateName);

/**
 * ASpiderHeroGameMode
 * Base game mode for the SpiderHero open world experience.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderHeroGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASpiderHeroGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	/** Respawn character at last safe checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameMode")
	virtual void RespawnPlayerAtCheckpoint(AController* Controller);

	/** Set last active checkpoint transform */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameMode")
	void SetCurrentCheckpoint(const FTransform& NewCheckpointTransform);

	/** Get current checkpoint transform */
	UFUNCTION(BlueprintPure, Category = "Spider|GameMode")
	FTransform GetCurrentCheckpoint() const { return CurrentCheckpointTransform; }

	/** Called when player character dies */
	UFUNCTION(BlueprintCallable, Category = "Spider|GameMode")
	virtual void OnPlayerCharacterKilled(AController* VictimController, AActor* KillerActor);

	/** Event triggered upon player respawn */
	UPROPERTY(BlueprintAssignable, Category = "Spider|GameMode|Events")
	FOnPlayerRespawned OnPlayerRespawned;

	/** Event triggered when game phase changes */
	UPROPERTY(BlueprintAssignable, Category = "Spider|GameMode|Events")
	FOnGameStateChangedSignature OnGameStateChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|GameMode|Spawning")
	FTransform CurrentCheckpointTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|GameMode|Spawning")
	float RespawnDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|GameMode|Config")
	bool bAutoRespawnPlayer;

private:
	FTimerHandle RespawnTimerHandle;
};
