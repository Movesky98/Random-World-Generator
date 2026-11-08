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

	// Create Spline의 커스텀 탄젠트 기본 속성 이름과 같다
	static const FName ArriveTangent = TEXT("ArriveTangent");
	static const FName LeaveTangent = TEXT("LeaveTangent");
}

namespace IntersectionAttributes
{
	static const FName IntersectionId = TEXT("IntersectionId");
	static const FName OutlineIndex = TEXT("OutlineIndex");	// 윤곽 안에서 점의 순서 (노드 기준 반시계)
}

namespace
{
	// 엣지 점들이 놓인 원의 XY 중심과 반지름 (처음·가운데·마지막 점 기준, 일직선이면 false)
	bool FindArcCircle(const TArray<FVector>& Points, FVector2D& OutCenter, double& OutRadius)
	{
		if (Points.Num() < 3) return false;

		const FVector2D A(Points[0]);
		const FVector2D B(Points[Points.Num() / 2]);
		const FVector2D C(Points.Last());

		// 세 점이 이루는 각의 사인이 거의 0이면 일직선
		const double Cross = FVector2D::CrossProduct(B - A, C - A);
		if (FMath::Abs(Cross) <= 1.0e-4 * (B - A).Size() * (C - A).Size()) return false;

		const double D = 2.0 * Cross;
		const double LenA = A.SizeSquared();
		const double LenB = B.SizeSquared();
		const double LenC = C.SizeSquared();

		OutCenter.X = (LenA * (B.Y - C.Y) + LenB * (C.Y - A.Y) + LenC * (A.Y - B.Y)) / D;
		OutCenter.Y = (LenA * (C.X - B.X) + LenB * (A.X - C.X) + LenC * (B.X - A.X)) / D;
		OutRadius = FVector2D::Distance(A, OutCenter);

		return true;
	}

	// 원호 위 Start → End 구간의 Start 쪽(bAtEnd: End 쪽) 에르미트 탄젠트
	FVector GetArcTangent(const FVector& Start, const FVector& End, const FVector2D& Center, double Radius, bool bAtEnd)
	{
		const FVector2D StartOffset = FVector2D(Start) - Center;
		const FVector2D EndOffset = FVector2D(End) - Center;

		// 구간의 중심각과 도는 방향 (반시계 +1)
		const double Cross = FVector2D::CrossProduct(StartOffset, EndOffset);
		const double Angle = FMath::Atan2(FMath::Abs(Cross), FVector2D::DotProduct(StartOffset, EndOffset));
		const double Sign = (Cross >= 0.0) ? 1.0 : -1.0;

		// 원호를 덮는 에르미트 탄젠트 길이 4·r·tan(φ/4)
		const double Length = 4.0 * Radius * FMath::Tan(Angle * 0.25);

		const FVector2D Offset = bAtEnd ? EndOffset : StartOffset;
		const FVector2D Direction = FVector2D(-Offset.Y, Offset.X).GetSafeNormal() * Sign;

		return FVector(Direction.X * Length, Direction.Y * Length, End.Z - Start.Z);
	}
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

FName URoadGraphToPointsSettings::GetIntersectionOutputLabel() const
{
	return TEXT("Intersections");
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
	Pins.Emplace(GetIntersectionOutputLabel(), EPCGDataType::Point);
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
	auto* AttrArriveTangent = MetaData->CreateAttribute<FVector>(RoadAttributes::ArriveTangent, FVector::ZeroVector, true, true);
	auto* AttrLeaveTangent = MetaData->CreateAttribute<FVector>(RoadAttributes::LeaveTangent, FVector::ZeroVector, true, true);

	const float Spacing = FMath::Max(10.0f, Settings->SegmentSpcing);

	// Edge 점 → 포인트 생성
	for (const FRoadEdge& Edge : Graph.GetEdges())
	{
		const TArray<FVector>& SegmentPoints = Edge.SegmentPoints;
		const int32 NumSegments = SegmentPoints.Num();

		// 점들이 원 위에 있으면 원호 엣지, 아니면 직선 엣지
		FVector2D ArcCenter = FVector2D::ZeroVector;
		double ArcRadius = 0.0;
		const bool bArc = FindArcCircle(SegmentPoints, ArcCenter, ArcRadius);

		for (int32 i = 0; i < NumSegments; ++i)
		{
			const float Alpha = (NumSegments <= 1) ? 0.0f : (float)i / (float)(NumSegments - 1);

			// 앞 구간 끝 쪽 탄젠트(Arrive)와 뒤 구간 시작 쪽 탄젠트(Leave)
			FVector ArriveTangent = FVector::ZeroVector;
			FVector LeaveTangent = FVector::ZeroVector;

			if (i > 0)
			{
				ArriveTangent = bArc
					? GetArcTangent(SegmentPoints[i - 1], SegmentPoints[i], ArcCenter, ArcRadius, true)
					: SegmentPoints[i] - SegmentPoints[i - 1];
			}

			if (i < NumSegments - 1)
			{
				LeaveTangent = bArc
					? GetArcTangent(SegmentPoints[i], SegmentPoints[i + 1], ArcCenter, ArcRadius, false)
					: SegmentPoints[i + 1] - SegmentPoints[i];
			}

			// 엣지 양 끝 점은 한쪽 구간만 있으므로 같은 값을 쓴다
			if (i == 0) ArriveTangent = LeaveTangent;
			if (i == NumSegments - 1) LeaveTangent = ArriveTangent;

			const FVector PointForward = LeaveTangent.IsNearlyZero() ? Edge.Direction : LeaveTangent.GetSafeNormal();

			FPCGPoint Point;
			Point.Transform = FTransform(FRotationMatrix::MakeFromX(PointForward).ToQuat(), SegmentPoints[i], FVector::OneVector);
			Point.Density = 1.0f;

			const int64 Key = MetaData->AddEntry();
			Point.MetadataEntry = Key;

			AttrEdgeId->SetValue(Key, Edge.Id);
			AttrSegIndex->SetValue(Key, i);
			AttrHalfWidth->SetValue(Key, Edge.Width);
			AttrRoadType->SetValue(Key, Edge.RoadType);
			AttrEdgeAlpha->SetValue(Key, Alpha);
			AttrForward->SetValue(Key, Edge.Direction);
			AttrArriveTangent->SetValue(Key, ArriveTangent);
			AttrLeaveTangent->SetValue(Key, LeaveTangent);

			OutPoints.Add(Point);
		}
	}

	// Output으로 내보냄
	FPCGTaggedData& Tagged = Context->OutputData.TaggedData.Emplace_GetRef();
	Tagged.Pin = Settings->GetMainOutputLabel();
	Tagged.Data = OutData;

	// 교차로 윤곽 점 → 포인트 생성
	UPCGPointData* IntersectionData = NewObject<UPCGPointData>();
	IntersectionData->InitializeFromData(nullptr);

	TArray<FPCGPoint>& IntersectionPoints = IntersectionData->GetMutablePoints();
	UPCGMetadata* IntersectionMetaData = IntersectionData->Metadata;

	auto* AttrIntersectionId = IntersectionMetaData->CreateAttribute<int32>(IntersectionAttributes::IntersectionId, -1, false, true);
	auto* AttrOutlineIndex = IntersectionMetaData->CreateAttribute<int32>(IntersectionAttributes::OutlineIndex, -1, false, true);

	const TArray<FRoadIntersection>& Intersections = Graph.GetIntersections();

	for (int32 IntersectionId = 0; IntersectionId < Intersections.Num(); ++IntersectionId)
	{
		const TArray<FVector>& OutlinePoints = Intersections[IntersectionId].OutlinePoints;

		for (int32 i = 0; i < OutlinePoints.Num(); ++i)
		{
			FPCGPoint Point;
			Point.Transform = FTransform(OutlinePoints[i]);
			Point.Density = 1.0f;

			const int64 Key = IntersectionMetaData->AddEntry();
			Point.MetadataEntry = Key;

			AttrIntersectionId->SetValue(Key, IntersectionId);
			AttrOutlineIndex->SetValue(Key, i);

			IntersectionPoints.Add(Point);
		}
	}

	FPCGTaggedData& IntersectionTagged = Context->OutputData.TaggedData.Emplace_GetRef();
	IntersectionTagged.Pin = Settings->GetIntersectionOutputLabel();
	IntersectionTagged.Data = IntersectionData;

	return true;
}

#undef LOCTEXT_NAMESPACE
