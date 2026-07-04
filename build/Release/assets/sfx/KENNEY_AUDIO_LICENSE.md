# Kenney Audio Asset Notice

이 폴더의 WAV 효과음은 Kenney의 무료 CC0 음원 팩을 게임용으로 변환하거나 이름을 정리해 사용한 파일입니다.

- Kenney Sci-fi Sounds: https://kenney.nl/assets/sci-fi-sounds
- Kenney Digital Audio: https://kenney.nl/assets/digital-audio
- Kenney Impact Sounds: https://kenney.nl/assets/impact-sounds
- Kenney Interface Sounds: https://kenney.nl/assets/interface-sounds
- Kenney RPG Audio: https://kenney.nl/assets/rpg-audio
- License: Creative Commons Zero, CC0

## 사용 위치

- `kenney/*.wav`: 유닛과 적의 공격 효과음입니다.
- `events/*.wav`: UI, 소환, 피격, 기지 타격, 보스 등장, 장면 전환, 스테이지 클리어 효과음입니다.
- 루트의 기존 WAV 파일: 이전 빌드 호환을 위해 남긴 기본 효과음입니다.

## 편집한 파일

- `kenney/player_comet_short.wav`
- `kenney/enemy_comet_short.wav`
- `kenney/enemy_spore_short.wav`

위 세 파일은 긴 원본 공격음이 전투 중 과하게 겹치지 않도록 같은 CC0 원본에서 앞부분을 잘라내고 짧은 페이드아웃을 적용한 게임용 파생 파일입니다.

WAV 변환은 Windows 기본 빌드의 `PlaySoundW` 호환성과 FMOD Core API 재생 안정성을 위해 진행했습니다.
