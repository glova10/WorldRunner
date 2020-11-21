// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Weapons/ShooterWeapon_Melee.h"
#include "Kismet/KismetMathLibrary.h"


AShooterWeapon_Melee::AShooterWeapon_Melee(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bGivingDamage = false;
	PrevStartDmgDirection = FVector(0.f);
	PrevStartDmgLocation = FVector(0.f);
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AShooterWeapon_Melee::FireWeapon()
{
	//bGivingDamage = true;
	/*FVector ShootDir = GetAdjustedAim();
	FVector Origin = GetMuzzleLocation();

	// trace from camera to check what's under crosshair
	const float ProjectileAdjustRange = 10000.0f;
	const FVector StartTrace = GetCameraDamageStartLocation(ShootDir);
	const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;
	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	
	// and adjust directions to hit that actor
	if (Impact.bBlockingHit)
	{
		const FVector AdjustedDir = (Impact.ImpactPoint - Origin).GetSafeNormal();
		bool bWeaponPenetration = false;

		const float DirectionDot = FVector::DotProduct(AdjustedDir, ShootDir);
		if (DirectionDot < 0.0f)
		{
			// shooting backwards = weapon is penetrating
			bWeaponPenetration = true;
		}
		else if (DirectionDot < 0.5f)
		{
			// check for weapon penetration if angle difference is big enough
			// raycast along weapon mesh to check if there's blocking hit

			FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 150.0f;
			FVector MuzzleEndTrace = Origin;
			FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);

			if (MuzzleImpact.bBlockingHit)
			{
				bWeaponPenetration = true;
			}
		}

		if (bWeaponPenetration)
		{
			// spawn at crosshair position
			Origin = Impact.ImpactPoint - ShootDir * 10.0f;
		}
		else
		{
			// adjust direction to hit
			ShootDir = AdjustedDir;
		}
	}

	ServerFireProjectile(Origin, ShootDir);*/
}

bool AShooterWeapon_Melee::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void AShooterWeapon_Melee::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	/*FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AShooterProjectile* Projectile = Cast<AShooterProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetInstigator(GetInstigator());
		Projectile->SetOwner(this);
		Projectile->InitVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}*/
}

void AShooterWeapon_Melee::ApplyWeaponConfig(FMeleeWeaponData& Data)
{
	Data = MeleeConfig;
}

bool AShooterWeapon_Melee::RENDER_DEBUG = false;

void AShooterWeapon_Melee::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FVector curr_location = GetStartDamageLocation();
	FVector curr_direction = GetAdjustedAim();
	if (bGivingDamage)
	{

		FVector end_dmg = curr_location + curr_direction * MeleeConfig.WeaponRange / 2.f;
		FVector start_dmg = PrevStartDmgLocation + curr_direction * MeleeConfig.WeaponRange / 2.f;
		// warning this is a bit innacurate since the direction can change so we will take median
		FVector mean_direction = (curr_direction + PrevStartDmgDirection).GetSafeNormal();
		FRotator rotation = mean_direction.Rotation();
		FVector box_half_size = FVector(MeleeConfig.WeaponRange, MeleeConfig.WeaponWidth, MeleeConfig.WeaponWidth) / 2.f;

		TArray<FHitResult> hit_result;
		UKismetSystemLibrary::BoxTraceMulti(GetWorld(), start_dmg, end_dmg, box_half_size, rotation, UEngineTypes::ConvertToTraceType(ECC_WorldDynamic),
			false, ActorsToIgnore, RENDER_DEBUG ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hit_result, true, FLinearColor::White, FLinearColor::Red, 2.f);

		for (FHitResult hit : hit_result)
		{
			if (ShouldDealDamage(hit.GetActor()))
			{
				DealDamage(hit, curr_direction.GetSafeNormal2D());
				ActorsToIgnore.Push(hit.GetActor());
			}
		}
	}
	else if (ActorsToIgnore.Num() > 1)
	{
		ActorsToIgnore.RemoveAt(1, ActorsToIgnore.Num() - 1);
	}
	else if (ActorsToIgnore.Num() == 0)
	{
		ActorsToIgnore.Push(GetPawnOwner());
	}
	
	if (RENDER_DEBUG)
	{
		if (bGivingDamage)
		{
			FVector offset = GetAdjustedAim() * 25.f;
			DrawDebugPoint(GetWorld(), PrevStartDmgLocation + offset, 5.f, FColor::Red);
			DrawDebugPoint(GetWorld(), curr_location + offset, 3.f, FColor::Blue);
		}
	}	

	PrevStartDmgDirection = curr_direction;
	PrevStartDmgLocation = curr_location;
	
}

FVector AShooterWeapon_Melee::GetStartDamageLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	FVector dmg_point_location = UseMesh->GetSocketLocation(StartDmgPointName);
	
	FVector ShootDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(ShootDir);
	FVector right = GetPawnOwner()->GetActorRightVector();
	FVector best = UKismetMathLibrary::FindClosestPointOnLine(dmg_point_location, StartTrace, right);

	if (RENDER_DEBUG)
	{
		FVector start = StartTrace + right;
		FVector end = StartTrace - right;
		DrawDebugLine(GetWorld(), start, end, FColor::Green);
	}
	
	return best;
}

FVector AShooterWeapon_Melee::GetStartDamageDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(StartDmgPointName).Vector();
}

void AShooterWeapon_Melee::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = MeleeConfig.DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = MeleeConfig.HitDamage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
}

bool AShooterWeapon_Melee::ShouldDealDamage(AActor* TestActor) const
{
	// if we're an actor on the server, or the actor's role is authoritative, we should register damage
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->GetLocalRole() == ROLE_Authority ||
			TestActor->GetTearOff())
		{
			if (ActorsToIgnore.Find(TestActor) == INDEX_NONE)
				return true;
		}
	}

	return false;
}
