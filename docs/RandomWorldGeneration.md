## 개요

Random World Generation은 RWG 프로젝트의 핵심 시스템입니다.

Seed 기반 결정론적 생성을 통해 매 세션마다 재현 가능한 도시 구조를 런타임에 생성합니다.

## 생성 파이프라인

```txt
Seed
→ Terrain 생성
→ RoadGraph 생성
→ Cell Grid 생성
→ Road Cell 마킹
→ Block 탐지
→ Building Lot 생성
→ Building 배치
→ PCG_Road, PCG_Building 생성
```


---
## Seed

월드 생성의 모든 과정은 Seed 값을 기반으로 동작합니다.

### 목표
- 동일한 Seed로 동일한 월드 생성
- 테스트 및 디버깅 시 동일한 결과 재현 가능
- Host와 Client가 동일한 Seed로 동일한 월드를 생성

### 구현 포인트

`FRandomStream`을 사용하여 결정론적 난수를 생성합니다.

각 생성 단계는 동일한 Seed에서 동일한 난수 흐름을 사용하도록 구성하여,
같은 입력 값에 대해 같은 결과가 나오도록 설계했습니다.


---
## Terrain 생성

지형은 런타임에 생성됩니다.

### 구현 방식
- Procedural Mesh Component 기반
- Grid Height Data 기반 메시 생성

### 설계 판단

초기에는 Unreal Landscape를 이용한 지형 생성을 고려했습니다.

하지만 Unreal Landscape는 에디터 중심의 지형 제작 도구로, 
런타임에서 Seed 기반으로 지형을 동적으로 생성하는 구조에는 적합하지 않다고 판단했습니다.

따라서 런타임 메시 생성이 가능한 Procedural Mesh Component를 선택했습니다.

다만 Procedural Mesh Component 기반 지형은 Navigation System 연동이 자동으로 보장되지 않으므로, AI 이동이 필요한 단계에서는 NavMesh 생성 및 갱신 방식을 별도로 검증해야 합니다.
### 결과
1. 지형 생성 전
<img src="Images/RWG_지형 생성 전.png" width="500">

2. 지형 생성 후
<img src="Images/RWG_지형 생성 후.png" width="500">


---
## 도시 영역

도시는 현재 원형 영역으로 정의됩니다.
```cpp
FVector CityCenter
float CityRadius
```

이 값은 Road 생성, Cell 필터링, PCG 배치의 기준이 됩니다.

---
## Road Graph

도로는 그래프 구조로 표현됩니다.

### 핵심 구조
```cpp
FRoadGraph
FRoadNode
FRoadEdge
FRoadGraphBuilder
```

### 역할
- 도로 노드 배치
- 노드 간 Edge 연결
- 도로 위상 정보 저장
- Road 시각화를 위한 데이터 제공
- Cell 기반 Block 탐지에 데이터 제공

---
## Road 시각화

RoadEdge를 샘플링하여 Spline 기반으로 시각화합니다.

### 흐름
```txt
RoadEdge
→ 샘플링 된 도로 포인트
→ Spline Control Point
→ PCG_Road 콘텐츠 생성
```

Road Graph는 논리적 도로 구조를 담당하고, 시각화는 Spline 및 PCG를 통해 별도로 처리합니다.

### 결과
1. 도로 생성 - 근접
<img src="Images/RWG_도로 생성 근접.png" width="500">

2. 도로 생성 - TDV
<img src="Images/RWG_도로 생성 TDV.png" width="500">

3. 도로 시각화 - PCG Road Graph
<img src="Images/RWG_도로 시각화 PCG.png">


---
## Cell Grid

도시 영역을 Cell 단위로 분할합니다.

### 역할
- 도시 영역을 관리 가능한 단위로 분할
- 도시 반경 외부 Cell 제외
- 도로가 지나가는 Cell 마킹
- Block 탐지 입력 데이터 제공

---
## Road Cell 마킹

각 Cell은 도로 Sgment와의 거리를 계산하여 도로 여부를 판단합니다.

```txt
Distance(CellCenter, RoadSegment) < RoadHalfWidth
```

조건을 만족하는 Cell은 Road Cell로 마킹됩니다.

이후 Block 탐지 단계에서는 Road Cell을 제외하고 건물 배치 가능한 영역을 찾습니다.

### 결과
1. Cell 생성 후 도로 마킹
<img src="Images/RWG_Cell 생성 후 디버그.png" width="500">


---
## Block 탐지

건물을 배치할 수 있는 도시 블록을 탐지합니다.

### 방식
- RoadCell 제외
- 도시 반경 외부 Cell 제외
- 인접한 Buildable Cell 그룹화
- Flood Fill을 이용한 Block 영역 탐지

### 결과
탐지된 Block은 Building Lot 생성의 후보 영역으로 사용됩니다.

1. Cell Grid - 블록 탐지
<img src="Images/RWG_Cell Grid - 블록 탐지.png" width="500">


---
## Building Lot 생성

탐지된 Block 내부에서 건물 배치 후보 영역을 생성합니다.

### 목표
- 유효한 Block 내부에 건물 배치
- 도로와의 겹침 방지
- 건물 간 간격 적용
- 인접 도로 방향에 맞춘 건물 정렬 (계획, 미구현)

### 결과
1. Cell Grid - Building Lot 생성
<img src="Images/RWG_Cell Grid - Building Lot 생성.png" width="500">


---
## Building 배치

생성된 Lot 데이터와 ThemeConfig를 기반으로 건물을 배치합니다.

### 멀티플레이 처리

초기 구현에서는 Host와 Client가 각각 Building PCG를 실행할 수 있는 구조였습니다.

이 방식은 동일한 Seed를 사용하더라도 서버와 클라이언트 양쪽에서 건물 Actor가 생성될 수 있어, 멀티플레이 환경에서 중복 생성 문제가 발생할 수 있습니다.

```txt
초기 구조

Host
	Building PCG 실행 → Building Actor 생성

Client 
	Building PCG 실행 → Building Actor 생성
```

이를 해결하기 위해 Building 생성 로직을 서버 권한 기준으로 분리했습니다.

```txt
수정 구조

Server
→ Building PCG 실행
→ ABuildingActor Spawn
→ bReplicates = true

Client
→ Building 생성 로직 실행 안함
→ 서버에서 생성된 ABuildingActor를 Replication으로 수신
```

이 구조를 통해 건물 생성 주체를 서버로 단일화하고,
추후 건물에 충돌, 상호작용, 파괴 가능성 같은 게임플레이 요소가 추가되더라도
서버 권한 기준으로 일관되게 처리할 수 있도록 했습니다.

### 결과
1. Cell Grid 有 - 빌딩 생성
<img src="Images/RWG_Cell Grid 有 - 빌딩 생성.png" width="500">

2. Cell Grid 無 - 빌딩 생성
<img src="Images/RWG_Cell Grid 無 - 빌딩 생성.png" width="500">

3. 멀티플레이 처리
<img src="Images/RWG_멀티플레이 처리.png" width="500">


---
## PCG 기반 콘텐츠 생성

RWG는 C++에서 계산한 런타임 생성 데이터를 PCG Graph로 전달하여,
도로와 건물 같은 월드 콘텐츠를 실제 Actor 또는 Mesh 형태로 배치합니다.

C++ 계층은 도시 구조 데이터를 계산하고,
PCG 계층은 해당 데이터를 기반으로 월드에 배치될 콘텐츠를 생성하는 역할을 담당합니다.

### 분리 구조

초기에는 하나의 PCG 흐름에서 도로와 건물 콘텐츠 생성을 함께 처리했습니다.

하지만 도로와 건물은 입력 데이터, 생성 방식, 멀티플레이 처리 방식이 서로 달랐습니다.
```txt
도로 → RoadGraph 기반, 시각적 도로 Mesh 배치
건물 → Lot 기반, 서버 권한 Actor Spawn
```

따라서 PCG Graph를 도로와 건물용으로 분리했습니다.

```txt
PCG_Road
→ RoadGraph 기반 도로 콘텐츠 생성

PCG_Building
→ Lot 데이터 기반 건물 Actor 생성
```

이 분리를 통해 각 Graph가 자신에게 필요한 입력 데이터와 생성 책임만 처리하도록 구성했습니다.


### C++ 데이터 → PCG Point 변환

C++에서 생성된 RoadGraph와 Lot 데이터는 그대로 PCG Graph에서 사용할 수 없습니다.

PCG Graph는 주로 Point 데이터를 기반으로 필터링, 변환, 스폰을 수행하므로,
런타임 생성 데이터를 PCG Point로 변환하는 Custom PCG Node를 구현했습니다.

```txt
RoadGraphToPoint
→ RoadGraph의 Edge/Segment 정보를 PCG Point로 변환
→ 도로 Mesh 배치 위치와 방향 계산에 사용

LotToPoint
→ 생성된 Lot 데이터를 PCG Point로 변환
→ 건물 배치 위치, 회전, 크기, 구조 타입 정보 전달에 사용
```


### ThemeConfig 기반 Mesh 선택

건물과 도로에 사용되는 Mesh 정보는 `WorldThemeConfig` DataAsset에서 관리합니다.
```txt
WorldThemeConfig
→ 도로 Mesh 목록
→ 건물 Mesh 목록
→ StructureType 별 Mesh 설정
→ 건물 크기 / Footprint 정보
```

PCG Graph Parameter로 Static Mesh를 직접 전달하는 방식은 타입 제약이 있었고,
건물 종류가 늘어날수록 사용할 Mesh 배열을 런타임에 하나씩 전달해야 하는 문제가 있었습니다.

이를 해결하기 위해 `WorldThemeConfig`를 입력으로 받아,
`StructureType`을 기준으로 적절한 Mesh를 선택하는 Custom PCG Node를 구현했습니다.


---
## 멀티플레이 생성 결과 확인

동일한 Seed를 사용했을 때 Host와 Client가 같은 생성 흐름을 따르는지 확인했습니다.

### 확인 방식

Host와 Client 양쪽에서 생성된 일부 데이터를 디버깅 로그로 출력하여 비교했습니다.

```txt
Road / Grid / Block / Lot
→ Seed 기반으로 생성
→ 서버와 클라이언트 양쪽에서 일부 생성 결과 로그 출력
→ Road 정보, Cell / Block / Lot 생성 결과 개수가 동일하게 생성되는지 샘플 단위로 확인

Building Actor
→ 서버에서만 생성
→ Replication으로 클라이언트에게 전달
```

현재 단계에서는 생성된 전체 데이터를 자동으로 비교하는 검증 시스템은 구현하지 않았습니다.

대신 Road, Cell, Block, Lot 등 주요 생성 데이터 일부를 로그로 비교하여
동일 Seed에서 Host와 Client의 생성 흐름이 일치하는 지 확인했습니다.

Seed 결정론은 생성 재현성과 디버깅 가능성을 위한 기반으로 사용되며,
멀티플레이에서 실제 게임 플레이 Actor는 서버 권한 생성과 Replication 방식을 사용합니다.


---
## PCG / CharacterMovementComponent 충돌 문제

멀티플레이 환경에서 런타임 생성 컴포넌트와 네트워크 이동 시스템이 충돌하는 문제가 발생했습니다.

### 문제

`CharacterMovementComponent`가 PCG로 생성된 `InstancedStaticMeshComponent`를 MovementBase로 참조하면서 네트워크 보정 경고가 반복 발생했습니다.

대표적인 경고는 다음과 같습니다.

```txt
FNetGUIDCache::SupportsObjects: InstancedStaticMeshComponent ... NOT Supported
ClientAdjustPosition_Implementation could not resolve the new relative base actor
```

### 원인 분석

문제의 핵심은 네트워크 이동 보정 과정에 있습니다.

`CharacterMovementComponent`는 캐릭터가 밟고 있는 바닥을 MovementBase로 추적합니다.

서버와 클라이언트 간 이동 보정 시 이 MovementBase를 Network GUID로 참조해야 하는데,
PCG의 Spawn Static Mesh 노드를 통해 생성된 `InstancedStaticMeshComponent`는 이 참조 구조에서 안정적으로 지원되지 않았습니다.

그 결과 클라이언트 이동 보정 과정에서 MovementBase를 해석하지 못하는 문제가 발생했습니다.

### 해결
PCG 그래프 내부의 Static Mesh Spawner 노드를 Spawn Actor 노드로 교체했습니다.

이후 C++ WorldGenerator 코드에서 PCG 실행 시 서버 / 클라이언트 분기를 추가하여,
서버에서만 Building PCG가 실행되도록 구현했습니다.

그 결과 ISMC 기반 컴포넌트가 `CharacterMovementComponent`의 MovementBase로 참조되는 상황이 제거되었고, MovementBase Network GUID 경고와 클라이언트 이동 보정 문제가 해결되었습니다.

### Trade-off

Static Mesh Spawner / ISMC 기반 구조는 동일한 Mesh를 다수 배치할 때 Draw Call을 줄이고 메모리 효율을 높일 수 있습니다.

하지만 멀티플레이 환경에서는 해당 컴포넌트가 MovementBase로 참조될 때 Network GUID 해석 문제가 발생했기에,

이 프로토타입에서는 렌더링 최적화보다 멀티플레이 이동 안정성과 서버 권한 구조의 명확성을 우선했습니다.