// Copyright SpiderHero Team. All Rights Reserved.

#include "Audio/SpiderAudioComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "SpiderHero.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

USpiderAudioComponent::USpiderAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentWindVolume = 0.0f;
}

void USpiderAudioComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());

	if (WindLoopSound && CharacterOwner.IsValid())
	{
		WindAudioComponent = UGameplayStatics::SpawnSoundAttached(WindLoopSound, CharacterOwner->GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true);
		if (WindAudioComponent)
		{
			WindAudioComponent->SetVolumeMultiplier(0.0f);
		}
	}
}

void USpiderAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateWindWhoosh(DeltaTime);
}

void USpiderAudioComponent::UpdateWindWhoosh(float DeltaTime)
{
	if (!CharacterOwner.IsValid() || !WindAudioComponent)
	{
		return;
	}

	float Speed = CharacterOwner->GetVelocity().Size();
	float TargetVolume = FMath::Clamp((Speed - 700.0f) / 2000.0f, 0.0f, 1.0f);

	CurrentWindVolume = FMath::FInterpTo(CurrentWindVolume, TargetVolume, DeltaTime, 4.0f);
	WindAudioComponent->SetVolumeMultiplier(CurrentWindVolume);
	WindAudioComponent->SetPitchMultiplier(0.8f + (CurrentWindVolume * 0.4f));
}

void USpiderAudioComponent::PlayWebShootSound()
{
	if (WebShootSound && CharacterOwner.IsValid())
	{
		UGameplayStatics::PlaySoundAtLocation(this, WebShootSound, CharacterOwner->GetActorLocation(), 1.0f, FMath::RandRange(0.95f, 1.05f));
	}
}

void USpiderAudioComponent::PlayWebAttachSound(const FVector& Location)
{
	if (WebAttachSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WebAttachSound, Location, 1.0f, FMath::RandRange(0.9f, 1.1f));
	}
}

void USpiderAudioComponent::PlayWebReleaseSound(float BoostFactor)
{
	if (WebReleaseSound && CharacterOwner.IsValid())
	{
		float Volume = 1.0f + FMath::Clamp(BoostFactor * 0.3f, 0.0f, 0.5f);
		UGameplayStatics::PlaySoundAtLocation(this, WebReleaseSound, CharacterOwner->GetActorLocation(), Volume, 1.0f);
	}
}

void USpiderAudioComponent::PlayCombatHitSound(const FVector& Location, bool bIsHeavy)
{
	USoundBase* SoundToPlay = bIsHeavy ? HeavyHitSound.Get() : LightHitSound.Get();
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Location, 1.0f, FMath::RandRange(0.92f, 1.08f));
	}
}

void USpiderAudioComponent::PlaySpiderSenseWarning(int32 AlertLevel)
{
	if (SpiderSenseChime && CharacterOwner.IsValid())
	{
		float Pitch = 1.0f + (AlertLevel * 0.2f);
		UGameplayStatics::PlaySoundAtLocation(this, SpiderSenseChime, CharacterOwner->GetActorLocation(), 1.0f, Pitch);
	}
}

void USpiderAudioComponent::PlayPointLaunchSound()
{
	if (PointLaunchSound && CharacterOwner.IsValid())
	{
		UGameplayStatics::PlaySoundAtLocation(this, PointLaunchSound, CharacterOwner->GetActorLocation(), 1.2f, 1.0f);
	}
}
