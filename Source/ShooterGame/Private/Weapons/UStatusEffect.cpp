// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Weapons/UStatusEffect.h"

#include "LOGHelper.h"

void UStatusEffect::Init(const FStatusEffectData& EffectData, AShooterCharacter* OwnerChar, AShooterCharacter* TargetChar)
{
	Data = EffectData;
	Owner = OwnerChar;
	Target = TargetChar;
}

void UEffectBurn::Start()
{
	if(Target)
	{
		Target->TakeDamage(Data.Value, FDamageEvent(UDamageType::StaticClass()), Owner->GetController(), Owner);
	}
}

void UEffectBurn::Update()
{
	if(Target)
	{
		Target->TakeDamage(Data.Value, FDamageEvent(UDamageType::StaticClass()), Owner->GetController(), Owner);
	}
	Data.LifeTime--;
}

void UEffectBurn::End()
{
	//Do Nothing
}

void UEffectSpeed::Start()
{
	if(Target && Target->GetMovementComponent())
	{
		Target->AddSpeedModifier(Data.Value);
	}
}

void UEffectSpeed::Update()
{
	Data.LifeTime--;
}

void UEffectSpeed::End()
{
	if(Target && Target->GetMovementComponent())
	{
		Target->AddSpeedModifier(-Data.Value);
	}
}
