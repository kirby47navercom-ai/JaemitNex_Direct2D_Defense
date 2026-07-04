# Kenney Audio Asset Notice

이 폴더의 WAV 효과음은 Kenney의 무료 CC0 음원 팩에서 가져온 원본을 게임용으로 변환하거나 이름을 정리해 사용합니다.

- Kenney Sci-fi Sounds: https://kenney.nl/assets/sci-fi-sounds
- Kenney Digital Audio: https://kenney.nl/assets/digital-audio
- Kenney Impact Sounds: https://kenney.nl/assets/impact-sounds
- Kenney Interface Sounds: https://kenney.nl/assets/interface-sounds
- Kenney RPG Audio: https://kenney.nl/assets/rpg-audio
- License: Creative Commons Zero, CC0

## 사용 위치

- `kenney/*.wav`: 유닛별 기본 공격음
- `events/*.wav`: UI, 소환, 타격, 탄착, 기지 피격, 보스 등장, 장면 전환, 스테이지 클리어 효과음
- 루트의 기존 WAV 파일: 이전 빌드 호환용 기본 효과음

WAV 변환은 Windows 기본 빌드의 `PlaySoundW` 호환성과 FMOD Core API 재생 안정성을 위해 진행했습니다.
