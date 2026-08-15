// Copyright SpiderHero Team. All Rights Reserved.

#include "Web/SpiderWebShootingComponent.h"
#include "Character/SpiderHeroCharacter.h"
#include "Web/SpiderWebProjectile.h"
#include "SpiderHero.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

USpiderWebShootingComponent::USpiderWebShootingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	WebProjectileClass = ASpiderWebProjectile::StaticClass();
	MaxWebShots = 8;
	CurrentWebShots = MaxWebShots;
	WebReloadRate = 1.25f; // Reload 1 shot every 1.25s
	FireRateLimit = 0.15f;

	LeftWristSocket = TEXT("Socket_Wrist_L");
	RightWristSocket = TEXT("Socket_Wrist_R");

	WebYankForce = 2200.0f;
	WebYankUpwardLift = 600.0f;

	bNextShotIsLeftHand = false;
	LastFireTime = -10.0f;
}

void USpiderWebShootingComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ASpiderHeroCharacter>(GetOwner());
	CurrentWebShots = MaxWebShots;
}

void USpiderWebShootingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Automatic web fluid reload over time
	static float ReloadAccumulator = 0.0f;
	if (CurrentWebShots < MaxWebShots)
	{
		ReloadAccumulator += DeltaTime;
		if (ReloadAccumulator >= WebReloadRate)
		{
			ReloadAccumulator = 0.0f;
			ReplenishWebShots(1);
		}
	}
	else
	{
		ReloadAccumulator = 0.0f;
	}
}

FVector USpiderWebShootingComponent::GetMuzzleLocation(bool bLeftHand) const
{
	if (CharacterOwner.IsValid())
	{
		USkeletalMeshComponent* Mesh = CharacterOwner->GetMesh();
		FName Socket = bLeftHand ? LeftWristSocket : RightWristSocket;
		if (Mesh && Mesh->DoesSocketExist(Socket))
		{
			return Mesh->GetSocketLocation(Socket);
		}
		// Fallback to chest forward offset
		FVector Forward = CharacterOwner->GetActorForwardVector();
		FVector Right = CharacterOwner->GetActorRightVector();
		return CharacterOwner->GetActorLocation() + (Forward * 40.0f) + (Right * (bLeftHand ? -25.0f : 25.0f)) + FVector(0.0f, 0.0f, 20.0f);
	}
	return FVector::ZeroVector;
}

FVector USpiderWebShootingComponent::GetAimDirection() const
{
	if (CharacterOwner.IsValid())
	{
		AController* Controller = CharacterOwner->GetController();
		if (Controller)
		{
			return Controller->GetControlRotation().Vector();
		}
		return CharacterOwner->GetActorForwardVector();
	}
	return FVector::ForwardVector;
}

bool USpiderWebShootingComponent::FireWebShot()
{
	if (!CharacterOwner.IsValid() || CurrentWebShots <= 0)
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < FireRateLimit)
	{
		return false;
	}

	LastFireTime = CurrentTime;

	FVector MuzzleLoc = GetMuzzleLocation(bNextShotIsLeftHand);
	FVector AimDir = GetAimDirection();
	FRotator SpawnRot = AimDir.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CharacterOwner.Get();
	SpawnParams.Instigator = CharacterOwner.Get();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* ClassToSpawn = WebProjectileClass ? WebProjectileClass.Get() : ASpiderWebProjectile::StaticClass();
	ASpiderWebProjectile* Projectile = GetWorld()->SpawnActor<ASpiderWebProjectile>(ClassToSpawn, MuzzleLoc, SpawnRot, SpawnParams);

	if (Projectile)
	{
		CurrentWebShots--;
		bool bFiredLeft = bNextShotIsLeftHand;
		bNextShotIsLeftHand = !bNextShotIsLeftHand;

		OnWebFired.Broadcast(MuzzleLoc, AimDir, bFiredLeft);
		OnWebAmmoChanged.Broadcast(CurrentWebShots, MaxWebShots);
		UE_LOG(LogSpiderCombat, Log, TEXT("Fired Web Shot (Ammo: %d / %d) from %s"), CurrentWebShots, MaxWebShots, bFiredLeft ? TEXT("LEFT") : TEXT("RIGHT"));
		return true;
	}

	return false;
}

bool USpiderWebShootingComponent::PerformWebYank(AActor* TargetActor)
{
	if (!TargetActor || !CharacterOwner.IsValid())
	{
		return false;
	}

	FVector PlayerLoc = CharacterOwner->GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();
	FVector PullDir = (PlayerLoc - TargetLoc).GetSafeNormal();

	FVector YankImpulse = (PullDir * WebYankForce) + (FVector::UpVector * WebYankUpwardLift);

	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	if (PrimComp && PrimComp->IsSimulatingPhysics())
	{
		PrimComp->AddImpulse(YankImpulse, NAME_None, true);
	}
	else if (ACharacter* TargetChar = Cast<ACharacter>(TargetActor))
	{
		TargetChar->LaunchCharacter(YankImpulse, true, true);
	}

	OnWebYankExecuted.Broadcast(TargetActor, PullDir);
	UE_LOG(LogSpiderCombat, Log, TEXT("Web Yank executed on %s"), *TargetActor->GetName());

	return true;
}

bool USpiderWebShootingComponent::FireWebTether(AActor* TargetActor, const FVector& AnchorLocation)
{
	if (!TargetActor)
	{
		return false;
	}

	FVector TargetLoc = TargetActor->GetActorLocation();
	FVector TetherDir = (AnchorLocation - TargetLoc).GetSafeNormal();
	FVector TetherImpulse = TetherDir * (WebYankForce * 1.25f);

	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	if (PrimComp && PrimComp->IsSimulatingPhysics())
	{
		PrimComp->AddImpulse(TetherImpulse, NAME_None, true);
	}
	else if (ACharacter* TargetChar = Cast<ACharacter>(TargetActor))
	{
		TargetChar->LaunchCharacter(TetherImpulse, true, true);
	}

	UE_LOG(LogSpiderCombat, Log, TEXT("Web Tether attached from %s to %s"), *TargetActor->GetName(), *AnchorLocation.ToString());
	return true;
}

void USpiderWebShootingComponent::ReplenishWebShots(int32 Amount)
{
	int32 OldShots = CurrentWebShots;
	CurrentWebShots = FMath::Clamp(CurrentWebShots + Amount, 0, MaxWebShots);
	if (CurrentWebShots != OldShots)
	{
		OnWebAmmoChanged.Broadcast(CurrentWebShots, MaxWebShots);
	}
}
