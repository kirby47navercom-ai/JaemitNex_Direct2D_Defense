# Asset Notes

이 폴더의 에셋은 과제 기간 중 프로젝트에 맞춰 직접 생성했거나, 무료/CC0 범위의 자료를 확인해서 사용한 것이다.

## 배경과 컷인

- `backgrounds/stage_00_space_hd.jpg` ~ `stage_09_space_hd.jpg`
  - 스테이지별 고화질 우주 배경 레이어.
  - `tools/generate_quality_assets.py`로 직접 생성한다.
  - 전투 무대는 행성 표면처럼 보이게 하고, 뒤쪽은 실제 우주 공간처럼 보이도록 분리해서 사용한다.
- `backgrounds/stage_00_space.png` ~ `stage_09_space.png`
  - 이전 버전의 스테이지 배경 레이어.
  - 현재 게임에서는 HD JPG 배경을 우선 사용한다.
- `cutins/prologue_story_art.jpg`
  - 프롤로그 스토리 화면 배경 아트.
- `cutins/ending_solar_route.jpg`
  - 태양 클리어 뒤 엔딩 화면 배경 아트.
- `cutins/solar_gatekeeper_cutin.png`
  - 보스 등장 배너에 쓰는 컷인 이미지.

## 캐릭터 스프라이트

- `sprites/kenney_toon_units/player_unit_atlas.png`
- `sprites/kenney_toon_units/enemy_unit_atlas.png`
- 원본: Kenney Toon Characters
- 출처: https://kenney.nl/assets/toon-characters
- 라이선스: Creative Commons CC0
- 적용 방식: 원본 PNG를 128x128 단위 아틀라스로 재구성하고, 유닛 종류별 역할 차이가 보이도록 보정했다.

## 백업 스프라이트

- `sprites/gameart2d_catdog/*`
- 원본: GameArt2D Cat and Dog Free Sprites
- 출처: https://www.gameart2d.com/cat-and-dog-free-sprites.html
- 현재 메인 렌더링은 Kenney 통합 아틀라스를 사용하며, 이 폴더는 백업/참고용으로 남겨두었다.

## 전투 VFX

- `vfx/*.png`
  - 게임에서 실제로 로드하는 전투 이펙트 시트.
  - 사용자가 제공한 이펙트와 외부 무료 에셋 기반 시트를 우선 사용한다.
  - 프레임 크기와 열/행 구조는 코드의 `ImageVfxSpec`과 맞춰야 한다.
- `vfx/source_effects/`
  - 사용자가 제공한 원본 이펙트와 참고용 시트 모음.
- `tools/generate_quality_assets.py`
  - 기본 실행에서는 VFX를 덮어쓰지 않는다.
  - `REGENERATE_VFX = True`로 바꾸면 절차형 VFX 시트가 다시 생성되므로 주의해야 한다.

## 무기

- `weapons/external_roles/*.png`
  - 플레이어/적 유닛 역할별 무기 이미지.
  - 유닛 손 위치 기준으로 회전/이동하며 수동 공격 애니메이션에 연결된다.
- `weapons/kenney_blaster/*`
  - Kenney 무기 에셋 백업/참고 자료.

## UI

- `ui/pawline_ui_atlas.png`
  - 버튼과 패널 질감을 위한 2x2 UI 아틀라스.
  - 256x128 타일 4개 구조를 사용한다.

## 사운드와 음악

- `sfx/events/*.wav`, `sfx/kenney/*.wav`
  - UI, 전투, 유닛별 공격 효과음.
- `music/stage_*.wav`
  - 수성부터 태양까지 스테이지별 전투 BGM.
  - 각 파일은 서로 다른 시드와 음계로 생성되어 같은 곡의 복사본이 아니다.
- `music/outer_space_loop.wav`
  - 메인 메뉴/스토리용 BGM.
- `music/layer_danger.wav`
  - 전투 위험도가 높아질 때 메인 BGM 위에 얹는 보조 레이어.
- 음악 파일은 `tools/generate_quality_assets.py`로 직접 생성한 절차적 오디오이며, 외부 샘플을 포함하지 않는다.

## 폰트

- `fonts/Galmuri11.ttf`
  - 점자처럼 보이는 영문 UI 텍스트에 사용한다.
  - 라이선스: SIL Open Font License 1.1
- `fonts/Gyeonggi*.otf`
  - 일반 한글 UI 텍스트에 사용한다.
