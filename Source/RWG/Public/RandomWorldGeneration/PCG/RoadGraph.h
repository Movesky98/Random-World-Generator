// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum class ERoadNodeType : uint8
{
	Unknown,
	DeadEnd,		// Degree == 1, 막다른 길
	PassThrough,	// Degree == 2, 직선 도로
	Bend,			// Degree == 2, 커브
	Intersection,	// Degree >= 3, 교차로
};

struct FRoadNode
{
	int32 Id = INDEX_NONE;
	FVector Position = FVector::ZeroVector;

	// 인접 엣지 Id 리스트
	TArray<int32> EdgeIds;

	explicit FRoadNode(int32 InId = INDEX_NONE, const FVector& InPos = FVector::ZeroVector)
		: Id(InId), Position(InPos) { }
};

struct FRoadEdge
{
	int32 Id = INDEX_NONE;
	int32 StartNodeId = INDEX_NONE;
	int32 EndNodeId = INDEX_NONE;

	// 메타 데이터
	float HalfWidth = 400.0f;
	int32 RoadType = 0;
	FVector Direction = FVector::ZeroVector;

	// 자연 곡선을 위한 제어점. 월드 좌표 기준 중간점들임 
	TArray<FVector> SegmentPoints;

	FRoadEdge() = default;
	FRoadEdge(int32 InId, int32 InStartNodeId, int32 InEndNodeId, float InHalfWidth = 400.0f, int32 InRoadType = 0)
		: Id(InId), StartNodeId(InStartNodeId), EndNodeId(InEndNodeId), HalfWidth(InHalfWidth), RoadType(InRoadType) { }
};

/**
 * 
 */
class RWG_API FRoadGraph
{
public:
	int32 AddNode(const FVector& Position);
	int32 AddEdge(int32 StartNodeId, int32 EndNodeId, float Width = 400.0f, int32 RoadType = 0);

	const FRoadNode* GetNode(int32 NodeId) const;
	FRoadNode* GetNodeMutable(int32 NodeId);

	const FRoadEdge* GetEdge(int32 EdgeId) const;
	FRoadEdge* GetEdgeMutable(int32 EdgeId);

	int32 GetNodeDegree(int32 NodeId) const;

	// NodeId 를 기준으로 Edge 반대편에 연결된 NodeId 반환
	int32 GetOtherNode(int32 EdgeId, int32 NodeId) const;

	ERoadNodeType ClassifyNode(int32 NodeId, float StraightDotThreshold = 0.95f) const;
	
	// Degree == 2일 때, 직선 도로인지 판단하는 함수
	bool IsPassThrough(int32 NodeId, float StraightDotThreshold = 0.95f) const;

	// 컨테이너
	const TArray<FRoadNode>& GetNodes() const { return Nodes; }
	const TArray<FRoadEdge>& GetEdges() const { return Edges; }

private:
	TArray<FRoadNode> Nodes;
	TArray<FRoadEdge> Edges;

	bool IsValidNodeId(int32 NodeId) const { return Nodes.IsValidIndex(NodeId); }
	bool IsValidEdgeId(int32 EdgeId) const { return Edges.IsValidIndex(EdgeId); }
};
