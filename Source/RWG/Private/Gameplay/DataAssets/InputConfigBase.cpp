// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/DataAssets/InputConfigBase.h"

#include "InputMappingContext.h"
#include "InputAction.h"

FPrimaryAssetId UInputConfigBase::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("InputConfig", GetFName());
}
