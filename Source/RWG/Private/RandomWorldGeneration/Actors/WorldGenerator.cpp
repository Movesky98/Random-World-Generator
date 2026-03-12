// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Actors/WorldGenerator.h"

#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"
#include "RandomWorldGeneration/DataAssets/WorldThemeConfig.h"
#include "RandomWorldGeneration/Core/WorldGenTypes.h"
#include "RandomWorldGeneration/PCG/RoadGraphBuilder.h"
#include "RandomWorldGeneration/Grid/CityGridBuilder.h"

#include "PCGGraph.h"

DEFINE_LOG_CATEGORY(LogWorldGenerator);

// Sets default values
AWorldGenerator::AWorldGenerator()
{
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	ProceduralMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = ProceduralMeshComponent;

	PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCG Component"));
	PCGComponent->bAutoActivate = false;
}

void AWorldGenerator::GenerateWorld(int32 Seed, TMap<FPrimaryAssetType, TObjectPtr<UObject>> Configs)
{
	for (auto& Config : Configs)
	{
		if (Config.Key == FPrimaryAssetType(WorldConfigTags::GenConfigName))
		{
			UWorldGenConfig* GenConfig = Cast<UWorldGenConfig>(Config.Value);

			GenerateTerrain(GenConfig);
		}
		else if (Config.Key == FPrimaryAssetType(WorldConfigTags::ThemeConfigName))
		{
			UWorldThemeConfig* ThemeConfig = Cast<UWorldThemeConfig>(Config.Value);

			GenerateContent(ThemeConfig);
		}
	}
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

	MainCityCenter = GetActorLocation() + FVector(HalfWidth, HalfHeight, 0.0f);
	MainCityRadius = Config->CityRadius * Config->GridSpacing;
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

void AWorldGenerator::InitializePCGConfigs(const FVector& CityCenter, const FVector& CityRadius)
{
	// PCG Graph에 파라미터 전달


}

void AWorldGenerator::GenerateContent(UWorldThemeConfig* Config)
{
	FRoadBuildParams RoadBuildParams;
	RoadBuildParams.CityCenterXY = FVector2D(MainCityCenter.X, MainCityCenter.Y);
	RoadBuildParams.CityRadius = MainCityRadius;
	RoadBuildParams.CityHeight = CityHeight;

	RoadGraph = FRoadGraphBuilder::BuildGraph2D(GetWorld(), RoadBuildParams);

	FGridBuildParams GridBuildParams;
	GridBuildParams.CellSize = 400.0f;
	GridBuildParams.CityCenter = MainCityCenter;
	GridBuildParams.CityRadius = MainCityRadius;
	GridBuildParams.RoadGraph = RoadGraph;

	CityGrid = FCityGridBuilder::BuildGrid2D(GridBuildParams);
	CityBlocks = FCityGridBuilder::BuildBlocks(CityGrid);
	FCityGridBuilder::ExtractLotsFromBlock(CityGrid, CityBlocks);

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, Config]()
		{
			if (PCGComponent && PCGComponent->GetGraph())
			{
				PCGComponent->GetGraph()->SetGraphParameter(FName("MainCityCenter"), MainCityCenter);
				PCGComponent->GetGraph()->SetGraphParameter(FName("MainCityRadius"), MainCityRadius);

				// Set Static Meshs by Theme
				SetPCGObjectParameter(WorldThemeStructureTags::MainBuildingName, Config->MainBuilding);
				TSoftObjectPtr<UObject> ThemePtr(Config);
				PCGComponent->GetGraph()->SetGraphParameter(FName("ThemeConfig"), ThemePtr);

				PCGComponent->Generate();

				UE_LOG(LogWorldGenerator, Warning, TEXT("PCG Generated on next tick."));

				DrawDebugGrid();
			}
		});
}

// Called when the game starts or when spawned
void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	
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

			Color = CityCells[i].BlockId != -1 ? Colors[CityCells[i].BlockId] : FColor::Black;


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

		// DrawDebugString(World, CityCells[i].WorldPosition, FString::FromInt(i), 0, FColor::White, -1.0f, false, 30.0f);
	}
}

template<typename TObject>
void AWorldGenerator::SetPCGObjectParameter(FName PropertyName, TObject* Value)
{
	static_assert(TIsDerivedFrom<TObject, UObject>::IsDerived, "TObject must derive from UObject");
	PCGComponent->GetGraph()->SetGraphParameter(PropertyName, Cast<UObject>(Value));
}

template <typename TObject>
void AWorldGenerator::SetPCGObjectParameters(FName PropertyName, const TArray<TObject*>& Values)
{
	static_assert(TIsDerivedFrom<TObject, UObject>::IsDerived, "TObject must derive from UObject");

	TArray<UObject*> Objs;
	Objs.Reserve(Values.Num());

	for (TObject* Obj : Values)
	{
		Objs.Add(Obj);
	}

	PCGComponent->GetGraph()->UpdateArrayGraphParameter(PropertyName, Objs);
}

