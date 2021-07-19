// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UStatusEffect.generated.h"
/** Type of effect that is used primarly for the effect factory to determine which
 *effect to create, add another enum here for new ones*/
UENUM()
enum class EStatusEffectType: uint8
{
	Burn,
	Speed
};
/** Shared data structure for each effect. */
USTRUCT(BlueprintType)
struct FStatusEffectData
{
	GENERATED_BODY()
	/** Type of effect E.g Burn, Speed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStatusEffectType Type;

	/** Data that determines the modification value
	 * for speed effect this is used as a speed modifier
	 * for burn effect this represents damage over time*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	/** Life time of this effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeTime;
};

/** Status Effect is a temporary modification to the stats of a player
 *They can modify player stats when they are first applied, when they update
 *every second or when their life time ends.
 */
UCLASS(Abstract)
class SHOOTERGAME_API UStatusEffect : public UObject
{
	GENERATED_BODY()
	public:
	virtual ~UStatusEffect() = default;

	/** Initialize data for this UObject */
	virtual void Init(const FStatusEffectData&, AShooterCharacter* Owner, AShooterCharacter* Target);
	
	/** Apply modification to target when this effect is created */
	virtual void Start() PURE_VIRTUAL(UStatusEffect::Start);
	
	/** Apply modification to target in each call*/
	virtual void Update() PURE_VIRTUAL(UStatusEffect::Start);
	
	/** Apply modification to target when life time ends */
	virtual void End() PURE_VIRTUAL(UStatusEffect::Start);

	const FStatusEffectData& GetEffectData() const
	{
		return Data;
	}
	/** Timer handle for update in each second*/
	FTimerHandle EffectTimerHandle;

protected:
	FStatusEffectData Data;
	UPROPERTY()
	AShooterCharacter* Owner = nullptr;
	UPROPERTY()
	AShooterCharacter* Target = nullptr;
};

/** Applies an initial damage and damage over time status
 **/
UCLASS()
class SHOOTERGAME_API UEffectBurn : public UStatusEffect
{
	GENERATED_BODY()
	public:
	
	virtual void Start() override;
	virtual void Update() override;
	virtual void End() override;
};

/** Modifies the max speed variable of the target player for given time
 **/
UCLASS()
class SHOOTERGAME_API UEffectSpeed : public UStatusEffect
{
	GENERATED_BODY()
	public:
	
	virtual void Start() override;
	virtual void Update() override;
	virtual void End() override;
};
