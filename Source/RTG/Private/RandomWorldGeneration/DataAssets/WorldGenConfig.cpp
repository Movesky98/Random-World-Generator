// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"

FPrimaryAssetId UWorldGenConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("WorldGenConfig", GetFName());
}
