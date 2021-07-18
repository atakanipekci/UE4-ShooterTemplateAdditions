// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UStatusEffect.h"

/**
 * 
 */
class SHOOTERGAME_API StatusEffectFactory
{
public:
 static UStatusEffect* CreateEffect(const FStatusEffectData&, class AShooterCharacter* Owner, class AShooterCharacter* Target);
};
