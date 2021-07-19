// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Weapons/ShooterWeapon_Projectile.h"
#include "Weapons/ShooterProjectile.h"

AShooterWeapon_Projectile::AShooterWeapon_Projectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AShooterWeapon_Projectile::FireWeapon()
{
	FVector ShootDir = GetAdjustedAim();
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

	ServerFireProjectile(Origin, ShootDir);
}

void AShooterWeapon_Projectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	/** If there is no dynamic object on the map that can collide with the projectile, you can optimize this to
	 only update when player moves themselves or moves their aim*/
	if(GetPawnOwner() && !GetPawnOwner()->IsTargeting() && !GetPawnOwner()->IsRunning())
	{
		DrawTrajectory();
	}
	else
	{
		ClearTrajectory();
	}
}

void AShooterWeapon_Projectile::OnEquipFinished()
{
	Super::OnEquipFinished();
	// Create the actor only on client side
	if(GetPawnOwner() && GetPawnOwner()->IsLocallyControlled() && TrajectorySplineActor == nullptr)
	{
		TrajectorySplineActor = static_cast<ASplineActor*>(GetWorld()->SpawnActor<ASplineActor>(FVector::ZeroVector,FRotator::ZeroRotator,FActorSpawnParameters{}));
	}

	if(TrajectorySplineActor)
	{
		//This line is only here because I did not have any other free assets that I can use as a trajectory mesh
		//Default Unreal meshes are way too big for trajectory so I scale it down
		TrajectorySplineActor->SetActorScale3D(FVector{0.1,0.1,0.1});
		TrajectorySplineActor->ClearNodes();
		TrajectorySplineActor->SplineMeshMap = TrajectorySplineMap;
		TrajectorySplineActor->UpdateSpline();
		TrajectorySplineActor->SetReplicates(false);
	}
}

void AShooterWeapon_Projectile::OnUnEquip()
{
	Super::OnUnEquip();
	ClearTrajectory();
}

void AShooterWeapon_Projectile::DrawTrajectory()
{
	//If gravity is 0 there is no need for a trajectory
	if(TrajectorySplineActor && ProjectileConfig.ProjectileGravityScale != 0)
	{
		FPredictProjectilePathResult ProjectileResult;
		FPredictProjectilePathParams ProjectileParams;
		
		FVector ShootDir = GetAdjustedAim();
		FVector Origin = GetMuzzleLocation();

		ProjectileParams.StartLocation = Origin;
		ProjectileParams.LaunchVelocity = ShootDir * ProjectileConfig.ProjectileInitialSpeed;
		ProjectileParams.TraceChannel = COLLISION_PROJECTILE;
		ProjectileParams.ProjectileRadius = 5;
		ProjectileParams.bTraceWithCollision = true;
		ProjectileParams.bTraceWithChannel = true;
		ProjectileParams.MaxSimTime = ProjectileConfig.ProjectileLife;
		ProjectileParams.OverrideGravityZ = GetWorld()->GetGravityZ()*ProjectileConfig.ProjectileGravityScale;

		UGameplayStatics::PredictProjectilePath(GetWorld(), ProjectileParams, ProjectileResult);

		//ProjectileParams.LaunchVelocity = ProjectileConfig.ProjectileInitialSpeed;
		TrajectorySplineActor->SetActorHiddenInGame(true);
		TrajectorySplineActor->ClearNodes();
		
		for(int i = 0; i < ProjectileResult.PathData.Num(); i++)
		{
			TrajectorySplineActor->AddNode(ProjectileResult.PathData[i].Location);
		}
		TrajectorySplineActor->UpdateSpline();
		TrajectorySplineActor->SetActorHiddenInGame(false);
	}
}

void AShooterWeapon_Projectile::ClearTrajectory()
{
	if(TrajectorySplineActor == nullptr)
	{
		return;
	}
	TrajectorySplineActor->ClearNodes();
	TrajectorySplineActor->UpdateSpline();
}

bool AShooterWeapon_Projectile::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void AShooterWeapon_Projectile::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AShooterProjectile* Projectile = Cast<AShooterProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetInstigator(GetInstigator());
		Projectile->SetOwner(this);
		Projectile->InitVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

void AShooterWeapon_Projectile::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}
