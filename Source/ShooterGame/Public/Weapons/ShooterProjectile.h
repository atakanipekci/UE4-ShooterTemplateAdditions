// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ShooterWeapon_Projectile.h"
#include "ShooterProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

// 
UCLASS(Abstract, Blueprintable)
class AShooterProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** Begin Play Override */
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	/** setup velocity */
	void InitVelocity(FVector& ShootDirection);

	/** handle stop after hit */
	UFUNCTION()
	virtual void OnStopOnImpact(const FHitResult& HitResult);
protected:
	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UParticleSystemComponent* ParticleComp;

	/** The status effects this projectile will apply to targets, can be modified from BPs*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStatusEffectData> StatusEffects;

	/** Countdown To Explode and Destroy*/
	inline void CountDownLife(float DeltaSeconds);
private:
	/** Life Time of this Projectile */
	float LifeTimeTimer;

protected:

	/** effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<class AShooterExplosionEffect> ExplosionTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** projectile data */
	struct FProjectileWeaponData WeaponConfig;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** trigger an explosion on hit */
	void Explode(const FHitResult& Impact);
	
	/** Explode on a given position without the need to hit anything */
	void Explode(const FVector& ExplosionPoint);
	
	/** Actual Explosion method */
	virtual class AShooterExplosionEffect* const Explosion(const FVector& ExplosionPoint, const FRotator& Rotation);
	
	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }
};
