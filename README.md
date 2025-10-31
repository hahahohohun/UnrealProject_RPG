# ProjectClass (Unreal Portfolio)

[![Unreal Engine](https://img.shields.io/badge/Unreal-5.x-0E1128?logo=unrealengine)](#)  
**전투/AI/보스 중심 언리얼 포트폴리오**입니다. Control Rig 발바닥 IK, AI Perception(시각/청각), 락온, 암살/백스탭, 스태미나·상태이상, 히트 리액션, 보스 패턴(프로젝타일/광역기) 등 실전형 콘텐츠 설계를 담았습니다.

//> 🎬 **데모 영상**:  
> 🖼️ **GIF**: 

---

## 📘 목차
- [주요 기능](#주요-기능)
- [시연 GIF](#시연-gif)
- [프로젝트 구조](#프로젝트-구조)
- [주요 시스템 설계](#주요-시스템-설계)

---

## ⚔️ 주요 기능
- **전투 코어**
- 
- **전투 코어**
  - 락온 타깃 선택(시야각 기반), 콤보/구르기/가드, **스태미나 시스템**(소모/지연/재생)
  - **히트 리액션**(일반/CC 구분, Overlay 머테리얼 중복 방지)
  - **데미지 플로터**(월드→뷰포트 변환, 클램프·패딩, 페이드/상승 애니메이션)
- **AI**
  - **AI Perception** 시각(각도/거리), 청각, 피해 인지
  - **BT Decorator/Task** 커스텀 (예: 거리 체크, **대시백** 동작, 몽타주 완료 델리게이트 바인딩)
  - 상태 머신: Patrol → Investigating → Battle → CrowdControlled → SkillUsing → Dead
- **보스 패턴**
  - 원형 낙하/다중 발사 등 **프로젝타일 스킬** (환경 충돌/피격 충돌 분리)
  - **카메라 셰이크** & **타임 딜레이션**(Notify 연동)
- **애니메이션/IK**
  - **Control Rig 발바닥 IK**(지면 트레이스, 본 Transform 보정)
  - 암살/백스탭 몽타주, 방향각도 기반 이동 애니메이션 스위칭
- **상태이상/버프**
  - `StatusEffectTable` 기반 **가산/곱연산** 모드 구분, 버프형/비버프형 필터
  - HUD 아이콘 동적 정렬/숨김 처리
- **UI 인프라**
  - `UGameInstanceSubsystem` 기반 UI 생성/수명 관리
  - **UISubsystem → DamageFloater 생성** 등 공통 접근 지점 제공

---

## 🎥 시연 GIF

| 기능 | 미리보기 |
|------|-----------|
| 락온 & 각도 타깃팅 | Docs/gifs/lockon.gif |
| Control Rig 발바닥 IK | Docs/gifs/ik.gif |
| 데미지 플로터 | Docs/gifs/damage_floater.gif |
| AI 대시백 | Docs/gifs/dashback.gif |
| 암살/백스탭 | Docs/gifs/assassination.gif |
| 보스 패턴(원형 낙하) | Docs/gifs/boss_rain.gif |

---

## 🧩 기술 스택
- **Engine**: Unreal Engine 5.x (예: 5.4)
- **Language**: C++20 / Blueprint
- **Animation**: Control Rig, Montage, Notify
- **AI**: Behavior Tree, AI Perception
- **FX**: Niagara / Camera Shake
- **UI**: UMG, UGameInstanceSubsystem
- **툴링**: Rider / Visual Studio / Git LFS

---
## 📂 소스 코드 구조

<details>
<summary>펼쳐보기 / 접기</summary>

```plaintext
D:.
│  PC.Target.cs
│  PCEditor.Target.cs
│  
└─PC
    │  PC.Build.cs
    │  PC.cpp
    │  PC.h
    │  PC_Enum.h
    │  
    ├─AI
    │  │  PC_AIController.cpp
    │  │  PC_AIController.h
    │  │  PC_BTDecorator_CheckRange.cpp
    │  │  PC_BTDecorator_CheckRange.h
    │  │  PC_BTTask_DashBack.cpp
    │  │  PC_BTTask_DashBack.h
    │  └─Actor
    │          PC_PatrolRoute.cpp
    │          PC_PatrolRoute.h
    │          
    ├─Animation
    │      PC_AttackTraceNotify.cpp
    │      PC_AttackTraceNotify.h
    │      PC_CamShakeNotify.cpp
    │      PC_CamShakeNotify.h
    │      
    ├─Character
    │  │  PC_BaseCharacter.cpp
    │  │  PC_BaseCharacter.h
    │  │  PC_PlayableCharaceter.cpp
    │  │  PC_PlayableCharaceter.h
    │  │  
    │  ├─Component
    │  │      PC_ActionComponent.cpp
    │  │      PC_ActionComponent.h
    │  │      PC_BattleComponent.cpp
    │  │      PC_BattleComponent.h
    │  │      PC_StatusEffectComponent.cpp
    │  │      PC_StatusEffectComponent.h
    │  │      
    │  └─Controller
    │          PC_PlayerController.cpp
    │          PC_PlayerController.h
    │          
    ├─Data
    │      PC_PlayerDataAsset.h
    │      PC_TableRows.h
    │      
    ├─Interface
    │      PC_CharacterInterface.h
    │      PC_CharacterWidgetInterface.h
    │      
    ├─Subsystem
    │      PC_DataSubsystem.cpp
    │      PC_DataSubsystem.h
    │      PC_UISubsystem.cpp
    │      PC_UISubsystem.h
    │      
    ├─UI
    │      PC_DamageFloaterWidget.cpp
    │      PC_DamageFloaterWidget.h
    │      PC_StatusEffectWidget.cpp
    │      PC_StatusEffectWidget.h
    │      
    └─Utills
            PC_GameUtill.cpp
            PC_GameUtill.h


