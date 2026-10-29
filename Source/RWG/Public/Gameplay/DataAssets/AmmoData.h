// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DataAssets/ItemData.h"
#include "AmmoData.generated.h"

class UStaticMesh;
class UMaterialInterface;

/**
 *
 */
UCLASS()
class RWG_API UAmmoData : public UItemData
{
	GENERATED_BODY()

public:
	// 월드에 놓였을 때 보이는 메시
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	TObjectPtr<UStaticMesh> Mesh;

	// 메시에 덮어쓸 머티리얼 (비우면 메시 기본 머티리얼)
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	TObjectPtr<UMaterialInterface> MeshMaterial;
};
