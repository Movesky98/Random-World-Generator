// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/DataAssets/BulletImpactData.h"

const FBulletImpactEffect& UBulletImpactData::GetEffect(bool bHitCharacter, EPhysicalSurface SurfaceType) const
{
	if (bHitCharacter)
	{
		return CharacterEffect;
	}

	if (const FBulletImpactEffect* SurfaceEffect = SurfaceEffects.Find(SurfaceType))
	{
		return *SurfaceEffect;
	}

	return DefaultEffect;
}
