// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/PCG/SelectMeshFromThemeConfig.h"
#include "RandomWorldGeneration/Grid/CityGridTypes.h"
#include "RandomWorldGeneration/PCG/RoadGraph.h"
#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"
#include "RandomWorldGeneration/Actors/WorldGenerator.h"

#include "Data/PCGPointData.h"
#include "Metadata/PCGMetadata.h"
#include "PCGParamData.h"
#include "PCGContext.h"

#define LOCTEXT_NAMESPACE "SelectMeshFromThemeConfigElement"

namespace LotAttribute
{
	static const FName Width(TEXT("Width"));
	static const FName Height(TEXT("Height"));
};

#if WITH_EDITOR
// The label the node is known as internally.
FName USelectMeshFromThemeConfigSettings::GetDefaultNodeName() const
{
	return FName(TEXT("SelectMeshFromThemeConfig"));
}

// Default node name shown in the graph editor. Include spaces.
FText USelectMeshFromThemeConfigSettings::GetDefaultNodeTitle() const
{
	return LOCTEXT("NodeTitle", "SelectMeshFromThemeConfig");
}

// Default tooltip for the node
FText USelectMeshFromThemeConfigSettings::GetNodeTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Add tooltip here.");
}
#endif //WITH_EDITOR

// Input/Output pin setup with specific properties, including:
// Pin data type, allowing singular or multiple inputs per pin, and creating multiple in/out pins.
TArray<FPCGPinProperties> USelectMeshFromThemeConfigSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(SelectMeshFromThemeConfigPins::InPinLabel, EPCGDataType::Point);
	
	FPCGPinProperties ConfigPin;
	ConfigPin.Label = SelectMeshFromThemeConfigPins::ConfigPinLabel;
	ConfigPin.AllowedTypes = EPCGDataType::Param;
	Pins.Add(ConfigPin);

	return Pins;
}

TArray<FPCGPinProperties> USelectMeshFromThemeConfigSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(SelectMeshFromThemeConfigPins::OutPinLabel, EPCGDataType::Point);

	return Pins;
}

// Creates the Element to be used for ExecuteInternal.
FPCGElementPtr USelectMeshFromThemeConfigSettings::CreateElement() const
{
	return MakeShared<FSelectMeshFromThemeConfigElement>();
}

/*
* Processing function for this node. 
* Context holds the InputData, containing the input data collection for this node 
* and the OutputData, the output data collection to write to as output.
* Returns true if the processing is done. 
* Returning false will call back this function at next tick, and will call it until it returns true.
* Settings contains all the setup options for this node, and if a property was marked PCG_Overridable, 
* "Context->GetInputSettings" will contain the overridden value for this property if it is overridden
*/ 
bool FSelectMeshFromThemeConfigElement::ExecuteInternal(FPCGContext* Context) const
{
	const USelectMeshFromThemeConfigSettings* Settings = Context ? 
		Context->GetInputSettings<USelectMeshFromThemeConfigSettings>() : nullptr;

	if (!Settings) return true; 

	UWorldThemeConfig* Config = LoadWorldThemeConfig(Context, Settings);
	if (!Config) return true;

	AWorldGenerator* WorldGenerator = nullptr;
	if (Context && Context->SourceComponent.IsValid())
	{
		if (AActor* Owner = Context->SourceComponent->GetOwner())
		{
			WorldGenerator = Cast<AWorldGenerator>(Owner);
		}
	}

	if (!WorldGenerator) return true;

	int32 Seed = WorldGenerator->GetSeed();

	const TArray<FPCGTaggedData>& Inputs = Context->InputData.GetInputsByPin(SelectMeshFromThemeConfigPins::InPinLabel);
	if (Inputs.Num() <= 0) return true;

	const UPCGPointData* InPointData = Cast<UPCGPointData>(Inputs[0].Data);
	if (!InPointData) return true;

	// 입력으로 들어온 Point + StaticMesh 경로 → 출력 Point로 내보낼 것임.
	UPCGPointData* OutPointData = Cast<UPCGPointData>(InPointData->DuplicateData(Context, true));

	switch (Settings->StructureType)
	{
	case EStructureType::Road:
		ExtractRoadMesh(Seed, Config, OutPointData, Settings->OutMeshAttribute);
		break;
	case EStructureType::Building:
		ExtractBuildingMesh(Seed, Config, OutPointData, Settings->OutMeshAttribute);
		break;
	default:
		break;
	}

	FPCGTaggedData& Out = Context->OutputData.TaggedData.Emplace_GetRef();
	Out.Pin = SelectMeshFromThemeConfigPins::OutPinLabel;
	Out.Data = OutPointData;

	return true;
}

UWorldThemeConfig* FSelectMeshFromThemeConfigElement::LoadWorldThemeConfig(const FPCGContext* Context, const USelectMeshFromThemeConfigSettings* Settings) const
{
	const TArray<FPCGTaggedData>& InConfigData = Context->InputData.GetInputsByPin(SelectMeshFromThemeConfigPins::ConfigPinLabel);
	if (InConfigData.IsEmpty())
	{
		return nullptr;
	}

	const UPCGParamData* ConfigParam = Cast<UPCGParamData>(InConfigData[0].Data);
	if (!ConfigParam || !ConfigParam->Metadata)
	{
		return nullptr;
	}

	const FPCGMetadataAttributeBase* AttributeBase = ConfigParam->Metadata->GetConstAttribute(Settings->ThemeConfigAttribute);
	if (!AttributeBase)
	{
		return nullptr;
	}

	const auto* ThemePathAttribute = static_cast<const FPCGMetadataAttribute<FSoftObjectPath>*>(AttributeBase);
	const FSoftObjectPath ThemeConfigPath = ThemePathAttribute->GetValueFromItemKey(0);
	if (!ThemeConfigPath.IsValid())
	{
		return nullptr;
	}

	UObject* LoadedObject = ThemeConfigPath.TryLoad();
	UWorldThemeConfig* ThemeConfig = Cast<UWorldThemeConfig>(LoadedObject);
	if (!ThemeConfig)
	{
		return nullptr;
	}

	return ThemeConfig;
}

void FSelectMeshFromThemeConfigElement::ExtractRoadMesh(int32 Seed, class UWorldThemeConfig* WorldConfig, UPCGPointData* OutPointData, FName MeshAttribute) const
{
	TArray<FPCGPoint>& OutPoints = OutPointData->GetMutablePoints();
	UPCGMetadata* Metadata = OutPointData->MutableMetadata();

	check(Metadata);

	FRandomStream ContentStream(Seed);
	auto* MeshAttr = Metadata->CreateAttribute<FString>(MeshAttribute, FString(), false, true);

	int32 MaxIndex = WorldConfig->Roads.Num() - 1;

	for (FPCGPoint& Point : OutPoints)
	{
		const TSoftObjectPtr<UStaticMesh>& RoadMesh = WorldConfig->Roads[ContentStream.RandRange(0, MaxIndex)];
		const FSoftObjectPath Path = RoadMesh.ToSoftObjectPath();

		const PCGMetadataEntryKey Key = Point.MetadataEntry;
		MeshAttr->SetValue(Key, Path.ToString());
	}
}

void FSelectMeshFromThemeConfigElement::ExtractBuildingMesh(int32 Seed, class UWorldThemeConfig* WorldConfig, UPCGPointData* OutPointData, FName MeshAttribute) const
{
	TArray<FPCGPoint>& OutPoints = OutPointData->GetMutablePoints();
	UPCGMetadata* Metadata = OutPointData->MutableMetadata();

	check(Metadata);

	auto* MeshAttr = Metadata->CreateAttribute<FString>(MeshAttribute, FString(), false, true);

	const FPCGMetadataAttribute<int32>* WidthAttr = Metadata->GetConstTypedAttribute<int32>(LotAttribute::Width);
	const FPCGMetadataAttribute<int32>* HeightAttr = Metadata->GetConstTypedAttribute<int32>(LotAttribute::Height);

	// TODO : 사이즈에 맞춰서 메시를 연결해줘야 함.
	for (FPCGPoint& Point : OutPoints)
	{
		const int32 Width = WidthAttr->GetValue(Point.MetadataEntry);
		const int32 Height = HeightAttr->GetValue(Point.MetadataEntry);

		TArray<TSoftObjectPtr<UStaticMesh>> MeshPaths;
		
		FRandomStream ContentStream(Seed);

		// 조건에 만족하는 Mesh들을 저장
		for (auto& BuildingEntry : WorldConfig->BuildingEntries)
		{
			if (BuildingEntry.FootprintSizeX <= Width && BuildingEntry.FootprintSizeY <= Height)
			{
				MeshPaths.Add(BuildingEntry.Mesh);
			}
		}

		const int32 Index = ContentStream.RandRange(0, MeshPaths.Num() - 1);

		FBuildingAssetEntry* Found = WorldConfig->BuildingEntries.FindByPredicate(
			[&](const FBuildingAssetEntry& Entry)
			{
				return Entry.Mesh == MeshPaths[Index];
			});

		if (Found->FootprintSizeX == 1 && Found->FootprintSizeY == 1)
		{
			const float ScaleSize = WorldConfig->CellSize / 100.0f;

			Point.Transform.SetScale3D(FVector(Width * ScaleSize, Height * ScaleSize, Width * Height / ScaleSize));
		}
	
		const PCGMetadataEntryKey Key = Point.MetadataEntry;
		MeshAttr->SetValue(Key, MeshPaths[Index].ToSoftObjectPath().ToString());
	}
}

#undef LOCTEXT_NAMESPACE