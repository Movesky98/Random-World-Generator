// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/DataAssets/BaseInputConfig.h"

#include "InputMappingContext.h"
#include "InputAction.h"

FPrimaryAssetId UBaseInputConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("InputConfig", GetFName());
}
