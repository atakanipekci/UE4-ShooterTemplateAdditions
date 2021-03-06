// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Weapons/ShooterProjectile.h"

#include "LOGHelper.h"
#include "StatusEffectFactory.h"
#include "Particles/ParticleSystemComponent.h"
#include "Effects/ShooterExplosionEffect.h"

AShooterProjectile::AShooterProjectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	ParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleComp"));
	ParticleComp->bAutoActivate = false;
	ParticleComp->bAutoDestroy = false;
	ParticleComp->SetupAttachment(RootComponent);

	MovementComp = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->MaxSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicatingMovement(true);
}

void AShooterProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MovementComp->OnProjectileStop.AddDynamic(this, &AShooterProjectile::OnStopOnImpact);
	CollisionComp->MoveIgnoreActors.Add(GetInstigator());

	AShooterWeapon_Projectile* OwnerWeapon = Cast<AShooterWeapon_Projectile>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}
	MovementComp->ProjectileGravityScale = WeaponConfig.ProjectileGravityScale;
	MovementComp->InitialSpeed = WeaponConfig.ProjectileInitialSpeed;
	//SetLifeSpan( WeaponConfig.ProjectileLife );
	MyController = GetInstigatorController();
}

void AShooterProjectile::BeginPlay()
{
	Super::BeginPlay();
	LifeTimeTimer = WeaponConfig.ProjectileLife;
}

void AShooterProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CountDownLife(DeltaSeconds);
}

void AShooterProjectile::InitVelocity(FVector& ShootDirection)
{
	if (MovementComp)
	{
		MovementComp->Velocity = ShootDirection * MovementComp->InitialSpeed;
	}
}

void AShooterProjectile::OnStopOnImpact(const FHitResult& HitResult)
{
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		Explode(HitResult);
		DisableAndDestroy();
	}
}

inline void AShooterProjectile::CountDownLife(float DeltaSeconds)
{
	LifeTimeTimer -= DeltaSeconds;
	if(LifeTimeTimer <= 0 && GetLocalRole() == ROLE_Authority && !bExploded)
	{
		Explode(GetActorLocation());
		DisableAndDestroy();
	}
}

AShooterExplosionEffect* const AShooterProjectile::Explosion(const FVector& ExplosionPoint, const FRotator& Rotation)
{
	bExploded = true;
	
	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, WeaponConfig.ExplosionDamage, ExplosionPoint, WeaponConfig.ExplosionRadius, WeaponConfig.DamageType, TArray<AActor*>(), this, MyController.Get());
	}

	/* Get the overlapping actors in a sphere */
	TArray<FOverlapResult> Overlaps;
	/* Make sure to hit the same character only once */
	TSet<AShooterCharacter*> HitShooterCharacters;
	if (GetWorld() && MyController.IsValid() && MyController->GetCharacter())
	{
		GetWorld()->OverlapMultiByObjectType(Overlaps, ExplosionPoint, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(WeaponConfig.ExplosionRadius));
		AShooterCharacter* OwnerCharacter = Cast<AShooterCharacter>(MyController->GetCharacter());
		for (auto OverlapResult : Overlaps)
		{
			if(OverlapResult.GetActor())
			{
				AShooterCharacter* TargetCharacter = Cast<AShooterCharacter>(OverlapResult.GetActor());
				if(TargetCharacter &&  !HitShooterCharacters.Contains(TargetCharacter))
				{
					HitShooterCharacters.Add(TargetCharacter);
					for (auto Effect : StatusEffects)
					{
						TargetCharacter->ApplyStatusEffect(StatusEffectFactory::CreateEffect(Effect, OwnerCharacter, TargetCharacter));
					}
				}
			}
		}
	}

	if (ExplosionTemplate)
	{
		FTransform const SpawnTransform(Rotation, ExplosionPoint);
		AShooterExplosionEffect* const EffectActor = GetWorld()->SpawnActorDeferred<AShooterExplosionEffect>(ExplosionTemplate, SpawnTransform);
		return EffectActor;
	}
	return nullptr;
}

void AShooterProjectile::Explode(const FHitResult& Impact)
{
	// effects and damage origin shouldn't be placed inside mesh at impact point
	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
	AShooterExplosionEffect* const EffectActor = Explosion(NudgedImpactLocation, Impact.ImpactNormal.Rotation());
	if(EffectActor)
	{
		EffectActor->SurfaceHit = Impact;
		FTransform const SpawnTransform(FRotator{}, NudgedImpactLocation);
		UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
	}
}

void AShooterProjectile::Explode(const FVector& ExplosionPoint)
{
	AShooterExplosionEffect* const EffectActor = Explosion(ExplosionPoint, FRotator{});
	if(EffectActor)
	{
		FTransform const SpawnTransform(FRotator{}, ExplosionPoint);
		UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
	}
}

void AShooterProjectile::DisableAndDestroy()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	MovementComp->StopMovementImmediately();

	// give clients some time to show explosion
	SetLifeSpan( 2.0f );
}

///CODE_SNIPPET_START: AActor::GetActorLocation AActor::GetActorRotation
void AShooterProjectile::OnRep_Exploded()
{
	FVector ProjDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;
	
	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(SCENE_QUERY_STAT(ProjClient), true, GetInstigator())))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	Explode(Impact);
}
///CODE_SNIPPET_END

void AShooterProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void AShooterProjectile::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	
	DOREPLIFETIME( AShooterProjectile, bExploded );
}