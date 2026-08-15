// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpiderTestAutomation.generated.h"

USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderTestResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Testing")
	FString TestName = TEXT("");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Testing")
	bool bPassed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Testing")
	FString Details = TEXT("");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTestSuiteCompletedSignature, int32, TotalPassed, int32, TotalFailed);

/**
 * USpiderTestAutomation
 * Automated gameplay testing and quality assurance subsystem validating
 * movement transitions, web physics math, combat damage routing, mission flow, and save serialization.
 */
UCLASS()
class SPIDERHERO_API USpiderTestAutomation : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USpiderTestAutomation();

	/** Runs all automated test suites and outputs results */
	UFUNCTION(Exec, BlueprintCallable, Category = "Spider|Testing")
	void RunAllGameplayTests();

	UFUNCTION(BlueprintCallable, Category = "Spider|Testing")
	FSpiderTestResult TestMovementModeTransitions();

	UFUNCTION(BlueprintCallable, Category = "Spider|Testing")
	FSpiderTestResult TestWebPendulumCalculations();

	UFUNCTION(BlueprintCallable, Category = "Spider|Testing")
	FSpiderTestResult TestDamageAndHealthMitigation();

	UFUNCTION(BlueprintCallable, Category = "Spider|Testing")
	FSpiderTestResult TestMissionObjectiveProgression();

	UFUNCTION(BlueprintCallable, Category = "Spider|Testing")
	FSpiderTestResult TestSaveGameSerialization();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Testing|Events")
	FOnTestSuiteCompletedSignature OnTestSuiteCompleted;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Testing")
	TArray<FSpiderTestResult> LastTestResults;
};
