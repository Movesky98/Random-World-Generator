// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"


FPrimaryAssetId UWorldThemeConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("WorldThemeConfig", GetFName());
}
