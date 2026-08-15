// Copyright SpiderHero Team. All Rights Reserved.

#include "AI/SpiderAIController.h"
#include "SpiderHero.h"
#include "AI/SpiderEnemyBase.h"
#include "AI/SpiderEnemyDirector.h"
#include "Character/SpiderHeroCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"

const FName ASpiderAIController::Key_TargetActor(TEXT("TargetActor"));
const FName ASpiderAIController::Key_TargetLocation(TEXT("TargetLocation"));
const FName ASpiderAIController::Key_SlotLocation(TEXT("SlotLocation"));
const FName ASpiderAIController::Key_HasAttackToken(TEXT("bHasAttackToken"));
const FName ASpiderAIController::Key_CombatState(TEXT("CombatState"));
const FName ASpiderAIController::Key_DistanceToTarget(TEXT("DistanceToTarget"));

ASpiderAIController::ASpiderAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	// Perception Component
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	// Sight Config
	SightRadius = 2500.0f;
	LoseSightRadius = 3200.0f;
	PeripheralVisionAngleDegrees = 90.0f;

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	// Hearing Config
	HearingRange = 2000.0f;
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = HearingRange;
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerceptionComp->ConfigureSense(*HearingConfig);

	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ASpiderAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledEnemy = Cast<ASpiderEnemyBase>(InPawn);

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ASpiderAIController::OnPerceptionTargetUpdated);
	}

	if (DefaultBehaviorTree)
	{
		RunBehaviorTree(DefaultBehaviorTree);
	}

	FindOrCreateDirector();
	if (EnemyDirector.IsValid() && ControlledEnemy.IsValid())
	{
		EnemyDirector->RegisterEnemy(ControlledEnemy.Get());
	}

	UE_LOG(LogSpiderAI, Log, TEXT("SpiderAIController possessed: %s"), InPawn ? *InPawn->GetName() : TEXT("None"));
}

void ASpiderAIController::OnUnPossess()
{
	if (EnemyDirector.IsValid() && ControlledEnemy.IsValid())
	{
		EnemyDirector->UnregisterEnemy(ControlledEnemy.Get());
	}

	ControlledEnemy = nullptr;
	Super::OnUnPossess();
}

void ASpiderAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateBlackboardState();
}

void ASpiderAIController::OnPerceptionTargetUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Actor->IsA<ASpiderHeroCharacter>())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		PerceivedTargetActor = Actor;
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsObject(Key_TargetActor, Actor);
			BB->SetValueAsVector(Key_TargetLocation, Actor->GetActorLocation());
		}
	}
}

bool ASpiderAIController::RequestAttackToken()
{
	if (!EnemyDirector.IsValid())
	{
		FindOrCreateDirector();
	}

	if (EnemyDirector.IsValid() && ControlledEnemy.IsValid())
	{
		const bool bGranted = EnemyDirector->RequestAttackToken(ControlledEnemy.Get());
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsBool(Key_HasAttackToken, bGranted);
		}
		return bGranted;
	}

	return false;
}

void ASpiderAIController::ReleaseAttackToken()
{
	if (EnemyDirector.IsValid() && ControlledEnemy.IsValid())
	{
		EnemyDirector->ReleaseAttackToken(ControlledEnemy.Get());
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsBool(Key_HasAttackToken, false);
		}
	}
}

void ASpiderAIController::SetSlotTargetLocation(const FVector& SlotWorldLocation)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(Key_SlotLocation, SlotWorldLocation);
	}
}

void ASpiderAIController::FindOrCreateDirector()
{
	if (EnemyDirector.IsValid())
	{
		return;
	}

	AActor* FoundDirector = UGameplayStatics::GetActorOfClass(GetWorld(), ASpiderEnemyDirector::StaticClass());
	if (FoundDirector)
	{
		EnemyDirector = Cast<ASpiderEnemyDirector>(FoundDirector);
	}
}

void ASpiderAIController::UpdateBlackboardState()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB || !ControlledEnemy.IsValid())
	{
		return;
	}

	BB->SetValueAsEnum(Key_CombatState, static_cast<uint8>(ControlledEnemy->GetAIState()));
	BB->SetValueAsBool(Key_HasAttackToken, ControlledEnemy->HasAttackToken());

	if (PerceivedTargetActor.IsValid())
	{
		const float Dist = FVector::Distance(ControlledEnemy->GetActorLocation(), PerceivedTargetActor->GetActorLocation());
		BB->SetValueAsFloat(Key_DistanceToTarget, Dist);
		BB->SetValueAsVector(Key_TargetLocation, PerceivedTargetActor->GetActorLocation());
	}
}
