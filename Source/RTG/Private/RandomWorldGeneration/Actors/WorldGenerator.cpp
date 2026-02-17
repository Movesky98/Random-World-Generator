// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWorldGeneration/Actors/WorldGenerator.h"
#include "RandomWorldGeneration/DataAssets/WorldGenConfig.h"

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

void AWorldGenerator::UpdateMesh(int32 Seed, class UWorldGenConfig* Config)
{
	if (!Config)
	{
		UE_LOG(LogWorldGenerator, Error, TEXT("UpdateMesh() :: Config is not found."))
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;
	TArray<FLinearColor> VertexColors;

	float CenterX = Config->XSize / 2.0f;
	float CenterY = Config->YSize / 2.0f;
	
	FVector2D CenterPos(CenterX, CenterY);

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


	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		if (PCGComponent && PCGComponent->GetGraph())
		{
			PCGComponent->Cleanup();
			PCGComponent->Generate();
			
			UE_LOG(LogWorldGenerator, Warning, TEXT("PCG Generated on next tick."));
		}
	});
}

// Called when the game starts or when spawned
void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

