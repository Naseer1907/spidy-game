// Copyright SpiderHero Team. All Rights Reserved.

#include "Core/SpiderHeroGameMode.h"
#include "Character/SpiderHeroCharacter.h"
#include "Core/SpiderHeroPlayerController.h"
#include "Core/SpiderHeroGameState.h"
#include "SpiderHero.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ASpiderHeroGameMode::ASpiderHeroGameMode()
{
	DefaultPawnClass = ASpiderHeroCharacter::StaticClass();
	PlayerControllerClass = ASpiderHeroPlayerController::StaticClass();
	GameStateClass = ASpiderHeroGameState::StaticClass();

	RespawnDelay = 3.0f;
	bAutoRespawnPlayer = true;
	CurrentCheckpointTransform = FTransform::Identity;
}

void ASpiderHeroGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogSpiderHero, Log, TEXT("SpiderHeroGameMode::InitGame - Map: %s, Options: %s"), *MapName, *Options);
}

void ASpiderHeroGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Cache initial player start position as default checkpoint if not set
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	if (PlayerStarts.Num() > 0 && CurrentCheckpointTransform.Equals(FTransform::Identity))
	{
		CurrentCheckpointTransform = PlayerStarts[0]->GetActorTransform();
	}

	UE_LOG(LogSpiderHero, Log, TEXT("SpiderHeroGameMode::BeginPlay initialized."));
}

void ASpiderHeroGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (NewPlayer && NewPlayer->GetPawn())
	{
		ASpiderHeroCharacter* SpiderChar = Cast<ASpiderHeroCharacter>(NewPlayer->GetPawn());
		if (SpiderChar)
		{
			OnPlayerRespawned.Broadcast(SpiderChar);
			UE_LOG(LogSpiderHero, Log, TEXT("Player %s restarted successfully."), *NewPlayer->GetName());
		}
	}
}

void ASpiderHeroGameMode::RespawnPlayerAtCheckpoint(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	APawn* CurrentPawn = Controller->GetPawn();
	if (CurrentPawn)
	{
		CurrentPawn->Destroy();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UClass* PawnClassToSpawn = DefaultPawnClass ? DefaultPawnClass : ASpiderHeroCharacter::StaticClass();
	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, CurrentCheckpointTransform.GetLocation(), CurrentCheckpointTransform.GetRotation().Rotator(), SpawnParams);

	if (NewPawn)
	{
		Controller->Possess(NewPawn);
		ASpiderHeroCharacter* SpiderChar = Cast<ASpiderHeroCharacter>(NewPawn);
		if (SpiderChar)
		{
			OnPlayerRespawned.Broadcast(SpiderChar);
		}
		UE_LOG(LogSpiderHero, Log, TEXT("Respawned player at checkpoint %s"), *CurrentCheckpointTransform.GetLocation().ToString());
	}
}

void ASpiderHeroGameMode::SetCurrentCheckpoint(const FTransform& NewCheckpointTransform)
{
	CurrentCheckpointTransform = NewCheckpointTransform;
	UE_LOG(LogSpiderHero, Log, TEXT("New checkpoint saved at: %s"), *CurrentCheckpointTransform.GetLocation().ToString());
}

void ASpiderHeroGameMode::OnPlayerCharacterKilled(AController* VictimController, AActor* KillerActor)
{
	UE_LOG(LogSpiderHero, Warning, TEXT("Player %s killed by %s"), 
		VictimController ? *VictimController->GetName() : TEXT("Unknown"), 
		KillerActor ? *KillerActor->GetName() : TEXT("Unknown"));

	if (bAutoRespawnPlayer && VictimController)
	{
		GetWorld()->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			FTimerDelegate::CreateUObject(this, &ASpiderHeroGameMode::RespawnPlayerAtCheckpoint, VictimController),
			RespawnDelay,
			false
		);
	}
}
