// Copyright SpiderHero Team. All Rights Reserved.

#include "Character/SpiderHeroCharacter.h"
#include "SpiderHero.h"
#include "Core/SpiderHeroPlayerController.h"
#include "Movement/SpiderMovementComponent.h"
#include "Traversal/SpiderTraversalComponent.h"
#include "Web/SpiderWebTargetingComponent.h"
#include "Web/SpiderWebSwingComponent.h"
#include "Web/SpiderWebZipComponent.h"
#include "Web/SpiderWebShootingComponent.h"
#include "Abilities/SpiderHealthComponent.h"
#include "Abilities/SpiderStaminaComponent.h"
#include "Progression/SpiderProgressionComponent.h"
#include "Missions/SpiderMissionManager.h"
#include "Audio/SpiderAudioComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/DamageEvents.h"

ASpiderHeroCharacter::ASpiderHeroCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USpiderMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("SpiderPawn"));

	// Character Rotation Setup
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Spider Movement Component Configuration
	SpiderMovementComp = Cast<USpiderMovementComponent>(GetCharacterMovement());
	if (SpiderMovementComp)
	{
		SpiderMovementComp->bOrientRotationToMovement = true;
		SpiderMovementComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		SpiderMovementComp->JumpZVelocity = 750.0f;
		SpiderMovementComp->AirControl = 0.45f;
		SpiderMovementComp->MaxWalkSpeed = 600.0f;
		SpiderMovementComp->MinAnalogWalkSpeed = 20.0f;
		SpiderMovementComp->BrakingDecelerationWalking = 2000.0f;
		SpiderMovementComp->GravityScale = 1.6f;
		SpiderMovementComp->MaxStepHeight = 45.0f;
		SpiderMovementComp->SetWalkableFloorAngle(50.0f);
	}

	// Camera Boom Setup
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.0f;
	CameraBoom->CameraLagMaxDistance = 80.0f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 15.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 60.0f);

	// Follow Camera Setup
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetFieldOfView(90.0f);

	// Abilities Components
	HealthComponent = CreateDefaultSubobject<USpiderHealthComponent>(TEXT("HealthComponent"));
	StaminaComponent = CreateDefaultSubobject<USpiderStaminaComponent>(TEXT("StaminaComponent"));

	// Traversal & Web Components
	TraversalComp = CreateDefaultSubobject<USpiderTraversalComponent>(TEXT("TraversalComponent"));
	WebTargetingComp = CreateDefaultSubobject<USpiderWebTargetingComponent>(TEXT("WebTargetingComponent"));
	WebSwingComp = CreateDefaultSubobject<USpiderWebSwingComponent>(TEXT("WebSwingComponent"));
	WebZipComp = CreateDefaultSubobject<USpiderWebZipComponent>(TEXT("WebZipComponent"));
	WebShootingComp = CreateDefaultSubobject<USpiderWebShootingComponent>(TEXT("WebShootingComponent"));

	// Progression, Missions & Audio
	ProgressionComp = CreateDefaultSubobject<USpiderProgressionComponent>(TEXT("ProgressionComponent"));
	MissionManager = CreateDefaultSubobject<USpiderMissionManager>(TEXT("MissionManager"));
	AudioComp = CreateDefaultSubobject<USpiderAudioComponent>(TEXT("AudioComponent"));

	// State Defaults
	SpiderMovementMode = ESpiderCustomMovementMode::None;
	TraversalState = ESpiderTraversalState::None;
	SwingState = ESpiderSwingState::None;
	CombatState = ESpiderCombatState::Neutral;
	WallRunSide = ESpiderWallRunSide::None;
	bIsSprinting = false;
	bIsParkouring = false;

	// Config Speeds
	JogSpeed = 600.0f;
	SprintSpeed = 1100.0f;
	WallRunSpeed = 1350.0f;

	// Dynamic Camera Defaults
	BaseArmLength = 400.0f;
	MaxSwingArmLength = 550.0f;
	BaseFOV = 90.0f;
	MaxSpeedFOV = 105.0f;
	CameraInterpSpeed = 4.0f;
}

void ASpiderHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ASpiderHeroCharacter::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &ASpiderHeroCharacter::HandleDeath);
	}

	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaChanged.AddDynamic(this, &ASpiderHeroCharacter::HandleStaminaChanged);
	}

	UE_LOG(LogSpiderHero, Log, TEXT("SpiderHeroCharacter fully initialized with all systems: %s"), *GetName());
}

void ASpiderHeroCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateCameraDynamics(DeltaSeconds);
}

void ASpiderHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	ASpiderHeroPlayerController* PC = Cast<ASpiderHeroPlayerController>(GetController());

	if (!EnhancedInputComp || !PC)
	{
		return;
	}

	// Movement & Look
	if (PC->IA_Move)
	{
		EnhancedInputComp->BindAction(PC->IA_Move, ETriggerEvent::Triggered, this, &ASpiderHeroCharacter::Input_Move);
	}
	if (PC->IA_Look)
	{
		EnhancedInputComp->BindAction(PC->IA_Look, ETriggerEvent::Triggered, this, &ASpiderHeroCharacter::Input_Look);
	}

	// Jump
	if (PC->IA_Jump)
	{
		EnhancedInputComp->BindAction(PC->IA_Jump, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_JumpStart);
		EnhancedInputComp->BindAction(PC->IA_Jump, ETriggerEvent::Completed, this, &ASpiderHeroCharacter::Input_JumpStop);
	}

	// Sprint
	if (PC->IA_Sprint)
	{
		EnhancedInputComp->BindAction(PC->IA_Sprint, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_SprintStart);
		EnhancedInputComp->BindAction(PC->IA_Sprint, ETriggerEvent::Completed, this, &ASpiderHeroCharacter::Input_SprintStop);
	}

	// Parkour
	if (PC->IA_Parkour)
	{
		EnhancedInputComp->BindAction(PC->IA_Parkour, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_ParkourStart);
		EnhancedInputComp->BindAction(PC->IA_Parkour, ETriggerEvent::Completed, this, &ASpiderHeroCharacter::Input_ParkourStop);
	}

	// Web Actions
	if (PC->IA_WebSwing)
	{
		EnhancedInputComp->BindAction(PC->IA_WebSwing, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_WebSwingStart);
		EnhancedInputComp->BindAction(PC->IA_WebSwing, ETriggerEvent::Completed, this, &ASpiderHeroCharacter::Input_WebSwingStop);
	}
	if (PC->IA_WebZip)
	{
		EnhancedInputComp->BindAction(PC->IA_WebZip, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_WebZipStart);
	}
	if (PC->IA_PointZip)
	{
		EnhancedInputComp->BindAction(PC->IA_PointZip, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_PointZipStart);
	}
	if (PC->IA_WebShoot)
	{
		EnhancedInputComp->BindAction(PC->IA_WebShoot, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_WebShootStart);
	}

	// Combat
	if (PC->IA_LightAttack)
	{
		EnhancedInputComp->BindAction(PC->IA_LightAttack, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_LightAttack);
	}
	if (PC->IA_HeavyAttack)
	{
		EnhancedInputComp->BindAction(PC->IA_HeavyAttack, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_HeavyAttack);
	}
	if (PC->IA_Dodge)
	{
		EnhancedInputComp->BindAction(PC->IA_Dodge, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_Dodge);
	}
	if (PC->IA_Finisher)
	{
		EnhancedInputComp->BindAction(PC->IA_Finisher, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_Finisher);
	}

	// General
	if (PC->IA_Interact)
	{
		EnhancedInputComp->BindAction(PC->IA_Interact, ETriggerEvent::Started, this, &ASpiderHeroCharacter::Input_Interact);
	}
}

void ASpiderHeroCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (SpiderMovementMode != ESpiderCustomMovementMode::None)
	{
		SetSpiderMovementMode(ESpiderCustomMovementMode::None);
	}
	if (SwingState != ESpiderSwingState::None)
	{
		SetSwingState(ESpiderSwingState::None);
	}
	if (TraversalState != ESpiderTraversalState::None)
	{
		SetTraversalState(ESpiderTraversalState::None);
	}
}

void ASpiderHeroCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	UE_LOG(LogSpiderMovement, Verbose, TEXT("Movement mode changed to: %d"), static_cast<int32>(GetCharacterMovement()->MovementMode));
}

float ASpiderHeroCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (HealthComponent && ActualDamage > 0.0f)
	{
		FHitResult HitInfo;
		FVector ImpulseDir;
		DamageEvent.GetBestHitInfo(this, DamageCauser, HitInfo, ImpulseDir);
		ActualDamage = HealthComponent->ApplyDamage(ActualDamage, DamageCauser, HitInfo);
	}

	return ActualDamage;
}

void ASpiderHeroCharacter::SetSpiderMovementMode(ESpiderCustomMovementMode NewMode)
{
	if (SpiderMovementMode != NewMode)
	{
		SpiderMovementMode = NewMode;
		OnCustomMovementModeChanged.Broadcast(SpiderMovementMode);
		UE_LOG(LogSpiderMovement, Log, TEXT("Spider Movement Mode set to: %d"), static_cast<int32>(SpiderMovementMode));
	}
}

void ASpiderHeroCharacter::SetTraversalState(ESpiderTraversalState NewState)
{
	if (TraversalState != NewState)
	{
		TraversalState = NewState;
		OnTraversalStateChanged.Broadcast(TraversalState);
		UE_LOG(LogSpiderMovement, Log, TEXT("Traversal State set to: %d"), static_cast<int32>(TraversalState));
	}
}

void ASpiderHeroCharacter::SetSwingState(ESpiderSwingState NewState)
{
	if (SwingState != NewState)
	{
		SwingState = NewState;
		OnSwingStateChanged.Broadcast(SwingState);
		UE_LOG(LogSpiderWeb, Log, TEXT("Swing State set to: %d"), static_cast<int32>(SwingState));
	}
}

void ASpiderHeroCharacter::SetCombatState(ESpiderCombatState NewState)
{
	if (CombatState != NewState)
	{
		CombatState = NewState;
		OnCombatStateChanged.Broadcast(CombatState);
		UE_LOG(LogSpiderCombat, Log, TEXT("Combat State set to: %d"), static_cast<int32>(CombatState));
	}
}

void ASpiderHeroCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (SwingState == ESpiderSwingState::Swinging && WebSwingComp)
	{
		WebSwingComp->ApplySteeringInput(MovementVector);
	}

	if (Controller && !MovementVector.IsNearlyZero())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASpiderHeroCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASpiderHeroCharacter::Input_JumpStart(const FInputActionValue& Value)
{
	// 1. Check Point Launch during Zip
	if (WebZipComp && WebZipComp->IsZipping() && WebZipComp->TryTriggerPointLaunch())
	{
		if (AudioComp)
		{
			AudioComp->PlayPointLaunchSound();
		}
		return;
	}

	// 2. Check Wall Jump during Wall Run
	if (SpiderMovementComp && SpiderMovementComp->GetSpiderCustomMovementMode() == ESpiderCustomMovementMode::WallRunning)
	{
		SpiderMovementComp->PerformWallJump();
		return;
	}

	// 3. Check Parkour Traversal
	if (TraversalComp && TraversalComp->TryPerformTraversal())
	{
		return;
	}

	// 4. Default Jump
	Jump();
}

void ASpiderHeroCharacter::Input_JumpStop(const FInputActionValue& Value)
{
	StopJumping();
}

void ASpiderHeroCharacter::Input_SprintStart(const FInputActionValue& Value)
{
	bIsSprinting = true;
	if (SpiderMovementComp)
	{
		SpiderMovementComp->MaxWalkSpeed = SprintSpeed;
		if (SpiderMovementComp->IsFalling())
		{
			SpiderMovementComp->TryStartWallRun();
		}
	}
}

void ASpiderHeroCharacter::Input_SprintStop(const FInputActionValue& Value)
{
	bIsSprinting = false;
	if (SpiderMovementComp)
	{
		SpiderMovementComp->MaxWalkSpeed = JogSpeed;
	}
}

void ASpiderHeroCharacter::Input_ParkourStart(const FInputActionValue& Value)
{
	bIsParkouring = true;
	if (TraversalComp)
	{
		TraversalComp->TryPerformTraversal();
	}
}

void ASpiderHeroCharacter::Input_ParkourStop(const FInputActionValue& Value)
{
	bIsParkouring = false;
}

void ASpiderHeroCharacter::Input_WebSwingStart(const FInputActionValue& Value)
{
	if (WebSwingComp && WebSwingComp->StartWebSwing())
	{
		if (AudioComp)
		{
			AudioComp->PlayWebShootSound();
		}
	}
}

void ASpiderHeroCharacter::Input_WebSwingStop(const FInputActionValue& Value)
{
	if (WebSwingComp && WebSwingComp->IsSwinging())
	{
		WebSwingComp->ReleaseWebSwing(true);
		if (AudioComp)
		{
			AudioComp->PlayWebReleaseSound(1.0f);
		}
	}
}

void ASpiderHeroCharacter::Input_WebZipStart(const FInputActionValue& Value)
{
	if (WebZipComp && WebZipComp->StartPointZip())
	{
		if (AudioComp)
		{
			AudioComp->PlayWebShootSound();
		}
	}
}

void ASpiderHeroCharacter::Input_PointZipStart(const FInputActionValue& Value)
{
	if (WebZipComp && WebZipComp->StartPointZip())
	{
		if (AudioComp)
		{
			AudioComp->PlayWebShootSound();
		}
	}
}

void ASpiderHeroCharacter::Input_WebShootStart(const FInputActionValue& Value)
{
	if (WebShootingComp && WebShootingComp->FireWebShot())
	{
		if (AudioComp)
		{
			AudioComp->PlayWebShootSound();
		}
	}
}

void ASpiderHeroCharacter::Input_LightAttack(const FInputActionValue& Value)
{
	SetCombatState(ESpiderCombatState::LightAttacking);
	if (AudioComp)
	{
		AudioComp->PlayCombatHitSound(GetActorLocation(), false);
	}
}

void ASpiderHeroCharacter::Input_HeavyAttack(const FInputActionValue& Value)
{
	SetCombatState(ESpiderCombatState::HeavyAttacking);
	if (AudioComp)
	{
		AudioComp->PlayCombatHitSound(GetActorLocation(), true);
	}
}

void ASpiderHeroCharacter::Input_Dodge(const FInputActionValue& Value)
{
	SetCombatState(ESpiderCombatState::Dodging);
	if (HealthComponent)
	{
		HealthComponent->GrantInvulnerability(0.4f);
	}
}

void ASpiderHeroCharacter::Input_Finisher(const FInputActionValue& Value)
{
	SetCombatState(ESpiderCombatState::Finisher);
}

void ASpiderHeroCharacter::Input_Interact(const FInputActionValue& Value)
{
	UE_LOG(LogSpiderHero, Verbose, TEXT("Interact triggered"));
}

void ASpiderHeroCharacter::UpdateCameraDynamics(float DeltaSeconds)
{
	if (!CameraBoom || !FollowCamera)
	{
		return;
	}

	const float Speed = GetVelocity().Size();
	const float MaxExpectedSpeed = 2500.0f;
	const float SpeedAlpha = FMath::Clamp(Speed / MaxExpectedSpeed, 0.0f, 1.0f);

	// Target Arm Length calculation
	float TargetArmLength = FMath::Lerp(BaseArmLength, MaxSwingArmLength, SpeedAlpha);
	if (SwingState == ESpiderSwingState::Swinging)
	{
		TargetArmLength += 50.0f;
	}
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaSeconds, CameraInterpSpeed);

	// Target FOV calculation
	float TargetFOV = FMath::Lerp(BaseFOV, MaxSpeedFOV, SpeedAlpha);
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, CameraInterpSpeed));
}

void ASpiderHeroCharacter::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta, AActor* InstigatorActor)
{
}

void ASpiderHeroCharacter::HandleDeath(AActor* KillerActor)
{
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	}

	UE_LOG(LogSpiderHero, Warning, TEXT("Character %s died."), *GetName());
}

void ASpiderHeroCharacter::HandleStaminaChanged(float CurrentStamina, float MaxStamina, float Delta, bool bIsFatigued)
{
	if (bIsFatigued && bIsSprinting)
	{
		Input_SprintStop(FInputActionValue());
	}
}
