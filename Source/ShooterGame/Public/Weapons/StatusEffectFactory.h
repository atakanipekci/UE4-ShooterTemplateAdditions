// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UStatusEffect.h"

/** Creates effects in static function with given data
 * 
 */
class SHOOTERGAME_API StatusEffectFactory
{
public:
 /** Return a new and initialized effect with given data */
 static UStatusEffect* CreateEffect(const FStatusEffectData&, class AShooterCharacter* Owner, class AShooterCharacter* Target);
};
