// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWorldGeneration/PCG/RoadGraphToPoints.h"
#include "RandomWorldGeneration/PCG/RoadGraph.h"
#include "RandomWorldGeneration/Actors/WorldGenerator.h"

#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Metadata/PCGMetadata.h"


#define LOCTEXT_NAMESPACE "RoadGraphToPointsElement"

namespace RoadAttributes
{
	static const FName EdgeId = TEXT("EdgeId");
	static const FName SegIndex = TEXT("SegmentIndex");
	static const FName HalfWidth = TEXT("HalfWidth");
	static const FName RoadType = TEXT("RoadType");
	static const FName EdgeAlpha = TEXT("EdgeAlpha");	// 포인트가 해당 엣지 위에서 얼마나 진행되었는지 (0 ~ 1)
	static const FName Forward = TEXT("Forward");
}

#if WITH_EDITOR
// The label the node is known as internally.
FName URoadGraphToPointsSettings::GetDefaultNodeName() const
{
	return FName(TEXT("RoadGraphToPoints"));
}

// Default node name shown in the graph editor. Include spaces.
FText URoadGraphToPointsSettings::GetDefaultNodeTitle() const
{
	return LOCTEXT("NodeTitle", "RoadGraphToPoints");
}

// Default tooltip for the node
FText URoadGraphToPointsSettings::GetNodeTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Read RoadGraph from WorldGenerator and output sampled road points.");
}
#endif //WITH_EDITOR

FName URoadGraphToPointsSettings::GetMainOutputLabel() const
{
	return TEXT("RoadPoints");
}

// Input/Output pin setup with specific properties, including:
// Pin data type, allowing singular or multiple inputs per pin, and creating multiple in/out pins.
TArray<FPCGPinProperties> URoadGraphToPointsSettings::InputPinProperties() const
{
	// WorldGenerator를 참조하기 때문에 입력 없음.
	return { };
}

TArray<FPCGPinProperties> URoadGraphToPointsSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(GetMainOutputLabel(), EPCGDataType::Point);
	return Pins;
}

// Creates the Element to be used for ExecuteInternal.
FPCGElementPtr URoadGraphToPointsSettings::CreateElement() const
{
	return MakeShared<FRoadGraphToPointsElement>();
}

bool FRoadGraphToPointsElement::ExecuteInternal(FPCGContext* Context) const
{
	const URoadGraphToPointsSettings* Settings = Context ? Context->GetInputSettings<URoadGraphToPointsSettings>() : nullptr;
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

	const FRoadGraph& Graph = WorldGenerator->GetRoadGraph();

	// PointData 생성
	UPCGPointData* OutData = NewObject<UPCGPointData>();
	OutData->InitializeFromData(nullptr);

	TArray<FPCGPoint>& OutPoints = OutData->GetMutablePoints();
	UPCGMetadata* MetaData = OutData->Metadata;

	auto* AttrEdgeId = MetaData->CreateAttribute<int32>(RoadAttributes::EdgeId, -1, false, true);
	auto* AttrSegIndex = MetaData->CreateAttribute<int32>(RoadAttributes::SegIndex, -1, false, true);
	auto* AttrHalfWidth = MetaData->CreateAttribute<float>(RoadAttributes::HalfWidth, 400.0f, true, true);
	auto* AttrRoadType = MetaData->CreateAttribute<int32>(RoadAttributes::RoadType, 0, false, true);
	auto* AttrEdgeAlpha = MetaData->CreateAttribute<float>(RoadAttributes::EdgeAlpha, 0.0f, true, true);
	auto* AttrForward = MetaData->CreateAttribute<FVector>(RoadAttributes::Forward, FVector::ForwardVector, true, true);

	const float Spacing = FMath::Max(10.0f, Settings->SegmentSpcing);

	// 직선 Edge 샘플링 → 포인트 생성
	for (const FRoadEdge& Edge : Graph.GetEdges())
	{
		const int32 NumSegments = Edge.SegmentPoints.Num();

		for (int32 i = 0; i < NumSegments; ++i)
		{
			const float Alpha = (NumSegments <= 1) ? 0.0f : (float)i / (float)(NumSegments - 1);

			FPCGPoint Point;
			Point.Transform = FTransform(FRotationMatrix::MakeFromX(Edge.Direction).ToQuat(), Edge.SegmentPoints[i], FVector::OneVector);
			Point.Density = 1.0f;

			const int64 Key = MetaData->AddEntry();
			Point.MetadataEntry = Key;

			AttrEdgeId->SetValue(Key, Edge.Id);
			AttrSegIndex->SetValue(Key, i);
			AttrHalfWidth->SetValue(Key, Edge.HalfWidth);
			AttrRoadType->SetValue(Key, Edge.RoadType);
			AttrEdgeAlpha->SetValue(Key, Alpha);
			AttrForward->SetValue(Key, Edge.Direction);

			OutPoints.Add(Point);
		}
	}

	// Output으로 내보냄
	FPCGTaggedData& Tagged = Context->OutputData.TaggedData.Emplace_GetRef();
	Tagged.Pin = Settings->GetMainOutputLabel();
	Tagged.Data = OutData;

	return true;
}

#undef LOCTEXT_NAMESPACE