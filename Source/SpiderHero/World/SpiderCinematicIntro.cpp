// Copyright SpiderHero Team. All Rights Reserved.

#include "World/SpiderCinematicIntro.h"
#include "Character/SpiderHeroCharacter.h"
#include "SpiderHero.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

ASpiderCinematicIntro::ASpiderCinematicIntro()
{
	PrimaryActorTick.bCanEverTick = true;

	CinematicCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CinematicCamera"));
	RootComponent = CinematicCamera;

	TotalIntroDuration = 6.0f;
	HandoffBlendDuration = 1.5f;
	bAutoPlayOnStart = false;
	bIsPlayingIntro = false;
	IntroElapsedTime = 0.0f;

	// Initial default transforms
	SkylineStartTransform = FTransform(FRotator(-20.0f, 45.0f, 0.0f), FVector(0.0f, 0.0f, 15000.0f));
	CanyonMidTransform = FTransform(FRotator(-10.0f, 60.0f, 0.0f), FVector(4000.0f, 3000.0f, 7500.0f));
	RooftopHeroTransform = FTransform(FRotator(-5.0f, 90.0f, 0.0f), FVector(500.0f, 200.0f, 2500.0f));
}

void ASpiderCinematicIntro::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	TArray<AActor*> Heroes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpiderHeroCharacter::StaticClass(), Heroes);
	if (Heroes.Num() > 0)
	{
		TargetHero = Cast<ASpiderHeroCharacter>(Heroes[0]);
		if (TargetHero.IsValid())
		{
			FVector HeroLoc = TargetHero->GetActorLocation();
			RooftopHeroTransform = FTransform(
				TargetHero->GetActorRotation(),
				HeroLoc + (-TargetHero->GetActorForwardVector() * 350.0f) + FVector(0.0f, 0.0f, 120.0f)
			);
		}
	}

	if (bAutoPlayOnStart)
	{
		PlayIntroSequence();
	}
}

void ASpiderCinematicIntro::PlayIntroSequence()
{
	if (!PlayerController.IsValid())
	{
		PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	}

	if (PlayerController.IsValid())
	{
		PlayerController->SetViewTarget(this);
		bIsPlayingIntro = true;
		IntroElapsedTime = 0.0f;
		SetActorTransform(SkylineStartTransform);
		UE_LOG(LogSpiderHero, Log, TEXT("Cinematic Intro started."));
	}
}

void ASpiderCinematicIntro::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsPlayingIntro)
	{
		return;
	}

	IntroElapsedTime += DeltaSeconds;
	float Alpha = FMath::Clamp(IntroElapsedTime / TotalIntroDuration, 0.0f, 1.0f);

	// Quadratic Bezier interpolation across 3 keyframes
	float OneMinusAlpha = 1.0f - Alpha;
	FVector P0 = SkylineStartTransform.GetLocation();
	FVector P1 = CanyonMidTransform.GetLocation();
	FVector P2 = RooftopHeroTransform.GetLocation();

	FVector NewLocation = (OneMinusAlpha * OneMinusAlpha * P0) + (2.0f * OneMinusAlpha * Alpha * P1) + (Alpha * Alpha * P2);

	FQuat Q0 = SkylineStartTransform.GetRotation();
	FQuat Q1 = CanyonMidTransform.GetRotation();
	FQuat Q2 = RooftopHeroTransform.GetRotation();

	FQuat Slerp1 = FQuat::Slerp(Q0, Q1, Alpha);
	FQuat Slerp2 = FQuat::Slerp(Q1, Q2, Alpha);
	FQuat NewRotation = FQuat::Slerp(Slerp1, Slerp2, Alpha);

	SetActorLocationAndRotation(NewLocation, NewRotation);

	if (IntroElapsedTime >= TotalIntroDuration)
	{
		CompleteIntroSequence();
	}
}

void ASpiderCinematicIntro::SkipIntro()
{
	if (bIsPlayingIntro)
	{
		CompleteIntroSequence();
	}
}

void ASpiderCinematicIntro::CompleteIntroSequence()
{
	bIsPlayingIntro = false;

	if (PlayerController.IsValid() && TargetHero.IsValid())
	{
		PlayerController->SetViewTargetWithBlend(TargetHero.Get(), HandoffBlendDuration, EViewTargetBlendFunction::VTBlend_EaseInOut, 2.0f);
	}

	OnCinematicIntroComplete.Broadcast();
	UE_LOG(LogSpiderHero, Log, TEXT("Cinematic Intro complete, control handed over to gameplay camera."));
}
