// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderHeroPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpiderSenseTriggeredSignature, const FSpiderThreatInfo&, ThreatInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputContextChangedSignature, const UInputMappingContext*, ActiveContext);

/**
 * ASpiderHeroPlayerController
 * Manages Enhanced Input Contexts, UI interaction, and Player input flow.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderHeroPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASpiderHeroPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Adds an Enhanced Input Mapping Context with a given priority */
	UFUNCTION(BlueprintCallable, Category = "Spider|Input")
	void AddMappingContext(const UInputMappingContext* ContextToAdd, int32 Priority = 0);

	/** Removes an Enhanced Input Mapping Context */
	UFUNCTION(BlueprintCallable, Category = "Spider|Input")
	void RemoveMappingContext(const UInputMappingContext* ContextToRemove);

	/** Trigger Spider-Sense threat alert in HUD */
	UFUNCTION(BlueprintCallable, Category = "Spider|UI")
	void NotifySpiderSenseThreat(const FSpiderThreatInfo& ThreatInfo);

	// Contexts
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Contexts")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Contexts")
	TObjectPtr<UInputMappingContext> SwingMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Contexts")
	TObjectPtr<UInputMappingContext> CombatMappingContext;

	// Input Actions - Movement & Traversal
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Parkour;

	// Input Actions - Web Mechanics
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_WebSwing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_WebZip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_PointZip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_WebShoot;

	// Input Actions - Combat
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_LightAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_HeavyAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Dodge;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Finisher;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_SpideySense;

	// Input Actions - General
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider|Input|Actions")
	TObjectPtr<UInputAction> IA_Pause;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Spider|Events")
	FOnSpiderSenseTriggeredSignature OnSpiderSenseTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Events")
	FOnInputContextChangedSignature OnInputContextChanged;

protected:
	void SetupDefaultInputContext();
};
