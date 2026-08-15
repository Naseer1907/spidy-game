// Copyright SpiderHero Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpiderDebugManager.generated.h"

class ASpiderHeroCharacter;

/**
 * USpiderDebugManager
 * World subsystem providing real-time debug visualization commands for movement physics,
 * web targeting frustums, pendulum forces, combat hitboxes, and traversal ledges.
 */
UCLASS()
class SPIDERHERO_API USpiderDebugManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USpiderDebugManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Toggle Movement physics debugging */
	UFUNCTION(Exec, Category = "Spider|Debug")
	void ToggleDebugMovement();

	/** Toggle Web physics and targeting debugging */
	UFUNCTION(Exec, Category = "Spider|Debug")
	void ToggleDebugWeb();

	/** Toggle Combat hitboxes and damage debugging */
	UFUNCTION(Exec, Category = "Spider|Debug")
	void ToggleDebugCombat();

	/** Toggle Traversal ledge scans */
	UFUNCTION(Exec, Category = "Spider|Debug")
	void ToggleDebugTraversal();

	/** Toggle all debug visualizers simultaneously */
	UFUNCTION(Exec, Category = "Spider|Debug")
	void ToggleDebugAll();

	bool IsDebugMovementEnabled() const { return bDebugMovement; }
	bool IsDebugWebEnabled() const { return bDebugWeb; }
	bool IsDebugCombatEnabled() const { return bDebugCombat; }
	bool IsDebugTraversalEnabled() const { return bDebugTraversal; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Spider|Debug")
	bool bDebugMovement;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Debug")
	bool bDebugWeb;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Debug")
	bool bDebugCombat;

	UPROPERTY(VisibleAnywhere, Category = "Spider|Debug")
	bool bDebugTraversal;
};
