// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplay,				Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSession,					Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRandomWorldGen,	Log, All);

#define CL_FILENAME (FPaths::GetCleanFilename(__FILE__))

#define COMMON_LOG(Category, Verbosity, Format, ...) \
    UE_LOG(Category, Verbosity, TEXT("%s(%d) :: %s | " Format), \
        *CL_FILENAME, \
        __LINE__, \
        *FString(__FUNCTION__), \
        ##__VA_ARGS__)