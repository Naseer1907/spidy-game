// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "SpiderHeroTypes.generated.h"

/**
 * Custom Collision & Trace Channels defined in DefaultEngine.ini
 */
#define SPIDER_TRACE_WEB_ANCHOR       ECC_GameTraceChannel1
#define SPIDER_TRACE_WALL_SURFACE     ECC_GameTraceChannel2
#define SPIDER_TRACE_COMBAT_TARGET    ECC_GameTraceChannel3
#define SPIDER_TRACE_ZIPLINE_ANCHOR   ECC_GameTraceChannel4
#define SPIDER_TRACE_PARKOUR_OBSTACLE ECC_GameTraceChannel5

/**
 * Custom Movement Modes for SpiderHero Character Movement Component
 */
UENUM(BlueprintType)
enum class ESpiderCustomMovementMode : uint8
{
	None             = 0 UMETA(DisplayName = "None"),
	WebSwinging      = 1 UMETA(DisplayName = "Web Swinging"),
	WallClimbing     = 2 UMETA(DisplayName = "Wall Climbing"),
	WallRunning      = 3 UMETA(DisplayName = "Wall Running"),
	Parkour          = 4 UMETA(DisplayName = "Parkour"),
	Ziplining        = 5 UMETA(DisplayName = "Ziplining"),
	CeilingClimbing  = 6 UMETA(DisplayName = "Ceiling Climbing"),
	Glide            = 7 UMETA(DisplayName = "Glide"),
	PointLaunch      = 8 UMETA(DisplayName = "Point Launch"),
	DiveBomb         = 9 UMETA(DisplayName = "Dive Bomb")
};

/**
 * Traversal state for obstacles, ledges, and parkour transitions
 */
UENUM(BlueprintType)
enum class ESpiderTraversalState : uint8
{
	None         = 0 UMETA(DisplayName = "None"),
	VaultLow     = 1 UMETA(DisplayName = "Vault Low"),
	VaultHigh    = 2 UMETA(DisplayName = "Vault High"),
	Mantle       = 3 UMETA(DisplayName = "Mantle"),
	LedgeClimb   = 4 UMETA(DisplayName = "Ledge Climb"),
	LedgeHang    = 5 UMETA(DisplayName = "Ledge Hang"),
	WallRun      = 6 UMETA(DisplayName = "Wall Run"),
	WallClimb    = 7 UMETA(DisplayName = "Wall Climb"),
	CornerTurn   = 8 UMETA(DisplayName = "Corner Turn"),
	DropHang     = 9 UMETA(DisplayName = "Drop Hang"),
	Perch        = 10 UMETA(DisplayName = "Perch"),
	Slide        = 11 UMETA(DisplayName = "Slide")
};

/**
 * Wall Run Side
 */
UENUM(BlueprintType)
enum class ESpiderWallRunSide : uint8
{
	None     = 0 UMETA(DisplayName = "None"),
	Left     = 1 UMETA(DisplayName = "Left"),
	Right    = 2 UMETA(DisplayName = "Right"),
	Vertical = 3 UMETA(DisplayName = "Vertical")
};

/**
 * Web Swing Execution States
 */
UENUM(BlueprintType)
enum class ESpiderSwingState : uint8
{
	None       = 0 UMETA(DisplayName = "None"),
	Attaching  = 1 UMETA(DisplayName = "Attaching Web"),
	Swinging   = 2 UMETA(DisplayName = "Swinging"),
	Releasing  = 3 UMETA(DisplayName = "Releasing Web"),
	DiveBomb   = 4 UMETA(DisplayName = "Dive Bomb"),
	Boost      = 5 UMETA(DisplayName = "Swing Boost"),
	PointZip   = 6 UMETA(DisplayName = "Point Zip"),
	PerchZip   = 7 UMETA(DisplayName = "Perch Zip")
};

/**
 * Combat Execution States
 */
UENUM(BlueprintType)
enum class ESpiderCombatState : uint8
{
	Neutral        = 0 UMETA(DisplayName = "Neutral"),
	LightAttacking = 1 UMETA(DisplayName = "Light Attacking"),
	HeavyAttacking = 2 UMETA(DisplayName = "Heavy Attacking"),
	WebShooting    = 3 UMETA(DisplayName = "Web Shooting"),
	Dodging        = 4 UMETA(DisplayName = "Dodging"),
	Parrying       = 5 UMETA(DisplayName = "Parrying"),
	HitStun        = 6 UMETA(DisplayName = "Hit Stun"),
	Finisher       = 7 UMETA(DisplayName = "Finisher"),
	AirCombo       = 8 UMETA(DisplayName = "Air Combo"),
	WebYank        = 9 UMETA(DisplayName = "Web Yank"),
	WebStrike      = 10 UMETA(DisplayName = "Web Strike")
};

/**
 * Types of attacks performable by SpiderHero
 */
UENUM(BlueprintType)
enum class ESpiderAttackType : uint8
{
	LightGround   = 0 UMETA(DisplayName = "Light Ground Attack"),
	HeavyGround   = 1 UMETA(DisplayName = "Heavy Ground Attack"),
	LightAir      = 2 UMETA(DisplayName = "Light Air Attack"),
	HeavyAir      = 3 UMETA(DisplayName = "Heavy Air Attack"),
	WebStrike     = 4 UMETA(DisplayName = "Web Strike"),
	GroundPound   = 5 UMETA(DisplayName = "Ground Pound"),
	Uppercut      = 6 UMETA(DisplayName = "Uppercut"),
	DodgeAttack   = 7 UMETA(DisplayName = "Dodge Attack"),
	Finisher      = 8 UMETA(DisplayName = "Finisher"),
	WebThrow      = 9 UMETA(DisplayName = "Web Throw")
};

/**
 * Parkour obstacle classification
 */
UENUM(BlueprintType)
enum class ESpiderParkourAction : uint8
{
	None        = 0 UMETA(DisplayName = "None"),
	VaultLow    = 1 UMETA(DisplayName = "Vault Low"),
	VaultHigh   = 2 UMETA(DisplayName = "Vault High"),
	Mantle      = 3 UMETA(DisplayName = "Mantle"),
	WallHop     = 4 UMETA(DisplayName = "Wall Hop"),
	LedgePullUp = 5 UMETA(DisplayName = "Ledge Pull Up"),
	SlideUnder  = 6 UMETA(DisplayName = "Slide Under"),
	BeamWalk    = 7 UMETA(DisplayName = "Beam Walk")
};

/**
 * Spider Sense Threat Level for UI & Dodge Window
 */
UENUM(BlueprintType)
enum class ESpiderSenseAlertLevel : uint8
{
	None          = 0 UMETA(DisplayName = "None"),
	WhiteWarning  = 1 UMETA(DisplayName = "White Warning (Distant)"),
	YellowDanger  = 2 UMETA(DisplayName = "Yellow Danger (Imminent)"),
	RedCritical   = 3 UMETA(DisplayName = "Red Critical (Perfect Dodge Window)")
};

/**
 * Web anchor connection information
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderWebAnchorInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Web")
	FVector AnchorPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Web")
	FVector AttachNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Web")
	TWeakObjectPtr<AActor> AttachedActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Web")
	TWeakObjectPtr<UPrimitiveComponent> AttachedComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Web")
	float CableLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Web")
	bool bIsValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Web")
	bool bIsLeftHand = false;

	void Reset()
	{
		AnchorPoint = FVector::ZeroVector;
		AttachNormal = FVector::UpVector;
		AttachedActor = nullptr;
		AttachedComponent = nullptr;
		CableLength = 0.0f;
		bIsValid = false;
		bIsLeftHand = false;
	}
};

/**
 * Attack Configuration Data
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	ESpiderAttackType AttackType = ESpiderAttackType::LightGround;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float BaseDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float StaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float KnockbackForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float HitStunDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float AttackRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	FName MontageSectionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	bool bIsAirAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	bool bCanChain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float ChainWindowStart = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float ChainWindowEnd = 0.7f;
};

/**
 * Information passed when an entity receives damage or is hit
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderHitReaction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	float DamageTaken = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	TWeakObjectPtr<AActor> Attacker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	FVector ImpactForce = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	bool bIsCritical = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	bool bCausedStun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	bool bKnockdown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	bool bWasBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Combat")
	bool bWasParried = false;
};

/**
 * Incoming Threat Information for Spider-Sense
 */
USTRUCT(BlueprintType)
struct SPIDERHERO_API FSpiderThreatInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Sense")
	TWeakObjectPtr<AActor> ThreatSource = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Sense")
	FVector ThreatLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Sense")
	ESpiderSenseAlertLevel AlertLevel = ESpiderSenseAlertLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Sense")
	float TimeToImpact = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Sense")
	bool bIsProjectile = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider|Sense")
	bool bIsUnblockable = false;
};
