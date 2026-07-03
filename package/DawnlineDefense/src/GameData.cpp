#include "GameData.h"

#include <algorithm>
#include <array>

StageDefinition GetStageDefinition(int index)
{
    static const std::array<StageDefinition, kStageCount> kStageTable = {{
        {L"수성", L"가까운 첫 궤도", L"먼지형과 침형 적",
         2200.0f, 2500.0f, 190.0f, 2.36f, 0.96f, 42.0f,
         D2D1::ColorF(0x101B22), D2D1::ColorF(0x1C3037), D2D1::ColorF(0x13262D), D2D1::ColorF(0x8A9EA8)},
        {L"금성", L"짙은 대기권", L"황산 안개 원거리 적",
         2250.0f, 2850.0f, 205.0f, 2.18f, 1.05f, 39.0f,
         D2D1::ColorF(0x191722), D2D1::ColorF(0x332936), D2D1::ColorF(0x241E2B), D2D1::ColorF(0xE0B16D)},
        {L"지구", L"푸른 방어선", L"이끼형 균형 웨이브",
         2350.0f, 3200.0f, 220.0f, 2.05f, 1.14f, 36.0f,
         D2D1::ColorF(0x0E1D29), D2D1::ColorF(0x14303A), D2D1::ColorF(0x102731), D2D1::ColorF(0x56A7B7)},
        {L"화성", L"붉은 모래 전선", L"녹슨 장갑형 압박",
         2400.0f, 3550.0f, 230.0f, 1.98f, 1.24f, 33.0f,
         D2D1::ColorF(0x1D1718), D2D1::ColorF(0x3A2528), D2D1::ColorF(0x291D20), D2D1::ColorF(0xDD7666)},
        {L"목성", L"거대한 폭풍권", L"폭풍 방패형 중심",
         2600.0f, 4200.0f, 250.0f, 1.90f, 1.36f, 30.0f,
         D2D1::ColorF(0x1E1B16), D2D1::ColorF(0x3A3127), D2D1::ColorF(0x2B261F), D2D1::ColorF(0xD8A66A)},
        {L"토성", L"고리 위 방어전", L"고리 창병 원거리전",
         2650.0f, 4650.0f, 260.0f, 1.78f, 1.48f, 28.0f,
         D2D1::ColorF(0x171B20), D2D1::ColorF(0x2D3037), D2D1::ColorF(0x222731), D2D1::ColorF(0xCDBB83)},
        {L"천왕성", L"기울어진 얼음 궤도", L"빙결 질주형 압박",
         2750.0f, 5200.0f, 275.0f, 1.68f, 1.62f, 25.0f,
         D2D1::ColorF(0x101D22), D2D1::ColorF(0x19353A), D2D1::ColorF(0x122B31), D2D1::ColorF(0x80E5D4)},
        {L"해왕성", L"먼 푸른 심해", L"파도 망령 원거리 러시",
         2850.0f, 5850.0f, 290.0f, 1.56f, 1.78f, 23.0f,
         D2D1::ColorF(0x101928), D2D1::ColorF(0x182C48), D2D1::ColorF(0x13243C), D2D1::ColorF(0x75A7FF)},
        {L"명왕성", L"가장 먼 어둠", L"공허 중장갑과 빠른 보스",
         3000.0f, 6600.0f, 310.0f, 1.50f, 1.96f, 18.0f,
         D2D1::ColorF(0x15171F), D2D1::ColorF(0x292637), D2D1::ColorF(0x211F2E), D2D1::ColorF(0xC8B7FF)},
        {L"태양", L"마지막 항성 전선", L"플레어 돌격과 최종 보스",
         3300.0f, 7600.0f, 350.0f, 1.38f, 2.22f, 15.0f,
         D2D1::ColorF(0x241613), D2D1::ColorF(0x4A2A1F), D2D1::ColorF(0x351F18), D2D1::ColorF(0xFFB347)}
    }};
    return kStageTable[std::max(0, std::min(kStageCount - 1, index))];
}

UnitStats GetPlayerStats(PlayerUnit unit)
{
    UnitStats stats;
    switch (unit)
    {
    case PlayerUnit::Paw:
        stats.name = L"초보 발냥이";
        stats.cost = 75;
        stats.cooldown = 1.35f;
        stats.hp = 130.0f;
        stats.damage = 22.0f;
        stats.range = 30.0f;
        stats.attackDelay = 0.72f;
        stats.speed = 68.0f;
        stats.radius = 16.0f;
        stats.color = D2D1::ColorF(0xF4FBFF);
        stats.accent = D2D1::ColorF(0x65B8FF);
        break;
    case PlayerUnit::Box:
        stats.name = L"상자 수비냥";
        stats.cost = 145;
        stats.cooldown = 4.3f;
        stats.hp = 460.0f;
        stats.damage = 14.0f;
        stats.range = 28.0f;
        stats.attackDelay = 1.08f;
        stats.speed = 38.0f;
        stats.radius = 21.0f;
        stats.color = D2D1::ColorF(0xF8F0D6);
        stats.accent = D2D1::ColorF(0xDCA85B);
        break;
    case PlayerUnit::Spark:
        stats.name = L"번개 냥술사";
        stats.cost = 265;
        stats.cooldown = 6.4f;
        stats.hp = 96.0f;
        stats.damage = 68.0f;
        stats.range = 158.0f;
        stats.attackDelay = 1.78f;
        stats.speed = 47.0f;
        stats.radius = 15.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xE8D7FF);
        stats.accent = D2D1::ColorF(0xBA7BFF);
        break;
    case PlayerUnit::Dash:
        stats.name = L"돌진 꼬마냥";
        stats.cost = 115;
        stats.cooldown = 2.05f;
        stats.hp = 86.0f;
        stats.damage = 18.0f;
        stats.range = 26.0f;
        stats.attackDelay = 0.36f;
        stats.speed = 118.0f;
        stats.radius = 13.0f;
        stats.color = D2D1::ColorF(0xD9FFE6);
        stats.accent = D2D1::ColorF(0x62DD88);
        break;
    case PlayerUnit::Bell:
        stats.name = L"종소리 예언냥";
        stats.cost = 220;
        stats.cooldown = 5.15f;
        stats.hp = 118.0f;
        stats.damage = 42.0f;
        stats.range = 118.0f;
        stats.attackDelay = 1.06f;
        stats.speed = 45.0f;
        stats.radius = 15.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xFFF8C7);
        stats.accent = D2D1::ColorF(0xF2C94C);
        break;
    case PlayerUnit::Titan:
        stats.name = L"찹쌀 거대냥";
        stats.cost = 390;
        stats.cooldown = 9.2f;
        stats.hp = 760.0f;
        stats.damage = 88.0f;
        stats.range = 39.0f;
        stats.attackDelay = 1.66f;
        stats.speed = 24.0f;
        stats.radius = 27.0f;
        stats.color = D2D1::ColorF(0xFFE0EF);
        stats.accent = D2D1::ColorF(0xFF83B7);
        break;
    case PlayerUnit::Frost:
        stats.name = L"서리 방패냥";
        stats.cost = 185;
        stats.cooldown = 4.85f;
        stats.hp = 330.0f;
        stats.damage = 29.0f;
        stats.range = 48.0f;
        stats.attackDelay = 1.02f;
        stats.speed = 34.0f;
        stats.radius = 20.0f;
        stats.color = D2D1::ColorF(0xDDFBFF);
        stats.accent = D2D1::ColorF(0x74E8FF);
        break;
    case PlayerUnit::Comet:
        stats.name = L"혜성 돌격냥";
        stats.cost = 155;
        stats.cooldown = 3.05f;
        stats.hp = 104.0f;
        stats.damage = 34.0f;
        stats.range = 34.0f;
        stats.attackDelay = 0.54f;
        stats.speed = 136.0f;
        stats.radius = 14.0f;
        stats.color = D2D1::ColorF(0xFFF0D8);
        stats.accent = D2D1::ColorF(0xFF9F4A);
        break;
    case PlayerUnit::Orbit:
        stats.name = L"궤도 마도냥";
        stats.cost = 330;
        stats.cooldown = 7.2f;
        stats.hp = 112.0f;
        stats.damage = 84.0f;
        stats.range = 190.0f;
        stats.attackDelay = 2.05f;
        stats.speed = 35.0f;
        stats.radius = 16.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xDBE6FF);
        stats.accent = D2D1::ColorF(0x88A8FF);
        break;
    case PlayerUnit::Solar:
        stats.name = L"태양 용사냥";
        stats.cost = 480;
        stats.cooldown = 10.6f;
        stats.hp = 520.0f;
        stats.damage = 126.0f;
        stats.range = 58.0f;
        stats.attackDelay = 1.48f;
        stats.speed = 42.0f;
        stats.radius = 24.0f;
        stats.color = D2D1::ColorF(0xFFE7B5);
        stats.accent = D2D1::ColorF(0xFFB347);
        break;
    case PlayerUnit::Mint:
        stats.name = L"민트 치유냥";
        stats.cost = 245;
        stats.cooldown = 5.7f;
        stats.hp = 170.0f;
        stats.damage = 36.0f;
        stats.range = 126.0f;
        stats.attackDelay = 0.98f;
        stats.speed = 43.0f;
        stats.radius = 16.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xD8FFF3);
        stats.accent = D2D1::ColorF(0x61E6B0);
        break;
    case PlayerUnit::Drill:
        stats.name = L"드릴 굴착냥";
        stats.cost = 315;
        stats.cooldown = 6.8f;
        stats.hp = 420.0f;
        stats.damage = 72.0f;
        stats.range = 34.0f;
        stats.attackDelay = 1.18f;
        stats.speed = 54.0f;
        stats.radius = 20.0f;
        stats.color = D2D1::ColorF(0xE8E0D2);
        stats.accent = D2D1::ColorF(0xCDAA72);
        break;
    case PlayerUnit::Prism:
        stats.name = L"프리즘 저격냥";
        stats.cost = 430;
        stats.cooldown = 8.9f;
        stats.hp = 92.0f;
        stats.damage = 118.0f;
        stats.range = 230.0f;
        stats.attackDelay = 2.35f;
        stats.speed = 31.0f;
        stats.radius = 15.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xF2E8FF);
        stats.accent = D2D1::ColorF(0xE19BFF);
        break;
    case PlayerUnit::Nebula:
        stats.name = L"성운 여왕냥";
        stats.cost = 620;
        stats.cooldown = 13.4f;
        stats.hp = 420.0f;
        stats.damage = 152.0f;
        stats.range = 168.0f;
        stats.attackDelay = 1.86f;
        stats.speed = 28.0f;
        stats.radius = 26.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xE5D9FF);
        stats.accent = D2D1::ColorF(0x9D83FF);
        break;
    }
    return stats;
}

UnitStats GetEnemyStats(EnemyUnit unit, float threat)
{
    UnitStats stats;
    switch (unit)
    {
    case EnemyUnit::Dust:
        stats.name = L"먼지 조각병";
        stats.reward = 34 + static_cast<int>(threat * 3.0f);
        stats.hp = 88.0f + threat * 24.0f;
        stats.damage = 18.0f + threat * 2.5f;
        stats.range = 29.0f;
        stats.attackDelay = 0.95f;
        stats.speed = 46.0f + threat * 1.7f;
        stats.radius = 15.0f;
        stats.color = D2D1::ColorF(0x3A5163);
        stats.accent = D2D1::ColorF(0x8FA8B8);
        break;
    case EnemyUnit::Brute:
        stats.name = L"양철 괴력병";
        stats.reward = 78 + static_cast<int>(threat * 5.0f);
        stats.hp = 310.0f + threat * 78.0f;
        stats.damage = 31.0f + threat * 4.0f;
        stats.range = 34.0f;
        stats.attackDelay = 1.32f;
        stats.speed = 31.0f;
        stats.radius = 22.0f;
        stats.color = D2D1::ColorF(0x684C75);
        stats.accent = D2D1::ColorF(0xD5A8EA);
        break;
    case EnemyUnit::Skitter:
        stats.name = L"침돌이";
        stats.reward = 45 + static_cast<int>(threat * 4.0f);
        stats.hp = 66.0f + threat * 20.0f;
        stats.damage = 16.0f + threat * 2.0f;
        stats.range = 27.0f;
        stats.attackDelay = 0.62f;
        stats.speed = 82.0f + threat * 2.0f;
        stats.radius = 12.0f;
        stats.color = D2D1::ColorF(0x756D3B);
        stats.accent = D2D1::ColorF(0xFFE76A);
        break;
    case EnemyUnit::Sulfur:
        stats.name = L"황산 망령";
        stats.reward = 50 + static_cast<int>(threat * 4.0f);
        stats.hp = 94.0f + threat * 25.0f;
        stats.damage = 19.0f + threat * 2.4f;
        stats.range = 88.0f;
        stats.attackDelay = 1.14f;
        stats.speed = 42.0f + threat * 1.2f;
        stats.radius = 15.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xB8794F);
        stats.accent = D2D1::ColorF(0xFFD27A);
        break;
    case EnemyUnit::Moss:
        stats.name = L"이끼 포자병";
        stats.reward = 58 + static_cast<int>(threat * 4.0f);
        stats.hp = 145.0f + threat * 36.0f;
        stats.damage = 22.0f + threat * 2.8f;
        stats.range = 31.0f;
        stats.attackDelay = 0.92f;
        stats.speed = 43.0f + threat * 1.0f;
        stats.radius = 17.0f;
        stats.color = D2D1::ColorF(0x385C42);
        stats.accent = D2D1::ColorF(0x7BDB88);
        break;
    case EnemyUnit::Rust:
        stats.name = L"녹슨 파쇄병";
        stats.reward = 86 + static_cast<int>(threat * 6.0f);
        stats.hp = 390.0f + threat * 88.0f;
        stats.damage = 36.0f + threat * 4.5f;
        stats.range = 36.0f;
        stats.attackDelay = 1.36f;
        stats.speed = 28.0f + threat * 0.7f;
        stats.radius = 24.0f;
        stats.color = D2D1::ColorF(0x7D3A2D);
        stats.accent = D2D1::ColorF(0xFF8B60);
        break;
    case EnemyUnit::Storm:
        stats.name = L"폭풍 방패병";
        stats.reward = 106 + static_cast<int>(threat * 7.0f);
        stats.hp = 520.0f + threat * 108.0f;
        stats.damage = 32.0f + threat * 4.0f;
        stats.range = 42.0f;
        stats.attackDelay = 1.18f;
        stats.speed = 25.0f + threat * 0.6f;
        stats.radius = 27.0f;
        stats.color = D2D1::ColorF(0x8A6846);
        stats.accent = D2D1::ColorF(0xF1D09A);
        break;
    case EnemyUnit::Ring:
        stats.name = L"고리 창병";
        stats.reward = 72 + static_cast<int>(threat * 5.0f);
        stats.hp = 160.0f + threat * 42.0f;
        stats.damage = 34.0f + threat * 3.8f;
        stats.range = 118.0f;
        stats.attackDelay = 1.38f;
        stats.speed = 47.0f + threat * 1.2f;
        stats.radius = 16.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0x736C55);
        stats.accent = D2D1::ColorF(0xE6D392);
        break;
    case EnemyUnit::Frost:
        stats.name = L"빙결 질주병";
        stats.reward = 62 + static_cast<int>(threat * 5.0f);
        stats.hp = 118.0f + threat * 30.0f;
        stats.damage = 21.0f + threat * 2.7f;
        stats.range = 33.0f;
        stats.attackDelay = 0.74f;
        stats.speed = 96.0f + threat * 2.4f;
        stats.radius = 14.0f;
        stats.color = D2D1::ColorF(0x4E8F95);
        stats.accent = D2D1::ColorF(0xB9FFF5);
        break;
    case EnemyUnit::Tide:
        stats.name = L"파도 망령";
        stats.reward = 76 + static_cast<int>(threat * 6.0f);
        stats.hp = 132.0f + threat * 35.0f;
        stats.damage = 28.0f + threat * 3.5f;
        stats.range = 132.0f;
        stats.attackDelay = 1.08f;
        stats.speed = 70.0f + threat * 1.8f;
        stats.radius = 17.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0x2F5C97);
        stats.accent = D2D1::ColorF(0x75A7FF);
        break;
    case EnemyUnit::Void:
        stats.name = L"공허 껍질병";
        stats.reward = 122 + static_cast<int>(threat * 8.0f);
        stats.hp = 650.0f + threat * 128.0f;
        stats.damage = 48.0f + threat * 5.4f;
        stats.range = 46.0f;
        stats.attackDelay = 1.28f;
        stats.speed = 24.0f + threat * 0.5f;
        stats.radius = 28.0f;
        stats.color = D2D1::ColorF(0x332B4A);
        stats.accent = D2D1::ColorF(0xC8B7FF);
        break;
    case EnemyUnit::Flare:
        stats.name = L"플레어 주자";
        stats.reward = 92 + static_cast<int>(threat * 7.0f);
        stats.hp = 190.0f + threat * 48.0f;
        stats.damage = 38.0f + threat * 4.6f;
        stats.range = 38.0f;
        stats.attackDelay = 0.68f;
        stats.speed = 112.0f + threat * 2.6f;
        stats.radius = 16.0f;
        stats.color = D2D1::ColorF(0x9E3F2D);
        stats.accent = D2D1::ColorF(0xFFB347);
        break;
    case EnemyUnit::Spore:
        stats.name = L"포자 등불병";
        stats.reward = 82 + static_cast<int>(threat * 6.0f);
        stats.hp = 210.0f + threat * 54.0f;
        stats.damage = 31.0f + threat * 3.6f;
        stats.range = 112.0f;
        stats.attackDelay = 1.24f;
        stats.speed = 36.0f + threat * 1.0f;
        stats.radius = 18.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0x625083);
        stats.accent = D2D1::ColorF(0xF0A8FF);
        break;
    case EnemyUnit::Quake:
        stats.name = L"지진 황소병";
        stats.reward = 145 + static_cast<int>(threat * 9.0f);
        stats.hp = 820.0f + threat * 150.0f;
        stats.damage = 58.0f + threat * 6.2f;
        stats.range = 48.0f;
        stats.attackDelay = 1.62f;
        stats.speed = 19.0f + threat * 0.4f;
        stats.radius = 32.0f;
        stats.color = D2D1::ColorF(0x57463D);
        stats.accent = D2D1::ColorF(0xD6B08C);
        break;
    case EnemyUnit::Mirror:
        stats.name = L"반사 임프";
        stats.reward = 88 + static_cast<int>(threat * 7.0f);
        stats.hp = 165.0f + threat * 44.0f;
        stats.damage = 35.0f + threat * 4.1f;
        stats.range = 152.0f;
        stats.attackDelay = 1.02f;
        stats.speed = 62.0f + threat * 1.7f;
        stats.radius = 15.0f;
        stats.ranged = true;
        stats.color = D2D1::ColorF(0xDDEAFF);
        stats.accent = D2D1::ColorF(0x9CEBFF);
        break;
    case EnemyUnit::Comet:
        stats.name = L"혜성 사냥견";
        stats.reward = 96 + static_cast<int>(threat * 7.0f);
        stats.hp = 150.0f + threat * 40.0f;
        stats.damage = 42.0f + threat * 4.8f;
        stats.range = 32.0f;
        stats.attackDelay = 0.54f;
        stats.speed = 132.0f + threat * 3.0f;
        stats.radius = 15.0f;
        stats.color = D2D1::ColorF(0xB7543B);
        stats.accent = D2D1::ColorF(0xFFDB7A);
        break;
    case EnemyUnit::Boss:
        stats.name = L"태양 관문";
        stats.reward = 240 + static_cast<int>(threat * 12.0f);
        stats.hp = 1180.0f + threat * 180.0f;
        stats.damage = 74.0f + threat * 7.0f;
        stats.range = 42.0f;
        stats.attackDelay = 1.45f;
        stats.speed = 25.0f;
        stats.radius = 31.0f;
        stats.color = D2D1::ColorF(0x71323A);
        stats.accent = D2D1::ColorF(0xFF9BA8);
        break;
    }
    return stats;
}
