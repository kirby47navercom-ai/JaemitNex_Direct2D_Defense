#pragma once

#include <d2d1.h>
#include <string>

constexpr int kLoadoutSize = 5;
constexpr int kRosterCount = 14;
constexpr int kStageCount = 10;

enum class PlayerUnit
{
    Paw = 0,
    Box = 1,
    Spark = 2,
    Dash = 3,
    Bell = 4,
    Titan = 5,
    Frost = 6,
    Comet = 7,
    Orbit = 8,
    Solar = 9,
    Mint = 10,
    Drill = 11,
    Prism = 12,
    Nebula = 13
};

enum class EnemyUnit
{
    Dust = 0,
    Brute = 1,
    Skitter = 2,
    Sulfur = 3,
    Moss = 4,
    Rust = 5,
    Storm = 6,
    Ring = 7,
    Frost = 8,
    Tide = 9,
    Void = 10,
    Flare = 11,
    Spore = 12,
    Quake = 13,
    Mirror = 14,
    Comet = 15,
    Boss = 16
};

struct UnitStats
{
    std::wstring name;
    int cost = 0;
    int reward = 0;
    float cooldown = 1.0f;
    float hp = 1.0f;
    float damage = 1.0f;
    float range = 24.0f;
    float attackDelay = 1.0f;
    float speed = 40.0f;
    float radius = 16.0f;
    bool ranged = false;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
    D2D1_COLOR_F accent = D2D1::ColorF(0xFFFFFF);
};

struct StageDefinition
{
    std::wstring name;
    std::wstring subtitle;
    std::wstring gimmick;
    float playerHp = 2200.0f;
    float enemyHp = 2600.0f;
    float startEnergy = 180.0f;
    float enemyInterval = 2.32f;
    float threatScale = 1.0f;
    float bossFirstTime = 38.0f;
    D2D1_COLOR_F backColor = D2D1::ColorF(0x0D1B23);
    D2D1_COLOR_F laneColor = D2D1::ColorF(0x142935);
    D2D1_COLOR_F laneInnerColor = D2D1::ColorF(0x10212D);
    D2D1_COLOR_F lineColor = D2D1::ColorF(0x4A6272);
};

StageDefinition GetStageDefinition(int index);
UnitStats GetPlayerStats(PlayerUnit unit);
UnitStats GetEnemyStats(EnemyUnit unit, float threat);
