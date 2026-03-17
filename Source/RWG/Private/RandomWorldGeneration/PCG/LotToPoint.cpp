// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/PCG/LotToPoint.h"
#include "RandomWorldGeneration/Actors/WorldGenerator.h"
#include "RandomWorldGeneration/Grid/CityGridTypes.h"
#include "PCGContext.h"
#include "Metadata/PCGMetadata.h"
#include "Data/PCGPointData.h"

#define LOCTEXT_NAMESPACE "LotToPointElement"

#if WITH_EDITOR
// The label the node is known as internally.
FName ULotToPointSettings::GetDefaultNodeName() const
{
	return FName(TEXT("LotToPoint"));
}

// Default node name shown in the graph editor. Include spaces.
FText ULotToPointSettings::GetDefaultNodeTitle() const
{
	return LOCTEXT("NodeTitle", "LotToPoint");
}

// Default tooltip for the node
FText ULotToPointSettings::GetNodeTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Add tooltip here.");
}
#endif //WITH_EDITOR

// Input/Output pin setup with specific properties, including:
// Pin data type, allowing singular or multiple inputs per pin, and creating multiple in/out pins.
TArray<FPCGPinProperties> ULotToPointSettings::InputPinProperties() const
{
	// 입력 없음
	return { };
}

TArray<FPCGPinProperties> ULotToPointSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(FName(TEXT("LotPoints")), EPCGDataType::Point);

	return Pins;
}

// Creates the Element to be used for ExecuteInternal.
FPCGElementPtr ULotToPointSettings::CreateElement() const
{
	return MakeShared<FLotToPointElement>();
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
bool FLotToPointElement::ExecuteInternal(FPCGContext* Context) const
{
	const ULotToPointSettings* Settings = Context ? Context->GetInputSettings<ULotToPointSettings>() : nullptr;
	if (!Settings) return true;

	AWorldGenerator* WorldGenerator = nullptr;
	if (Context && Context->SourceComponent.IsValid())
	{
		if (AActor* Owner = Context->SourceComponent->GetOwner())
		{
			WorldGenerator = Cast<AWorldGenerator>(Owner);
		}
	}

	if (!WorldGenerator) return true;

	const FCityGrid& Grid = WorldGenerator->GetCityGrid();

	UPCGPointData* OutData = NewObject<UPCGPointData>();
	OutData->InitializeFromData(nullptr);

	TArray<FPCGPoint>& OutPoints = OutData->GetMutablePoints();
	UPCGMetadata* Metadata = OutData->Metadata;

	auto* AttrCenterPosition = Metadata->CreateAttribute<FVector>(TEXT("CenterPosition"), FVector::ZeroVector, true, true);
	auto* AttrWidth = Metadata->CreateAttribute<int32>(TEXT("Width"), -1, false, true);
	auto* AttrHeight = Metadata->CreateAttribute<int32>(TEXT("Height"), -1, false, true);

	for (const FCityLot& Lot : Grid.GetLots())
	{
		FPCGPoint Point;

		Point.Transform = FTransform(FQuat::Identity, Lot.Center, FVector(1.0f));
		Point.Density = 1.0f;

		int32 Width = Lot.GetWidthInCells();
		int32 Height = Lot.GetHeightInCells();

		const int64 Key = Metadata->AddEntry();
		Point.MetadataEntry = Key;

		AttrCenterPosition->SetValue(Key, Lot.Center);
		AttrWidth->SetValue(Key, Width);
		AttrHeight->SetValue(Key, Height);

		OutPoints.Add(Point);
	}

	FPCGTaggedData& Tagged = Context->OutputData.TaggedData.Emplace_GetRef();
	Tagged.Pin = FName(TEXT("LotPoints"));
	Tagged.Data = OutData;

	return true;
}

#undef LOCTEXT_NAMESPACE