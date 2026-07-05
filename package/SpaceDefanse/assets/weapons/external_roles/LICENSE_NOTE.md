# External Role Weapons

전투 무기는 직접 그린 임시 PNG가 아니라 무료 외부 에셋을 게임 규격에 맞게 가공해 사용합니다.

- RgsDev, Free CC0 Melee Weapon Vector Sprites, CC0: https://opengameart.org/content/free-cc0-melee-weapon-vector-sprites
- Tom L, Weapon icons, CC0: https://opengameart.org/content/weapon-icons-0
- zeroisnotnull, Shield Sprite, CC0: https://opengameart.org/content/shield-sprite
- Kenney, Blaster Kit, Creative Commons CC0: https://kenney.nl/assets/blaster-kit

가공 내용: 원본 PNG를 오른쪽 공격 방향 기준으로 회전, 크기 조정, 색 보정하고 `192x128` 투명 캔버스에 배치했습니다.
게임 안에서는 적 유닛이 `attackDir = -1`과 좌우 반전 렌더링을 사용해 플레이어 쪽으로 무기를 들도록 처리합니다.
