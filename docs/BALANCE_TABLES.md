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

## Loadout Synergy

| Synergy | Required Units | Effect |
| --- | --- | --- |
| Guard Wall | Box Guard + Frost Guard | Front-line HP +10% |
| Arc Focus | Spark Nya + Prism Snipe | Ranged damage +8% |
| Rush Pack | Dash Kit + Comet Ace | Rush speed/damage +7% |
| Star Scope | Orbit Mage + Nebula Queen | Ranged range +6% |
| Mint Supply | Mint Medic | Energy regen +4 and stronger wallet healing |
| Sun Chime | Solar Brave + Bell Oracle | Cannon charge +12% |
