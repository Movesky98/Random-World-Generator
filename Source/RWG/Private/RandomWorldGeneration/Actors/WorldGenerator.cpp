// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Actors/WorldGenerator.h"

#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"
#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"
#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "RandomWorldGeneration/PCG/RoadGraphBuilder.h"
#include "RandomWorldGeneration/Grid/CityGridBuilder.h"

#include "PCGGraph.h"
#include "NavigationSystem.h"

#include "RandomWorldGeneration/Actors/BuildingActor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogWorldGenerator);

// Sets default values
AWorldGenerator::AWorldGenerator()
{
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	ProceduralMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = ProceduralMeshComponent;

	RoadPCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("RoadPCGComponent"));
	RoadPCGComponent->bAutoActivate = false;

	BuildingPCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("BuildingPCGComponent"));
	BuildingPCGComponent->bAutoActivate = false;

	ProceduralMeshComponent->bFillCollisionUnderneathForNavmesh = false;
	ProceduralMeshComponent->SetCanEverAffectNavigation(false);

	TerrainNavProxyPMC = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainNavProxyPMC"));
	TerrainNavProxyPMC->SetupAttachment(RootComponent);
	TerrainNavProxyPMC->SetHiddenInGame(true);
	TerrainNavProxyPMC->bFillCollisionUnderneathForNavmesh = true;
	TerrainNavProxyPMC->SetCanEverAffectNavigation(true);
}

void AWorldGenerator::GenerateWorld(TMap<FPrimaryAssetType, TObjectPtr<UObject>> Configs)
{
	if (Configs.Num() == 0)
	{
		return;
	}

	UWorldGenConfig* GenConfig = nullptr;
	UWorldThemeConfig* ThemeConfig = nullptr;

	for (auto& Config : Configs)
	{
		if (Config.Key == FPrimaryAssetType(WorldConfigTags::GenConfigName))
		{
			GenConfig = Cast<UWorldGenConfig>(Config.Value);
		}
		else if (Config.Key == FPrimaryAssetType(WorldConfigTags::ThemeConfigName))
		{
			ThemeConfig = Cast<UWorldThemeConfig>(Config.Value);
		}
	}

	if (!ensure(GenConfig))
	{
		return;
	}

	GenerateTerrain(GenConfig);

	GenerateNavProxyMesh(GenConfig);

	if(!ensure(ThemeConfig))
	{
		return;
	}

	GenerateContent(ThemeConfig);
}

void AWorldGenerator::GenerateTerrain(UWorldGenConfig* Config)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;
	TArray<FLinearColor> VertexColors;

	float CenterX = Config->XSize / 2.0f;
	float CenterY = Config->YSize / 2.0f;

	FVector2D CenterPos(CenterX, CenterY);

	float HalfWidth = Config->XSize * Config->GridSpacing / 2;
	float HalfHeight = Config->YSize * Config->GridSpacing / 2;

	CityCenter = GetActorLocation() + FVector(HalfWidth, HalfHeight, 0.0f);
	CityRadius = Config->CityRadius * Config->GridSpacing;
	CityHeight = Config->CityHeight;

	for (int32 Y = 0; Y <= Config->YSize; Y++)
	{
		for (int32 X = 0; X <= Config->XSize; X++)
		{
			FVector2D CurrentPos(X, Y);

			// 1. Perlin Noise로 기본 높이 생성 (Scale 조절로 굴곡 빈도 결정)
			// 1. --- 기본 노이즈 지형 생성 ---
			float NoiseScale = 0.4;
			float NoiseValue = FMath::PerlinNoise2D(FVector2D(CurrentPos * NoiseScale));

			// 노이즈 값은 보통 -1 ~ 1 사이 → 0 ~ 1로 보정 후 높이 곱하기
			// 기본 굴곡
			float BaseHeight = (NoiseValue + 1.0f) * 0.5f * 500.0f;

			// --- 2. 도시 고원 (Plateau) 계산 ---
			float DistToCenter = FVector2D::Distance(CurrentPos, CenterPos);
			float CityMask = 0.0f;

			if (DistToCenter < Config->CityRadius)
			{
				CityMask = 1.0f; // 도시 내부 (완전 평지)
			}
			else if (DistToCenter < Config->CityRadius + Config->CliffWidth)
			{
				// 절벽 구간 (1.0 → 0.0 부드럽게 감소)
				CityMask = 1.0f - ((DistToCenter - Config->CityRadius) / Config->CliffWidth);
				CityMask = FMath::InterpEaseInOut(0.0f, 1.0f, CityMask, 2.0f);
			}

			// FinalZ = (도시/다리 높이로 보간) + (마스크가 없는 곳은 기본 노이즈 지형)
			float FinalZ = FMath::Lerp(BaseHeight + Config->OuterHeight, Config->CityHeight, CityMask);

			Vertices.Add(FVector(X * Config->GridSpacing, Y * Config->GridSpacing, FinalZ));
			UV0.Add(FVector2D(X, Y));		// 텍스처 타일링을 위해 정수 단위로 설정
			Normals.Add(FVector(0, 0, 1));	// 하늘 방향
		}
	}

	// 2. 삼각형(Triangles) 생성 (인덱스 계산)
	// 1개의 사각형(Quad)은 2개의 삼각형으로 이루어진다.
	for (int32 Y = 0; Y < Config->YSize; Y++)
	{
		for (int32 X = 0; X < Config->XSize; X++)
		{
			int32 VertexIndex = X + (Y * (Config->XSize + 1));

			// 첫 번째 삼각형
			Triangles.Add(VertexIndex);
			Triangles.Add(VertexIndex + Config->XSize + 1);
			Triangles.Add(VertexIndex + 1);

			// 두 번째 삼각형
			Triangles.Add(VertexIndex + 1);
			Triangles.Add(VertexIndex + Config->XSize + 1);
			Triangles.Add(VertexIndex + Config->XSize + 2);
		}
	}

	// 3. 실제 메쉬 섹션 생성
	ProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);

	// 4. 충돌체 활성화
	ProceduralMeshComponent->ContainsPhysicsTriMeshData(true);
	ProceduralMeshComponent->bUseComplexAsSimpleCollision = true;
	ProceduralMeshComponent->UpdateBounds();
}

void AWorldGenerator::GenerateContent(UWorldThemeConfig* Config)
{
	FRoadBuildParams RoadBuildParams;
	RoadBuildParams.Seed = MasterSeed;
	RoadBuildParams.CityCenterXY = FVector2D(CityCenter.X, CityCenter.Y);
	RoadBuildParams.CityRadius = CityRadius;
	RoadBuildParams.CityHeight = CityHeight;
	RoadBuildParams.MinRoadNum = Config->MinRoadNum;
	RoadBuildParams.MaxRoadNum = Config->MaxRoadNum;
	RoadBuildParams.RoadWidth = Config->RoadWidth;

	RoadGraph = FRoadGraphBuilder::BuildGraph2D(GetWorld(), RoadBuildParams);

	FGridBuildParams GridBuildParams;
	GridBuildParams.CellSize = Config->CellSize;
	GridBuildParams.CityCenter = CityCenter;
	GridBuildParams.CityRadius = CityRadius;
	GridBuildParams.RoadGraph = RoadGraph;

	CityGrid = FCityGridBuilder::BuildGrid2D(GridBuildParams);
	CityBlocks = FCityGridBuilder::BuildBlocks(CityGrid);
	FCityGridBuilder::ExtractLotsFromBlock(CityGrid, CityBlocks);

	// DebugSeedResult();

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, Config]()
		{
			if (RoadPCGComponent && RoadPCGComponent->GetGraph())
			{
				RoadPCGComponent->GetGraph()->SetGraphParameter(FName("CityCenter"), CityCenter);
				RoadPCGComponent->GetGraph()->SetGraphParameter(FName("CityRadius"), CityRadius);
				RoadPCGComponent->GetGraph()->SetGraphParameter(FName("Seed"), MasterSeed + 2);

				// Set Static Meshs by Theme
				TSoftObjectPtr<UObject> ThemePtr(Config);
				RoadPCGComponent->GetGraph()->SetGraphParameter(FName("ThemeConfig"), ThemePtr);

				RoadPCGComponent->Generate();

				// DrawDebugGrid();
			}

			if (GetWorld()->GetNetMode() != NM_Client)
			{
				UE_LOG(LogWorldGenerator, Warning, TEXT("SERVER: Building PCG Generate"));
				if (BuildingPCGComponent && BuildingPCGComponent->GetGraph())
				{
					BuildingPCGComponent->GetGraph()->SetGraphParameter(FName("CityCenter"), CityCenter);
					BuildingPCGComponent->GetGraph()->SetGraphParameter(FName("CityRadius"), CityRadius);
					BuildingPCGComponent->GetGraph()->SetGraphParameter(FName("Seed"), MasterSeed + 2);

					TSoftObjectPtr<UObject> ThemePtr(Config);
					BuildingPCGComponent->GetGraph()->SetGraphParameter(FName("ThemeConfig"), ThemePtr);

					BuildingPCGComponent->OnPCGGraphGeneratedDelegate.AddUObject(this, &ThisClass::OnBuildingPCGGenerated);
					BuildingPCGComponent->Generate();
				}
			}
			else
			{
				UE_LOG(LogWorldGenerator, Warning, TEXT("CLIENT: Building PCG skipped"));
			}
		});
}

// Called when the game starts or when spawned
void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		NavSys->OnNavigationGenerationFinishedDelegate.AddDynamic(
			this, &AWorldGenerator::OnNavMeshBuilt);
	}
}

void AWorldGenerator::OnBuildingPCGGenerated(UPCGComponent* InComponent)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		UE_LOG(LogWorldGenerator, Warning, TEXT("Build NavSystem."));
		NavSys->Build();
	}

	bPCGComplete = true;
	CheckAllComplete();
}

void AWorldGenerator::GenerateNavProxyMesh(UWorldGenConfig* Config)
{
	int32 NavGridSize = Config->GridSpacing;
	float NavSpacing = (CityRadius * 2.0f) / NavGridSize;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;

	for (int32 Y = 0; Y <= NavGridSize; Y++)
	{
		for (int32 X = 0; X <= NavGridSize; X++)
		{
			float WorldX = CityCenter.X - CityRadius + X * NavSpacing;
			float WorldY = CityCenter.Y - CityRadius + Y * NavSpacing;

			Vertices.Add(FVector(WorldX, WorldY, CityHeight));
			UV0.Add(FVector2D(X, Y));
			Normals.Add(FVector(0, 0, 1));
		}
	}

	for (int32 Y = 0; Y < NavGridSize; Y++)
	{
		for (int32 X = 0; X < NavGridSize; X++)
		{
			// 쿼드 중심점이 도시 영역 안인지 체크
			float MidX = CityCenter.X - CityRadius + (X + 0.5f) * NavSpacing;
			float MidY = CityCenter.Y - CityRadius + (Y + 0.5f) * NavSpacing;

			float Dist = FVector2D::Distance(
				FVector2D(MidX, MidY),
				FVector2D(CityCenter.X, CityCenter.Y)
			);

			if (Dist > CityRadius) continue; // 도시 영역 밖이면 트라이앵글 생성 안 함

			int32 V = X + Y * (NavGridSize + 1);
			Triangles.Add(V);
			Triangles.Add(V + NavGridSize + 1);
			Triangles.Add(V + 1);
			Triangles.Add(V + 1);
			Triangles.Add(V + NavGridSize + 1);
			Triangles.Add(V + NavGridSize + 2);
		}
	}

	TArray<FLinearColor> Colors;
	TArray<FProcMeshTangent> Tangents;
	TerrainNavProxyPMC->CreateMeshSection_LinearColor(0, Vertices, Triangles,
		Normals, UV0, Colors, Tangents, true);

	TerrainNavProxyPMC->UpdateBounds();
	FNavigationSystem::UpdateComponentData(*TerrainNavProxyPMC);
}

void AWorldGenerator::DrawDebugGrid()
{
	const TArray<FCityCell>& CityCells = CityGrid.GetCityCells();
	UWorld* World = GetWorld();
	if (!CityCells.Num() || !World) return;

	const float CellSize = CityGrid.GetCellSize();

	TArray<FColor> Colors;
	Colors.SetNum(CityBlocks.Num());

	for (int32 i = 0; i < CityBlocks.Num(); ++i)
	{
		const float Hue = 360.0f * i / CityBlocks.Num();

		Colors[i] = FLinearColor::MakeFromHSV8(
			(uint8)(Hue / 360.0f * 255.0f),
			200,
			255
		).ToFColor(true);
	}

	for (int i = 0; i < CityCells.Num(); i++)
	{
		float MinX = CityCells[i].WorldPosition.X - CellSize / 2;
		float MaxX = CityCells[i].WorldPosition.X + CellSize / 2;
		float MinY = CityCells[i].WorldPosition.Y - CellSize / 2;
		float MaxY = CityCells[i].WorldPosition.Y + CellSize / 2;
		float Z = 8050.0f;

		FColor Color = FColor::White;

		switch (CityCells[i].Type)
		{
		case ECellType::Blocked:
			Color = FColor::Black;
			break;
		case ECellType::Empty:
		{
			int32 X = i % CityGrid.GetWidth();
			int32 Y = i / CityGrid.GetWidth();

			// Block 할당 시
			Color = CityCells[i].BlockId != -1 ? Colors[CityCells[i].BlockId] : FColor::Black;

			// Block 할당 안할 경우
			//Color = FColor::White;

			// Lot 표시
			for (const FCityLot& Lot : CityGrid.GetLots())
			{
				if (Lot.CellIndices.Contains(CityGrid.Index(X, Y)))
				{
					Color = FColor::White;
					break;
				}
			}
		}

			break;
		case ECellType::Road:
			Color = FColor::Yellow;
		default:
			break;
		}

		DrawDebugLine(World, FVector(MinX, MaxY, Z), FVector(MaxX, MaxY, Z), Color, true);	// Top
		DrawDebugLine(World, FVector(MinX, MinY, Z), FVector(MaxX, MinY, Z), Color, true);	// Bottom
		DrawDebugLine(World, FVector(MinX, MinY, Z), FVector(MinX, MaxY, Z), Color, true);	// Left
		DrawDebugLine(World, FVector(MaxX, MinY, Z), FVector(MaxX, MaxY, Z), Color, true);	// Right
	}
}

void AWorldGenerator::DebugSeedResult()
{
	UE_LOG(LogWorldGenerator, Warning, TEXT("Master Seed : %d"), MasterSeed);
	UE_LOG(LogWorldGenerator, Warning, TEXT("***************************** Road Graph *****************************"));
	UE_LOG(LogWorldGenerator, Warning, TEXT("Road Num : %d"), RoadGraph.GetEdges().Num());

	int32 SegmentCount = 0;
	for (const FRoadEdge& Edge : RoadGraph.GetEdges())
	{
		SegmentCount += Edge.SegmentPoints.Num();
	}

	UE_LOG(LogWorldGenerator, Warning, TEXT("Road's all segment Num : %d"), SegmentCount);

	for (int i = 0; i < RoadGraph.GetEdges().Num() && i < 10; i++)
	{
		int32 SNodeId = RoadGraph.GetEdges()[i].StartNodeId;
		int32 ENodeId = RoadGraph.GetEdges()[i].EndNodeId;
		const FRoadNode* SNode = RoadGraph.GetNode(SNodeId);
		const FRoadNode* ENode = RoadGraph.GetNode(ENodeId);

		UE_LOG(LogWorldGenerator, Warning, TEXT("%dth Road's StartNode Position : X = %.3f / Y = %.3f / Z = %.3f"), i + 1, SNode->Position.X, SNode->Position.Y, SNode->Position.Z);
		UE_LOG(LogWorldGenerator, Warning, TEXT("%dth Road's EndNode Position : X = %.3f / Y = %.3f / Z = %.3f"), i + 1, ENode->Position.X, ENode->Position.Y, ENode->Position.Z);
	}

	UE_LOG(LogWorldGenerator, Warning, TEXT("***************************** Grid *****************************"));
	
	int32 RoadCellCount = 0;
	int32 BuildableCellCount = 0;

	for (auto& Cell : CityGrid.GetCityCells())
	{
		if (Cell.Type == ECellType::Road)
			RoadCellCount++;
	}

	UE_LOG(LogWorldGenerator, Warning, TEXT("RoadCells : %d"), RoadCellCount);
	UE_LOG(LogWorldGenerator, Warning, TEXT("Lots : %d"), CityGrid.GetLots().Num());
	UE_LOG(LogWorldGenerator, Warning, TEXT("Blocks : %d"), CityBlocks.Num());

}

void AWorldGenerator::CheckAllComplete()
{
	if (!bPCGComplete || !bNavComplete) return;
	OnWorldGenerationComplete.Broadcast();

	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		NavSys->OnNavigationGenerationFinishedDelegate.RemoveAll(this);
	}
}

void AWorldGenerator::OnNavMeshBuilt(ANavigationData* NavData)
{
	bNavComplete = true;
	CheckAllComplete();
}
