# 코드 구조 설명

이 문서는 `Space Defence`의 코드를 처음 읽는 사람이 어디서부터 보면 되는지 정리한 안내서다. 게임잼 리포트나 코드 설명을 쓸 때도 이 흐름을 따라가면 된다.

## 전체 흐름

1. `src/main.cpp`가 Win32 창을 만들고 `SpaceDefenceGame`을 실행한다.
2. `SpaceDefenceGame`은 외부에 보이는 얇은 껍데기이고, 실제 게임 상태는 `SpaceDefenceGameImpl`이 가진다.
3. 매 프레임 `DeltaTimer`가 델타타임을 계산한다.
4. `SpaceDefenceGameImplCore.cpp`의 업데이트 루프가 입력, 전투, VFX, 사운드, 저장 상태를 갱신한다.
5. `SpaceDefenceGameImplRender.cpp`가 현재 화면 상태에 맞춰 타이틀, 메뉴, 브리핑, 전투, 결과 화면을 그린다.

## 파일별 역할

- `SpaceDefenceGameImpl.h`: 전투 유닛, 투사체, 파티클, 저장 데이터, 화면 상태 같은 핵심 구조체와 멤버 변수를 모아둔 중심 헤더다.
- `SpaceDefenceGameImplCore.cpp`: 초기화, 리소스 로딩, 저장/불러오기, 전체 프레임 업데이트, 효과음 호출을 담당한다.
- `SpaceDefenceGameImplCombat.cpp`: 유닛 이동, 타겟 탐색, 공격, 피해 처리, 스폰, 스테이지 기믹, 월렛, 문빔 캐논을 담당한다.
- `SpaceDefenceGameImplInput.cpp`: 키보드와 마우스 입력을 화면별로 해석한다.
- `SpaceDefenceGameImplLayout.cpp`: 버튼과 카드의 좌표를 한곳에서 관리한다. 입력 판정과 렌더링이 같은 좌표를 쓰게 하기 위한 파일이다.
- `SpaceDefenceGameImplRender.cpp`: Direct2D 렌더링, UI, 캐릭터, 무기, 배경, 이미지 VFX, 후처리 느낌의 빛 효과를 담당한다.
- `GameData.cpp`: 유닛, 적, 스테이지의 밸런스 수치를 가진다. 수치 조정은 되도록 이 파일에서 끝내는 것이 좋다.
- `framework/AudioManager.cpp`: FMOD가 있으면 위치음향을 쓰고, 없으면 WinMM으로 대체 재생한다.
- `framework/DeltaTimer.cpp`: 프레임마다 실제 경과 시간을 계산해 게임 속도가 PC 성능에 덜 흔들리게 한다.

## 상태 관리

`GameScreen`은 현재 화면을 나타낸다.

- `Title`: 시작 화면
- `Options`: 옵션과 저장 슬롯 관리
- `Menu`: 스테이지 선택과 편성
- `Archive`: 유닛/적/스테이지 도감
- `Shop`: 유닛 구매와 강화
- `Briefing`: 출격 전 확인 화면
- `Playing`: 실제 전투
- `Result`: 승패 결과

화면을 바꾸는 코드는 대부분 `SpaceDefenceGameImplInput.cpp`와 `SpaceDefenceGameImplCore.cpp`에 있다. 화면 전환 연출은 커튼 타이머와 사운드 이벤트를 통해 처리한다.

## 전투 구조

전투 유닛은 `Unit` 구조체로 관리한다. 핵심 값은 다음과 같다.

- `team`: 아군인지 적인지 구분한다.
- `kind`: 어떤 유닛 타입인지 나타낸다.
- `pos`: 월드 좌표다. 긴 전장을 카메라가 따라다니기 때문에 화면 좌표와 다르다.
- `hp`, `maxHp`: 현재 체력과 최대 체력이다.
- `range`, `attackDelay`, `speed`: 공격 거리, 공격 주기, 이동 속도다.
- `animState`, `stateTime`: 수동 애니메이션 상태와 상태 경과 시간이다.
- `stunTimer`, `knockbackTimer`, `hitFlash`: 피격, 넉백, 스턴 연출에 쓰인다.

전투 갱신은 크게 `이동 -> 타겟 탐색 -> 공격 준비 -> 피해 적용 -> 사망 처리` 순서로 흐른다. 원거리 유닛은 `Projectile`을 만들고, 근거리 유닛은 공격 타이밍에 바로 피해를 준다.

## 렌더링 구조

렌더링은 화면별 함수로 나뉜다.

- `DrawTitle`: 시작 화면
- `DrawMenu`: 스테이지 선택과 편성
- `DrawBriefing`: 출격 전 브리핑
- `DrawBattle`: 전투 전체
- `DrawUnits`: 유닛과 무기
- `DrawImageVfxSprites`: 이미지 시트 기반 VFX
- `DrawTelegraphs`: 스테이지 위험 예고
- `DrawStageGimmickOverlay`: 행성 이벤트 게이지와 발동 표시
- `DrawHud`: 상단/하단 전투 UI

UI 위치가 이상하면 먼저 `SpaceDefenceGameImplLayout.cpp`의 좌표 함수를 확인한다. 전투 중 월드 위치가 이상하면 `SpaceDefenceGameImplCombat.cpp`의 `pos` 계산과 `SpaceDefenceGameImplRender.cpp`의 카메라 변환을 같이 확인한다.

## 사운드 구조

효과음은 `PlaySfx`, `PlaySfxAt`, `PlayAttackSfxAt`으로 호출한다.

- `PlaySfx`: UI처럼 위치가 중요하지 않은 소리다.
- `PlaySfxAt`: 전장 X 위치에 따라 좌우 패닝과 감쇠가 들어가는 소리다.
- `PlayAttackSfxAt`: 유닛 종류별 공격음을 따로 스로틀링해서 여러 유닛이 동시에 싸워도 소리가 덜 잘리게 한다.

FMOD 빌드에서는 3D 채널을 사용하고, 기본 빌드에서는 WinMM 대체 재생을 쓴다. 동시 재생 품질은 FMOD 빌드가 더 안정적이다.

## 새 기능을 추가할 때

1. 밸런스 수치가 필요하면 `GameData.cpp`에 먼저 추가한다.
2. 전투 규칙이면 `SpaceDefenceGameImplCombat.cpp`에 구현한다.
3. 클릭 영역이 필요하면 `SpaceDefenceGameImplLayout.cpp`에 좌표 함수를 만든다.
4. 화면 표시가 필요하면 `SpaceDefenceGameImplRender.cpp`에 렌더 함수를 추가한다.
5. 저장되어야 하는 값이면 저장/로드 함수에 버전 호환을 고려해서 넣는다.
6. 마지막으로 `build.ps1`을 실행하고, 타이틀/메뉴/브리핑/전투/결과 화면을 직접 확인한다.

## QA 체크리스트

- 타이틀, 옵션, 메뉴, 브리핑, 전투, 결과 화면에서 글씨가 버튼이나 게이지를 가리지 않는가?
- 전체 화면과 창 모드 모두에서 검은 오버레이가 화면 전체를 덮는가?
- 유닛 카드, 월렛, 문빔, 속도 버튼을 눌렀을 때 눌림 피드백이 보이는가?
- 근거리와 원거리 유닛의 무기 방향이 팀에 맞게 보이는가?
- 보스는 조건에 맞게 한 번 등장하고, 등장 시 아군 스턴/넉백이 너무 과하지 않은가?
- 스테이지 기믹은 예고, 발동, 피해/효과가 모두 눈에 보이는가?
- 효과음 볼륨 조절이 옵션과 전투 중 모두 반영되는가?
- 저장, 불러오기, 삭제가 슬롯별로 동작하는가?
