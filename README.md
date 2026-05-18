# RWG (Random World Generator)

Unreal Engine 5.6 C++ 기반 프로젝트입니다.
절차적 도시 생성과 멀티플레이 게임플레이를 중심으로 구성됩니다.


## 프로젝트 개요
이 프로젝트는 고립된 도시를 배경으로 한 4인 서바이벌 / 탈출 게임입니다.

플레이어는 Seed 기반으로 생성된 도시에 진입해 자원을 수집하고 위험 요소를 피해 탈출을 목표로 합니다.

현재는 랜덤 월드 생성, 멀티플레이 세션, 게임플레이 기반 시스템을 중심으로 프로토타입을 구축하고 있습니다.


## 핵심 기능

### 랜덤 월드 생성
- Seed 기반 결정론적 생성
- 런타임 지형 생성
- 도로 그래프 생성 및 Spline 시각화
- Cell Grid 기반 도시 블록 탐지
- 블록 기반 건물 배치
- PCG 기반 도로 및 건물 콘텐츠 생성

### 멀티플레이
- 세션 생성 / 검색 / 참가 흐름
- Online Subsystem Null 기반 Listen Server
- SessionLevel → LobbyLevel → GameplayLevel 전환
- ServerTravel / ClientTravel 처리
- GameState / PlayerState 기반 로비 데이터 동기화

### 게임플레이
- 컴포넌트 기반 입력 구조
- 상호작용 시스템
- 아이템 및 인벤토리 시스템
- 무기 계층 구조
- 장착 / 해제 흐름
- 장전 및 발사 로직
- UI 연동 흐름

## 문서
상세 문서는 '/docs' 폴더에서 관리합니다.

- [Gameplay](docs/Gameplay.md)
- [Random World Generation](docs/RandomWorldGeneration.md)
- [Session](docs/Session.md)


## 기술 스택
- Unreal Engine 5.6
- C++
- PCG Framework
- Procedural Mesh Component
- Online Subsystem Null


## 현재 상태

현재 프로토타입 / 시스템 구축 단계입니다.

구현 완료 또는 부분 구현된 시스템:
- 랜덤 월드 생성 시스템
- 멀티플레이 세션 및 로비 ↔ 게임 플레이 흐름 구현
- 입력 / 상호작용 / 인벤토리 / 무기 구조 구현
- UI / HUD 등 UserWidget 연동 흐름 구조 구현 중