// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Core/SpiderHeroTypes.h"
#include "SpiderHeroCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USpiderHealthComponent;
class USpiderStaminaComponent;
class USpiderMovementComponent;
class USpiderTraversalComponent;
class USpiderWebTargetingComponent;
class USpiderWebSwingComponent;
class USpiderWebZipComponent;
class USpiderWebShootingComponent;
class USpiderProgressionComponent;
class USpiderMissionManager;
class USpiderAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCustomMovementModeChangedSignature, ESpiderCustomMovementMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTraversalStateChangedSignature, ESpiderTraversalState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwingStateChangedSignature, ESpiderSwingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChangedSignature, ESpiderCombatState, NewState);

/**
 * ASpiderHeroCharacter
 * The primary playable hero character class implementing core movement, camera dynamics,
 * traversal hooks, web swinging hooks, and combat integration.
 */
UCLASS(Blueprintable)
class SPIDERHERO_API ASpiderHeroCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpiderHeroCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// Component Getters
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE USpiderHealthComponent* GetHealthComponent() const { return HealthComponent; }
	FORCEINLINE USpiderStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }
	FORCEINLINE USpiderMovementComponent* GetSpiderMovementComponent() const { return SpiderMovementComp; }
	FORCEINLINE USpiderTraversalComponent* GetTraversalComponent() const { return TraversalComp; }
	FORCEINLINE USpiderWebTargetingComponent* GetWebTargetingComponent() const { return WebTargetingComp; }
	FORCEINLINE USpiderWebSwingComponent* GetWebSwingComponent() const { return WebSwingComp; }
	FORCEINLINE USpiderWebZipComponent* GetWebZipComponent() const { return WebZipComp; }
	FORCEINLINE USpiderWebShootingComponent* GetWebShootingComponent() const { return WebShootingComp; }
	FORCEINLINE USpiderProgressionComponent* GetProgressionComponent() const { return ProgressionComp; }
	FORCEINLINE USpiderMissionManager* GetMissionManager() const { return MissionManager; }
	FORCEINLINE USpiderAudioComponent* GetAudioComponent() const { return AudioComp; }

	// State Getters & Setters
	UFUNCTION(BlueprintPure, Category = "Spider|State")
	ESpiderCustomMovementMode GetSpiderMovementMode() const { return SpiderMovementMode; }

	UFUNCTION(BlueprintCallable, Category = "Spider|State")
	void SetSpiderMovementMode(ESpiderCustomMovementMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	ESpiderTraversalState GetTraversalState() const { return TraversalState; }

	UFUNCTION(BlueprintCallable, Category = "Spider|State")
	void SetTraversalState(ESpiderTraversalState NewState);

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	ESpiderSwingState GetSwingState() const { return SwingState; }

	UFUNCTION(BlueprintCallable, Category = "Spider|State")
	void SetSwingState(ESpiderSwingState NewState);

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	ESpiderCombatState GetCombatState() const { return CombatState; }

	UFUNCTION(BlueprintCallable, Category = "Spider|State")
	void SetCombatState(ESpiderCombatState NewState);

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	ESpiderWallRunSide GetWallRunSide() const { return WallRunSide; }

	UFUNCTION(BlueprintCallable, Category = "Spider|State")
	void SetWallRunSide(ESpiderWallRunSide NewSide) { WallRunSide = NewSide; }

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	const FSpiderWebAnchorInfo& GetActiveAnchorInfo() const { return ActiveAnchorInfo; }

	UFUNCTION(BlueprintCallable, Category = "Spider|State")
	void SetActiveAnchorInfo(const FSpiderWebAnchorInfo& NewAnchor) { ActiveAnchorInfo = NewAnchor; }

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	bool IsWallRunning() const { return TraversalState == ESpiderTraversalState::WallRun; }

	UFUNCTION(BlueprintPure, Category = "Spider|State")
	bool IsSwinging() const { return SwingState == ESpiderSwingState::Swinging; }

	// Input Handlers
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_JumpStart(const FInputActionValue& Value);
	void Input_JumpStop(const FInputActionValue& Value);
	void Input_SprintStart(const FInputActionValue& Value);
	void Input_SprintStop(const FInputActionValue& Value);
	void Input_ParkourStart(const FInputActionValue& Value);
	void Input_ParkourStop(const FInputActionValue& Value);

	void Input_WebSwingStart(const FInputActionValue& Value);
	void Input_WebSwingStop(const FInputActionValue& Value);
	void Input_WebZipStart(const FInputActionValue& Value);
	void Input_PointZipStart(const FInputActionValue& Value);
	void Input_WebShootStart(const FInputActionValue& Value);

	void Input_LightAttack(const FInputActionValue& Value);
	void Input_HeavyAttack(const FInputActionValue& Value);
	void Input_Dodge(const FInputActionValue& Value);
	void Input_Finisher(const FInputActionValue& Value);
	void Input_Interact(const FInputActionValue& Value);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Spider|Events")
	FOnCustomMovementModeChangedSignature OnCustomMovementModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Events")
	FOnTraversalStateChangedSignature OnTraversalStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Events")
	FOnSwingStateChangedSignature OnSwingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Spider|Events")
	FOnCombatStateChangedSignature OnCombatStateChanged;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderStaminaComponent> StaminaComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderMovementComponent> SpiderMovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderTraversalComponent> TraversalComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderWebTargetingComponent> WebTargetingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderWebSwingComponent> WebSwingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderWebZipComponent> WebZipComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderWebShootingComponent> WebShootingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderProgressionComponent> ProgressionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderMissionManager> MissionManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpiderAudioComponent> AudioComp;

	// State Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	ESpiderCustomMovementMode SpiderMovementMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	ESpiderTraversalState TraversalState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	ESpiderSwingState SwingState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	ESpiderCombatState CombatState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	ESpiderWallRunSide WallRunSide;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	FSpiderWebAnchorInfo ActiveAnchorInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	bool bIsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider|State")
	bool bIsParkouring;

	// Movement Speeds
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Config")
	float JogSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Config")
	float SprintSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Movement|Config")
	float WallRunSpeed;

	// Dynamic Camera Configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Camera|Config")
	float BaseArmLength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Camera|Config")
	float MaxSwingArmLength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Camera|Config")
	float BaseFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Camera|Config")
	float MaxSpeedFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spider|Camera|Config")
	float CameraInterpSpeed;

	// Handlers for Component Events
	UFUNCTION()
	virtual void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta, AActor* InstigatorActor);

	UFUNCTION()
	virtual void HandleDeath(AActor* KillerActor);

	UFUNCTION()
	virtual void HandleStaminaChanged(float CurrentStamina, float MaxStamina, float Delta, bool bIsFatigued);

private:
	void UpdateCameraDynamics(float DeltaSeconds);
};
