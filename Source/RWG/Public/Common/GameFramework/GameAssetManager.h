// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "GameAssetManager.generated.h"

/**
 * 데이터 에셋을 엔진 초기화 시점에 로드하고, 로드된 에셋을 조회하는 AssetManager.
 * 에디터 전용이 아닌 PrimaryAssetType을 전부 로드한다.
 */
UCLASS()
class RWG_API UGameAssetManager : public UAssetManager
{
	GENERATED_BODY()

/*********************************************************************
*                             LifeCycle
*********************************************************************/
protected:
	virtual void StartInitialLoading() override;

public:
	static UGameAssetManager& Get();

/*********************************************************************
*                             초기 로드
*********************************************************************/
private:
	void LoadInitialAssets();

	void OnInitialAssetsLoaded();

/*********************************************************************
*                             에셋 조회
*********************************************************************/
public:
	// 해당 타입의 첫 번째 에셋
	UObject* FindLoadedAsset(FPrimaryAssetType AssetType) const;

	// 해당 타입의 에셋 전부
	void GetLoadedAssets(FPrimaryAssetType AssetType, TArray<UObject*>& OutAssets) const;

	template<typename T>
	T* FindLoadedAsset(FPrimaryAssetType AssetType) const { return Cast<T>(FindLoadedAsset(AssetType)); }
};
