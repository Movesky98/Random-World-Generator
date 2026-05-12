## 개요
Gameplay 시스템은 컴포넌트 기반 책임 분리를 중심으로 설계되었습니다.

Character나 PlayerController에 모든 로직을 집중시키는 대신, 입력 / 상호작용 / 인벤토리 / 전투 / UI / Locomotion 처리를 각각 독립된 컴포넌트로 분리했습니다.

전체 시스템은 **클라이언트는 요청하고, 서버는 판단한다**는 멀티플레이 권한 원칙을 기준으로 구성됩니다.

## 주요 시스템
- Input System
- Interaction System
- Item / Inventory System
- Weapon System
- Combat System
- UI System

---
## Input System

플레이어 입력은 'PlayerController'에 부착된 'InputHandlerComponent'를 통해 초기화 됩니다.

### 구조

```txt
Enhanced Input
→ InputHandlerComponent (PlayerController 소유)
→ LocomotionComponent / CombatComponent / InteractionComponent / InventoryComponent
(Character 소유) 
(BaseInputComponent의 파생 클래스, IInputBindable 구현)

```

### 구현 포인트
`InputHandlerComponent`는 Possess 시점에 `IInputBindable` 인터페이스를 이용해,
입력 바인딩이 필요한 컴포넌트들을 수집합니다.

이후 `InputHandlerComponent`는 각 컴포넌트가 입력을 바인딩할 수 있도록
`PlayerController`의 `InputComponent`를 전달합니다.

현재는 `PlayerController::AcknowledgePossess()` 시점에서 이 수집 및 전달 과정을 수행합니다.


또, 각 입력 처리 컴포넌트는 `InitializeComponent()` 시점에
`AssetManager`를 통해 자신에게 필요한 DA_InputConfig를 비동기 로드합니다.

어떤 Config를 로드할지는 각 컴포넌트가 GetConfigClass()를 오버라이드해서 직접 명시합니다.

```cpp
TSubclassOf<UBaseInputConfig> UInteractionComponent::GetConfigClass()
{
    return UInteractionInputConfig::StaticClass();
}
```

### 설계 판단
입력 처리 책임을 `InputHandlerComponent`에 집중하지 않고 
실제 입력이 필요한 각 기능 컴포넌트가 직접 자신의 입력을 바인딩하도록 구성했습니다.

`InputHandlerComponent`는 입력 대상 컴포넌트를 찾고, 각 컴포넌트가 입력을 바인딩할 수 있도록 `PlayerController의 InputComponent`를 전달하는 관리자 역할을 담당합니다.

이를 통해 `Character`에 필요한 입력 컴포넌트만 선택적으로 부착할 수 있으며,
Character를 상속한 다른 클래스에서도 BaseInputComponent 기반의 동일한 입력 바인딩 구조를 재사용할 수 있습니다.

---
## Interaction System

Interaction System은 플레이어가 월드 오브젝트와 상호작용할 수 있도록 처리하는 시스템입니다.

### 흐름

```txt
클라이언트 입력
→ Interaction Trace
→ Server_RequestInteract()
→ 서버 검증 및 실행
→ Inventory / 게임 상태 갱신
```

### 구현 포인트
상호작용 대상 탐지는 클라이언트에서 LineTrace를 통해 수행할 수 있지만, 실제 상호작용 결과는 서버에서 처리합니다.

`AItem::Interact()`는 클라이언트에서 자유롭게 호출되는 함수가 아니라 서버 내부에서 실행되는 게임플레이 처리 함수로 사용됩니다.

### 설계 판단
아이템 획득이나 상태 변경은 멀티플레이 환경에서 권한 문제가 발생하기 쉽습니다.

따라서 클라이언트는 상호작용을 요청하고, 서버가 최종적으로 검증 및 실행하는 구조를 사용했습니다.

---
## Item / Inventory System

아이템 시스템은 월드에 존재하는 Actor, 아이템 정의를 담는 DataAsset, 
인벤토리에 저장되는 런타임 상태 데이터로 분리하여 관리합니다.

### 클래스 구조

```txt
AItem // 월드에 배치되는 아이템
└─ AWeaponBase  
	├─ AGunBase  
	├─ AMeleeWeaponBase  
	└─ AGrenade

UItemData // 이 아이템이 무엇인가를 정의하는 데이터
└─ UWeaponData
	├─ UGunData
	├─ UMeleeWeaponData
	└─ UGrenadeData

FItemInstance // 인벤토리에 저장되는 아이템의 런타임 상태 담당.
```

### 아이템 데이터 분리

```txt
AItem
- 월드에 존재하는 아이템 Actor
- Mesh, Collision, Interaction 처리
- 플레이어가 월드에서 직접 상호작용하는 대상
  
UItemData
- 아이템의 정적 정의 데이터
- 이름, 아이콘, 설명, 최대 스택 수, 기본 스탯 등
- 여러 아이템 인스턴스가 공유할 수 있는 설계 데이터
  
FItemInstance
- 인벤토리에 저장되는 아이템의 런타임 상태
- 어떤 UItemData를 기반으로 하는지 참조
- 수량으로 개별 아이템 슬롯의 상태를 표현
```

### 인벤토리 역할
- 일반 아이템 슬롯 관리
- 무기 슬롯 관리
- 아이템 획득 처리
- 소유 클라이언트 인벤토리 데이터 복제
- 인벤토리 변경 시 알림
- 월드의 AItem을 UItemInstance 형태로 변환하여 보관

### 구현 포인트
일반 아이템 `Slots`는 `COND_OwnerOnly` 조건으로 복제합니다.
이는 개인 인벤토리 데이터가 다른 클라이언트에게 불필요하게 노출되지 않도록 하기 위한 선택입니다.

무기 참조 배열 `Weapons`는 일반 Replication으로 복제합니다.
무기는 월드에 존재하고, 장착 상태가 다른 클라이언트에게도 보여야 하기 때문입니다.

### 설계 판단
이전 프로젝트에서는 아이템 Actor 자체를 인벤토리에 보관하는 방식으로 구현했습니다.

하지만 Actor는 월드에 존재하기 위한 객체이기 때문에 Mesh, Collision, Trnasform 등
월드 표현과 관련된 정보를 함께 가지고 있습니다.

인벤토리는 아이템의 월드 표현보다, 어떤 아이템을 가지고 있는 지와 해당 아이템의 개별 상태를 관리하는 것이 핵심입니다.

따라서, 아이템 Actor 자체를 인벤토리에 저장하는 방식은 아이템 수가 많아질수록 메모리 사용량과 객체 관리 부담이 커질 수 있다고 판단했습니다.

이를 해결하기 위해 아이템의 시스템을 위와 같이 분리했습니다.

---
## Weapon System과 Combat System

이 섹션은 현재 구현 중에 있으며, 무기 장착 / 재장전 / 발사 까지는 구현했으나

공격 판정 / 네트워크 동기화 / 해제 흐름이 정리된 이후 문서화할 예정입니다.

---
## UI System

UI는 `PlayerController`에 부착된 `UIManagerComponent`를 통해 관리합니다.

### 구조
```txt
Current Level
→ GamePhase
→ Widget
```

`UIManagerComponent`는 현재 레벨을 기준으로 `GamePhase`를 판단하고
해당 `GamePhase`에 연결된 Widget을 생성하거나 표시합니다.

예시 구조는 다음과 같습니다.
```txt
LV_Session
→ EGamePhase::Session
→ SessionMenu

LV_Lobby
→ EGamePhase::Lobby
→ LobbyMenu

LV_RandomWorldGeneration
→ EGamePhase::Gameplay
→ PlayerHUD / Inventory / GameOverResult(추가 예정)
```

### 구현 포인트
`UIManagerComponent`는 Blueprint에서 Level Asset과 Phase의 관계를 설정할 수 있도록 구성했습니다.

```txt
LevelToPhaseMap
- Level Asset과 GamePhase를 연결하는 설정용 Map

LevelNameToPhaseMap
- 런타임에서 현재 LevelName을 기준으로 GamePhase를 찾기 위한 Map
```

`LevelToPhaseMap`은 에디터에서 레벨 에셋과 `GamePhase`를 연결하기 위한 데이터입니다.

하지만 런타임에서는 현재 월드의 레벨 이름을 기준으로 찾아야 하므로,
`InitailizeComponent()` 시점에 `LevelToPhaseMap`을 기반으로 `LevelNameToPhaseMap` 을 생성합니다.

이를 통해 레벨 전환 이후 현재 레벨 이름을 기준으로 GamePhase를 찾고,
해당 Phase에 등록된 Widget Class를 관리할 수 있도록 구성했습니다.

#### Blueprint 설정 예시
1. Level To Phase Map 
![[UIManagerComponent_LevelToPhaseMap.png|500]]

2. Phase To Widget Class Map 
![[UIManagerComponent_WidgetClassMap.png|500]]

### 설계 판단
초기에는 `PlayerController`에서 레벨 별 UI를 직접 생성하고 관리했습니다.

하지만 이 방식은 UI 종류가 늘어날수록 `PlayerController`가 너무 많은 책임을 가지게 됩니다.
```txt
PlayerController
- 현재 레벨 확인
- 어떤 UI를 띄울 지 판단
- Widget 생성
...
```

이 구조는 기능이 추가될 수록 `PlayerController`가 UI 관리 코드로 비대해질 가능성이 있습니다.

UI 관리 책임을 `UIManagerComponent`로 분리하고,
`PlayerController`는 UI 시스템을 소유하나, 직접적인 UI 생성과 표시 판단은 담당하지 않도록 구성했습니다.

이를 통해 새로운 레벨이나 Phase가 추가되어도
PlayerController 코드를 수정하지 않고 Blueprint 설정만으로 UI 흐름을 확장할 수 있도록 하였습니다.