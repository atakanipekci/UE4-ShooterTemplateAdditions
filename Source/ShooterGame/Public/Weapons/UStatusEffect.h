// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UStatusEffect.generated.h"

UENUM()
enum class EStatusEffectType: uint8
{
	Burn,
	Speed
};

USTRUCT(BlueprintType)
struct FStatusEffectData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStatusEffectType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeTime;
};

/**
 * 
 */
UCLASS(Abstract)
class SHOOTERGAME_API UStatusEffect : public UObject
{
	GENERATED_BODY()
	public:
	//StatusEffect(const FStatusEffectData&, AShooterCharacter* Owner, AShooterCharacter* Target);
	//virtual ~UStatusEffect() = default;

	virtual void Init(const FStatusEffectData&, AShooterCharacter* Owner, AShooterCharacter* Target);
	virtual void Start() PURE_VIRTUAL(UStatusEffect::Start);
	virtual void Update() PURE_VIRTUAL(UStatusEffect::Start);
	virtual void End() PURE_VIRTUAL(UStatusEffect::Start);

	const FStatusEffectData& GetEffectData() const
	{
		return Data;
	}
	FTimerHandle EffectTimerHandle;

protected:
	FStatusEffectData Data;
	UPROPERTY()
	AShooterCharacter* Owner = nullptr;
	UPROPERTY()
	AShooterCharacter* Target = nullptr;
};
UCLASS()
class SHOOTERGAME_API UEffectBurn : public UStatusEffect
{
	GENERATED_BODY()
	public:
	
	virtual void Start() override;
	virtual void Update() override;
	virtual void End() override;
};
UCLASS()
class SHOOTERGAME_API UEffectSpeed : public UStatusEffect
{
	GENERATED_BODY()
	public:
	
	virtual void Start() override;
	virtual void Update() override;
	virtual void End() override;
};
