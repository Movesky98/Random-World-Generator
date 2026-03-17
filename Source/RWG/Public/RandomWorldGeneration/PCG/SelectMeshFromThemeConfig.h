// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PCGSettings.h"

#include "SelectMeshFromThemeConfig.generated.h"

UENUM(BlueprintType)
enum class EStructureType : uint8
{
	Road UMETA(DisplayName = "Road"),
	Building UMETA(DisplayName = "Building"),
};

namespace SelectMeshFromThemeConfigPins
{
	static const FName InPinLabel = PCGPinConstants::DefaultInputLabel;
	static const FName OutPinLabel = PCGPinConstants::DefaultOutputLabel;
	static const FName ConfigPinLabel = FName(TEXT("Config"));
};

/**
* Add your tooltip here
*/
UCLASS(MinimalAPI, BlueprintType)
class USelectMeshFromThemeConfigSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override;
	virtual FText GetDefaultNodeTitle() const override;
	virtual FText GetNodeTooltipText() const override;
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif //WITH_EDITOR

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EStructureType StructureType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FName ThemeConfigAttribute = FName("ThemeConfig");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
	FName OutMeshAttribute = FName("Output");

/**
* Define various user facing properties here. 
* Tooltip can be added above UPROPERTY line.
* PCG_Overridable meta tag allows the properties to be overriden with other attributes in the graph editor.
*
* public:
*	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
*	bool CheckBox = true;
*/
};

class FSelectMeshFromThemeConfigElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	class UWorldThemeConfig* LoadWorldThemeConfig(const FPCGContext* Context, const USelectMeshFromThemeConfigSettings* Settings) const;

	void ExtractRoadMesh(int32 Seed, class UWorldThemeConfig* WorldConfig, class UPCGPointData* OutPointData, FName MeshAttribute) const;

	void ExtractBuildingMesh(int32 Seed, class UWorldThemeConfig* WorldConfig, class UPCGPointData* OutPointData, FName MeshAttribute) const;
};
