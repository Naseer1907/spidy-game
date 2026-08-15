// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Missions/SpiderMissionDataAsset.h"
#include "SpiderMissionManager.generated.h"

class ASpiderHeroCharacter;
class ASpiderHeroGameState;
class USpiderProgressionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStartedSignature, const USpiderMissionDataAsset*, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdatedSignature, const FSpiderMissionObjective&, Objective, int32, ObjectiveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionCompletedSignature, const USpiderMissionDataAsset*, Mission, int32, XPEarned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailedSignature, const USpiderMissionDataAsset*, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynamicCrimeSpawnedSignature, FName, CrimeID, const FVector&, Location);

/**
 * USpiderMissionManager
 * Manages active mission lifecycle, objective tracking, rewards dispatch,
 * dynamic crime generation, and the tutorial mission "Rooftop Disturbance".
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPIDERHERO_API USpiderMissionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	USpiderMissionManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Starts a specific mission */
	UFUNCTION(BlueprintCallable, Category = "Spider|Missions")
	bool StartMission(USpiderMissionDataAsset* MissionAsset);

	/** Advances progress on an objective by ID */
	UFUNCTION(BlueprintCallable, Category = "Spider|Missions")
	bool AdvanceObjective(FName ObjectiveID, int32 Amount = 1);

	/** Manually complete the active mission */
	UFUNCTION(BlueprintCallable, Category = "Spider|Missions")
	void CompleteActiveMission();

	/** Fail the active mission */
	UFUNCTION(BlueprintCallable, Category = "Spider|Missions")
	void FailActiveMission();

	/** Abandon current active mission */
	UFUNCTION(BlueprintCallable, Category = "Spider|Missions")
	void AbandonActiveMission();

	/** Setup and start the default tutorial mission "Rooftop Disturbance" */
	UFUNCTION(BlueprintCallable, Category = "Spider|Missions|Tutorial")
	void StartTutorialMission();

	/** Attempts to spawn a dynamic crime in the world around player */
	UFUNCTION(BlueprintCallable, Category = "Spider|Missions|Crimes")
	void TriggerDynamicCrime();

	/** Get current active mission */
	UFUNCTION(BlueprintPure, Category = "Spider|Missions")
	USpiderMissionDataAsset* GetActiveMission() const { return ActiveMissionAsset; }

	/** Get current active objectives array */
	UFUNCTION(BlueprintPure, Category = "Spider|Missions")
	const TArray<FSpiderMissionObjective>& GetActiveObjectives() const { return CurrentObjectives; }

	/** Check if player has an active mission */
	UFUNCTION(BlueprintPure, Category = "Spider|Missions")
	bool HasActiveMission() const { return ActiveMissionAsset != nullptr; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Missions|Events")
	FOnMissionStartedSignature OnMissionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Missions|Events")
	FOnObjectiveUpdatedSignature OnObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Missions|Events")
	FOnMissionCompletedSignature OnMissionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Missions|Events")
	FOnMissionFailedSignature OnMissionFailed;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Missions|Events")
	FOnDynamicCrimeSpawnedSignature OnDynamicCrimeSpawned;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Missions|State")
	TObjectPtr<USpiderMissionDataAsset> ActiveMissionAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Missions|State")
	TArray<FSpiderMissionObjective> CurrentObjectives;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Missions|State")
	TArray<FName> CompletedMissionIDs;

	// Dynamic Crime Spawner Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Missions|Crimes")
	bool bEnableDynamicCrimes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Missions|Crimes")
	float DynamicCrimeSpawnInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Missions|Crimes")
	float DynamicCrimeMinRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Missions|Crimes")
	float DynamicCrimeMaxRadius;

private:
	float TimeSinceLastCrimeSpawn;
	TWeakObjectPtr<ASpiderHeroCharacter> CharacterOwner;

	void CheckAllObjectivesComplete();
};
