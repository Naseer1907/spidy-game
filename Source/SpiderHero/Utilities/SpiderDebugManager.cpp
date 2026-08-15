// Copyright SpiderHero Team. All Rights Reserved.

#include "Utilities/SpiderDebugManager.h"
#include "SpiderHero.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

USpiderDebugManager::USpiderDebugManager()
{
	bDebugMovement = false;
	bDebugWeb = false;
	bDebugCombat = false;
	bDebugTraversal = false;
}

void USpiderDebugManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderDebugManager Subsystem initialized."));
}

void USpiderDebugManager::ToggleDebugMovement()
{
	bDebugMovement = !bDebugMovement;
	UE_LOG(LogSpiderMovement, Warning, TEXT("Debug Movement: %s"), bDebugMovement ? TEXT("ON") : TEXT("OFF"));
}

void USpiderDebugManager::ToggleDebugWeb()
{
	bDebugWeb = !bDebugWeb;
	UE_LOG(LogSpiderWeb, Warning, TEXT("Debug Web: %s"), bDebugWeb ? TEXT("ON") : TEXT("OFF"));
}

void USpiderDebugManager::ToggleDebugCombat()
{
	bDebugCombat = !bDebugCombat;
	UE_LOG(LogSpiderCombat, Warning, TEXT("Debug Combat: %s"), bDebugCombat ? TEXT("ON") : TEXT("OFF"));
}

void USpiderDebugManager::ToggleDebugTraversal()
{
	bDebugTraversal = !bDebugTraversal;
	UE_LOG(LogSpiderMovement, Warning, TEXT("Debug Traversal: %s"), bDebugTraversal ? TEXT("ON") : TEXT("OFF"));
}

void USpiderDebugManager::ToggleDebugAll()
{
	bool bNewState = !bDebugMovement;
	bDebugMovement = bNewState;
	bDebugWeb = bNewState;
	bDebugCombat = bNewState;
	bDebugTraversal = bNewState;
	UE_LOG(LogSpiderHero, Warning, TEXT("Debug All Systems: %s"), bNewState ? TEXT("ON") : TEXT("OFF"));
}
