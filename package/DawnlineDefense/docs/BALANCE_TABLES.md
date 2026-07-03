# Balance Tables

This document mirrors the runtime values in `src/GameData.cpp` so the stage curve can be reviewed without reading C++ switch logic.

## Stage Curve

| Stage | Player HP | Enemy HP | Start Energy | Enemy Interval | Threat | First Boss | Main Gimmick |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| Mercury | 2200 | 2500 | 190 | 2.36 | 0.96 | 42s | Heat wave area damage |
| Venus | 2250 | 2850 | 205 | 2.18 | 1.05 | 39s | Acid fog and ranged penalty |
| Earth | 2350 | 3200 | 220 | 2.05 | 1.14 | 36s | Supply bloom healing |
| Mars | 2400 | 3550 | 230 | 1.98 | 1.24 | 33s | Meteor impact |
| Jupiter | 2600 | 4200 | 250 | 1.90 | 1.36 | 30s | Gravity surge |
| Saturn | 2650 | 4650 | 260 | 1.78 | 1.48 | 28s | Ring reinforcement |
| Uranus | 2750 | 5200 | 275 | 1.68 | 1.62 | 25s | Ice gust |
| Neptune | 2850 | 5850 | 290 | 1.56 | 1.78 | 23s | Tide surge |
| Pluto | 3000 | 6600 | 310 | 1.50 | 1.96 | 18s | Void eclipse |
| Sun | 3300 | 7600 | 350 | 1.38 | 2.22 | 15s | Solar flare and boss pressure |

## Notes

- `Enemy Interval` decreases across the campaign so later stages feel denser.
- `Threat` scales enemy stats and boss cadence.
- `First Boss` gets earlier in later stages to make the final planets feel more urgent.
- Gimmicks are implemented in `UpdateStageGimmicks()` and `TriggerStageGimmick()` in `src/PawlineGameImplCombat.cpp`.

## Difficulty

| Difficulty | Player Base | Enemy Base | Threat | Spawn Interval | Reward |
| --- | ---: | ---: | ---: | ---: | ---: |
| Easy | 116% | 88% | 84% | 115% | 88% |
| Normal | 100% | 100% | 100% | 100% | 100% |
| Hard | 92% | 118% | 122% | 86% | 128% |

## Briefing Balance Read

The briefing screen compares the selected stage threat and current loadout power before launch.

- Stage threat uses enemy base HP, enemy interval, threat scale, first boss timing, and difficulty.
- Loadout power uses unit HP, damage, range, speed, attack cadence, cost, level, and active synergies.
- The panel recommends shop upgrades when power is low, normal launch when values are close, and hard difficulty when power is clearly above the stage curve.

## AI Director

During combat, the enemy director watches lane state and base HP.

- If player units and player base HP are strongly ahead, spawn intervals tighten slightly and rare elite spawns can appear.
- If the player base is in danger, spawn intervals loosen slightly so the run is recoverable.
- The multiplier is intentionally small, between 0.84x and 1.16x, so the authored stage curve still leads the balance.

## Loadout Synergy

| Synergy | Required Units | Effect |
| --- | --- | --- |
| Guard Wall | 방패냥 + 얼음방패냥 | Front-line HP +10% |
| Arc Focus | 전기냥 + 프리즘냥 | Ranged damage +8% |
| Rush Pack | 질주냥 + 혜성냥 | Rush speed/damage +7% |
| Star Scope | 궤도냥 + 성운포냥 | Ranged range +6% |
| Mint Supply | 지원냥 | Energy regen +4 and stronger wallet healing |
| Sun Chime | 태양검냥 + 종냥 | Cannon charge +12% |

## Evolution

| Requirement | Effect |
| --- | --- |
| Unit reaches Lv.5 | Name gains `진화`, HP +10%, damage +12%, speed +4%, cooldown -6% |
| Ranged evolved unit | Additional range +6% |
| Combat presentation | Crown-like light ring and stronger local attack glow |

## Boss Pattern Identity

| Boss | Signature Pattern |
| --- | --- |
| 철갑병 | 압축 파동 circle warning |
| 산성사수 | 산성 장막 lane warning |
| 포자병 | 포자 증식 reinforcement |
| 녹슨망치 | 붉은 낙하 multi-meteor |
| 중력방패 | 중력 소용돌이 |
| 고리사수 | 고리탄 소환 and line thrust |
| 얼음러너 | 얼음 가름 line warning |
| 해류사수 | 심해 밀물 lane push |
| 지진돌격 | 공허 균열 |
| 태양문지기 | 삼중 플레어 |
