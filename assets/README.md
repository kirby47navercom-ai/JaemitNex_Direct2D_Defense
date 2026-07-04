# Asset Notes

이 폴더의 PNG, WAV, 문서 에셋은 과제 기간 중 프로젝트에 맞게 직접 생성하거나, 무료/CC0 범위의 외부 에셋을 재가공해 사용한다.

## 캐릭터 스프라이트

- `sprites/kenney_toon_units/player_unit_atlas.png`
- `sprites/kenney_toon_units/enemy_unit_atlas.png`
- 원본: Kenney Toon Characters
- 출처: https://kenney.nl/assets/toon-characters
- 라이선스: Creative Commons CC0
- 적용 방식: 원본 포즈 PNG를 128x128 셀, 10열 프레임, 유닛별 7행 모션 구조로 재배치했다. 유닛별 색 보정과 역할 표식은 생성 단계에서 스프라이트 자체에 합성했다. 적 아틀라스는 게임 안에서 아군을 바라보도록 좌우 반전된 파일로 생성했다.

## 예전 백업 스프라이트

- `sprites/gameart2d_catdog/*`
- 원본: GameArt2D Cat and Dog Free Sprites
- 출처: https://www.gameart2d.com/cat-and-dog-free-sprites.html
- 라이선스: GameArt2D Freebies 라이선스 페이지 기준 CC0
- 현재 메인 렌더는 Kenney 통합 아틀라스를 사용하며, 이 폴더는 백업/비교용으로 남겨 둔다.

## 전투 VFX

- `vfx/*.png`: 전투 중 실제로 로드하는 이펙트 시트
- `vfx/source_effects/`: 사용자가 제공한 `effect` 폴더의 PNG 및 RAR 내부 PNG를 다시 풀어 둔 원본 소스 모음
- 게임 적용 방식: 유닛/투사체 종류별로 `ImageVfxKind`를 매핑해 slash, heal, fire, ice, thunder, water, dark, acid, earth, smoke, holy, wind, thrust, explosion 계열 이펙트를 다르게 재생한다.

## UI, 배경, 컷인

- `backgrounds/stage_00_space.png` ~ `stage_09_space.png`: 행성별 우주 배경 레이어
- `ui/pawline_ui_atlas.png`: 버튼/패널 질감 UI 아틀라스
- `cutins/solar_gatekeeper_cutin.png`: 보스 등장 컷인 배너

## 사운드

- `sfx/spawn.wav`, `hit.wav`, `shoot.wav`, `upgrade.wav`, `clear.wav`
- 외부 음원 없이 프로젝트에서 직접 합성한 짧은 PCM WAV 효과음이다.

## 폰트

- `fonts/Galmuri11.ttf`
- `fonts/Galmuri-OFL.md`
- Galmuri는 SIL Open Font License 1.1로 배포되는 무료 폰트이며, Reserved Font Name은 `Galmuri`다.
