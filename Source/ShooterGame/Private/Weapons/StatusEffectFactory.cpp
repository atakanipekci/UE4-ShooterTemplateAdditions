// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Weapons/StatusEffectFactory.h"

UStatusEffect* StatusEffectFactory::CreateEffect(const FStatusEffectData& Data, AShooterCharacter* Owner,
	AShooterCharacter* Target)
{
	UStatusEffect* Product;
	switch(Data.Type)
	{
		case EStatusEffectType::Burn:
			Product = NewObject<UEffectBurn>();
			Product->Init(Data, Owner, Target);
			return Product;
			break;

		case EStatusEffectType::Speed:
			Product = NewObject<UEffectSpeed>();
			Product->Init(Data, Owner, Target);
			return Product;
			break;

		default:
			return nullptr;
			break;
	}
}
