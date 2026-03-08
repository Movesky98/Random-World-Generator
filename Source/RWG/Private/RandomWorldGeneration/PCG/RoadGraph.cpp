// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/PCG/RoadGraph.h"

int32 FRoadGraph::AddNode(const FVector& Position)
{
	const int32 NewId = Nodes.Num();
	Nodes.Emplace(NewId, Position);

	return NewId;
}

int32 FRoadGraph::AddEdge(int32 StartNodeId, int32 EndNodeId, float Width, int32 RoadType)
{
	if (!IsValidNodeId(StartNodeId) || !IsValidNodeId(EndNodeId) || StartNodeId == EndNodeId)
	{
		return INDEX_NONE;
	}

	const int32 NewId = Edges.Num();
	Edges.Emplace(NewId, StartNodeId, EndNodeId, Width, RoadType);

	Nodes[StartNodeId].EdgeIds.Add(NewId);
	Nodes[EndNodeId].EdgeIds.Add(NewId);

	// Add SegmentPoints, Direction
	const FVector SPos = Nodes[StartNodeId].Position;
	const FVector EPos = Nodes[EndNodeId].Position;

	FRoadEdge* Edge = GetEdgeMutable(NewId);
	Edge->Direction = SPos - EPos;
	const float Length = Edge->Direction.Length();
	
	// 두 점의 위치가 거의 같을 경우 패스
	if (Length < KINDA_SMALL_NUMBER)
		return NewId;
	
	Edge->Direction /= Length;

	const float Spacing = 400.0f;
	const int32 NumSegment = FMath::Max(1, FMath::FloorToInt(Length / Spacing));
	
	for (int32 i = 0; i < NumSegment + 1; i++)
	{
		const float Alpha = (!NumSegment) ? 0.0f : (float)i / (float)(NumSegment);
		FVector Pos = FMath::Lerp(SPos, EPos, Alpha);

		Edge->SegmentPoints.Add(Pos);
	}

	return NewId;
}

const FRoadNode* FRoadGraph::GetNode(int32 NodeId) const
{
	return IsValidNodeId(NodeId) ? &Nodes[NodeId] : nullptr;
}

FRoadNode* FRoadGraph::GetNodeMutable(int32 NodeId)
{
	return IsValidNodeId(NodeId) ? &Nodes[NodeId] : nullptr;
}

const FRoadEdge* FRoadGraph::GetEdge(int32 EdgeId) const
{
	return IsValidEdgeId(EdgeId) ? &Edges[EdgeId] : nullptr;
}

FRoadEdge* FRoadGraph::GetEdgeMutable(int32 EdgeId)
{
	return IsValidEdgeId(EdgeId) ? &Edges[EdgeId] : nullptr;
}

int32 FRoadGraph::GetNodeDegree(int32 NodeId) const
{
	const FRoadNode* N = GetNode(NodeId);
	return N ? N->EdgeIds.Num() : 0;
}

int32 FRoadGraph::GetOtherNode(int32 EdgeId, int32 NodeId) const
{
	const FRoadEdge* E = GetEdge(EdgeId);
	if (!E) return INDEX_NONE;

	if (E->StartNodeId == NodeId) return E->EndNodeId;
	if (E->EndNodeId == NodeId) return E->StartNodeId;

	return INDEX_NONE;
}

ERoadNodeType FRoadGraph::ClassifyNode(int32 NodeId, float StraightDotThreshold) const
{
	const int32 Degree = GetNodeDegree(NodeId);
	
	if (Degree <= 0) return ERoadNodeType::Unknown;
	if (Degree == 1) return ERoadNodeType::DeadEnd;
	if (Degree >= 3) return ERoadNodeType::Intersection;

	return IsPassThrough(NodeId) ? ERoadNodeType::PassThrough : ERoadNodeType::Bend;
}

bool FRoadGraph::IsPassThrough(int32 NodeId, float StraightDotThreshold) const
{
	const FRoadNode* N = GetNode(NodeId);
	if (!N || N->EdgeIds.Num() != 2) return false;

	const int32 EdgeA = N->EdgeIds[0];
	const int32 EdgeB = N->EdgeIds[1];

	const int32 NodeA = GetOtherNode(EdgeA, NodeId);
	const int32 NodeB = GetOtherNode(EdgeB, NodeId);
	if (NodeA == INDEX_NONE || NodeB == INDEX_NONE) return false;

	const FVector VectorA = (Nodes[NodeA].Position - N->Position).GetSafeNormal();
	const FVector VectorB = (Nodes[NodeB].Position - N->Position).GetSafeNormal();

	// 내적, 직선 통과는 반대 방향에 가까워야 한다. (-1)
	const float Dot = FVector::DotProduct(VectorA, VectorB);
	return Dot <= -StraightDotThreshold;
}
