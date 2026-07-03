#include "PawlineGameImpl.h"

// Gameplay simulation, combat resolution, and short-lived VFX spawning.
void PawlineGameImpl::UpdateEnemyDirector(float dt)
{
    // The director scales spawn timing from the selected stage and elapsed time,
    // giving later planets denser waves without hard-coding every spawn.
    m_enemyTimer -= dt;
    const float threat = ThreatLevel();
    const StageDefinition stage = CurrentStage();

    if (m_stageTime >= m_nextBossTime)
    {
        const EnemyUnit bossType = StageBossType();
        SpawnEnemy(bossType, true);
        if (!m_units.empty())
        {
            const Unit& boss = m_units.back();
            m_bossFocusX = std::max(0.0f, std::min(kCameraMaxX, boss.pos.x - 760.0f));
        }
        m_bossBannerTimer = 3.15f;
        m_bossWarningTimer = 1.20f;
        AddCameraTrauma(0.62f);
        m_nextBossTime += 44.0f / stage.threatScale;
        m_enemyTimer = std::min(m_enemyTimer, 1.1f);
        SetMessage(GetEnemyStats(bossType, threat).name + L" incoming.");
        AddRing({kEnemyBaseX - 80.0f, kLaneY}, 220.0f, 0.70f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.46f), 5.0f);
    }

    if (m_enemyTimer > 0.0f)
    {
        return;
    }

    const int phase = static_cast<int>(m_stageTime / 22.0f);
    std::uniform_int_distribution<int> roll(0, 99);
    const int value = roll(m_rng);
    const EnemyUnit type = PickStageEnemy(value, phase);

    SpawnEnemy(type);
    const float difficultyInterval = m_difficulty == Difficulty::Easy ? 1.15f : (m_difficulty == Difficulty::Hard ? 0.86f : 1.0f);
    const float interval = std::max(0.48f, (stage.enemyInterval - threat * 0.11f) * difficultyInterval);
    m_enemyTimer += interval;
}

EnemyUnit PawlineGameImpl::PickStageEnemy(int value, int phase) const
{
    switch (m_selectedStage)
    {
    case 0:
        if (phase >= 3 && value < 18)
        {
            return EnemyUnit::Brute;
        }
        return value < 42 || phase < 1 ? EnemyUnit::Dust : EnemyUnit::Skitter;
    case 1:
        if (phase >= 2 && value < 24)
        {
            return EnemyUnit::Sulfur;
        }
        if (phase >= 3 && value < 42)
        {
            return EnemyUnit::Mirror;
        }
        return value < 62 ? EnemyUnit::Skitter : EnemyUnit::Dust;
    case 2:
        if (phase >= 2 && value < 32)
        {
            return EnemyUnit::Moss;
        }
        if (phase >= 3 && value < 48)
        {
            return EnemyUnit::Spore;
        }
        if (value < 54)
        {
            return EnemyUnit::Dust;
        }
        return value < 78 ? EnemyUnit::Skitter : EnemyUnit::Brute;
    case 3:
        if (phase >= 1 && value < 44)
        {
            return EnemyUnit::Rust;
        }
        return value < 66 ? EnemyUnit::Dust : EnemyUnit::Brute;
    case 4:
        if (value < 42)
        {
            return EnemyUnit::Storm;
        }
        if (phase >= 2 && value < 58)
        {
            return EnemyUnit::Quake;
        }
        return value < 76 ? EnemyUnit::Sulfur : EnemyUnit::Brute;
    case 5:
        if (value < 44)
        {
            return EnemyUnit::Ring;
        }
        return value < 72 ? EnemyUnit::Skitter : EnemyUnit::Storm;
    case 6:
        if (value < 54)
        {
            return EnemyUnit::Frost;
        }
        return value < 78 ? EnemyUnit::Ring : EnemyUnit::Tide;
    case 7:
        if (value < 50)
        {
            return EnemyUnit::Tide;
        }
        return value < 78 ? EnemyUnit::Frost : EnemyUnit::Void;
    case 8:
        if (value < 48)
        {
            return EnemyUnit::Void;
        }
        if (value < 68)
        {
            return EnemyUnit::Quake;
        }
        return value < 84 ? EnemyUnit::Tide : EnemyUnit::Rust;
    default:
        if (value < 42)
        {
            return EnemyUnit::Flare;
        }
        if (value < 62)
        {
            return EnemyUnit::Comet;
        }
        return value < 82 ? EnemyUnit::Void : EnemyUnit::Boss;
    }
}

EnemyUnit PawlineGameImpl::StageBossType() const
{
    switch (m_selectedStage)
    {
    case 0:
        return EnemyUnit::Brute;
    case 1:
        return EnemyUnit::Sulfur;
    case 2:
        return EnemyUnit::Moss;
    case 3:
        return EnemyUnit::Rust;
    case 4:
        return EnemyUnit::Storm;
    case 5:
        return EnemyUnit::Ring;
    case 6:
        return EnemyUnit::Frost;
    case 7:
        return EnemyUnit::Tide;
    case 8:
        return EnemyUnit::Quake;
    default:
        return EnemyUnit::Boss;
    }
}

std::wstring PawlineGameImpl::StageEnemySummary() const
{
    switch (m_selectedStage)
    {
    case 0:
        return L"먼지 / 침돌이 / 양철";
    case 1:
        return L"황산 / 반사 / 침돌이";
    case 2:
        return L"이끼 / 포자 / 양철";
    case 3:
        return L"녹슨 / 먼지 / 양철";
    case 4:
        return L"폭풍 / 지진 / 황산";
    case 5:
        return L"고리 / 침돌이 / 폭풍";
    case 6:
        return L"빙결 / 고리 / 파도";
    case 7:
        return L"파도 / 빙결 / 공허";
    case 8:
        return L"공허 / 지진 / 녹슨";
    default:
        return L"플레어 / 혜성 / 태양";
    }
}

float PawlineGameImpl::GimmickInterval() const
{
    switch (m_selectedStage)
    {
    case 0: return 12.0f;
    case 1: return 11.0f;
    case 2: return 14.0f;
    case 3: return 10.8f;
    case 4: return 13.0f;
    case 5: return 15.0f;
    case 6: return 11.5f;
    case 7: return 12.6f;
    case 8: return 10.4f;
    default: return 9.2f;
    }
}

Unit* PawlineGameImpl::FindBossUnit()
{
    Unit* boss = nullptr;
    for (Unit& unit : m_units)
    {
        if (unit.team == Team::Enemy && unit.alive && (unit.elite || static_cast<EnemyUnit>(unit.kind) == EnemyUnit::Boss))
        {
            if (!boss || unit.maxHp > boss->maxHp)
            {
                boss = &unit;
            }
        }
    }
    return boss;
}

const Unit* PawlineGameImpl::FindBossUnit() const
{
    const Unit* boss = nullptr;
    for (const Unit& unit : m_units)
    {
        if (unit.team == Team::Enemy && unit.alive && (unit.elite || static_cast<EnemyUnit>(unit.kind) == EnemyUnit::Boss))
        {
            if (!boss || unit.maxHp > boss->maxHp)
            {
                boss = &unit;
            }
        }
    }
    return boss;
}

void PawlineGameImpl::UpdateBossPatterns(float dt)
{
    Unit* boss = FindBossUnit();
    if (!boss)
    {
        m_bossPatternTimer = std::max(4.5f, 8.0f - static_cast<float>(m_selectedStage) * 0.25f);
        m_bossPhaseTwoTriggered = false;
        return;
    }

    if (!m_bossPhaseTwoTriggered && boss->hp < boss->maxHp * 0.50f)
    {
        m_bossPhaseTwoTriggered = true;
        m_bossPatternTimer = 1.2f;
        SetMessage(L"Boss phase two.");
        AddRing(boss->pos, 180.0f, 0.70f, D2D1::ColorF(0xFFB347, 0.48f), 5.0f);
        AddCameraTrauma(0.58f);
        SpawnStageReinforcement(m_selectedStage == 9 ? EnemyUnit::Flare : StageBossType(), 360.0f, false);
    }

    m_bossPatternTimer -= dt;
    if (m_bossPatternTimer > 0.0f)
    {
        return;
    }

    TriggerBossPattern(*boss);
    const float baseDelay = m_bossPhaseTwoTriggered ? 5.8f : 7.4f;
    m_bossPatternTimer = std::max(3.6f, baseDelay - ThreatLevel() * 0.22f);
}

void PawlineGameImpl::TriggerBossPattern(Unit& boss)
{
    const int pattern = static_cast<int>(std::fmod(m_stageTime * 10.0f + static_cast<float>(boss.id * 7), 3.0f));
    const EnemyUnit bossType = static_cast<EnemyUnit>(boss.kind);
    const D2D1_COLOR_F bossColor = GetEnemyStats(static_cast<EnemyUnit>(boss.kind), ThreatLevel()).accent;
    const float threat = ThreatLevel();
    const Vec2 bossCenter = boss.pos;

    // 보스는 공통 공격만 쓰지 않고, 자기 행성의 대표 기믹을 더 강한 패턴으로 변형해서 사용한다.
    switch (bossType)
    {
    case EnemyUnit::Brute:
        SetMessage(L"양철 보스: 압축 파동.");
        AddTelegraph(TelegraphKind::BossPulseCircle, TelegraphShape::Circle, bossCenter, bossCenter, 190.0f, 0.0f, 1.08f, 58.0f + threat * 3.6f, D2D1::ColorF(0xCFA27B));
        return;
    case EnemyUnit::Sulfur:
        SetMessage(L"황산 보스: 산성 장막.");
        AddTelegraph(TelegraphKind::VenusFog, TelegraphShape::FullLane, {std::max(kPlayerBaseX + 90.0f, bossCenter.x - 620.0f), kLaneY}, {bossCenter.x + 120.0f, kLaneY}, 360.0f, 0.0f, 0.95f, 16.0f + threat * 1.3f, D2D1::ColorF(0xE0B16D));
        return;
    case EnemyUnit::Moss:
        SetMessage(L"이끼 보스: 포자 증식.");
        AddTelegraph(TelegraphKind::BossReinforce, TelegraphShape::Circle, {std::max(kPlayerBaseX + 360.0f, bossCenter.x - 260.0f), RandomLaneY()}, bossCenter, 112.0f, 0.0f, 0.90f, 0.0f, D2D1::ColorF(0xB8FF89));
        return;
    case EnemyUnit::Rust:
        SetMessage(L"녹슨 보스: 붉은 낙하.");
        AddTelegraph(TelegraphKind::MarsMeteor, TelegraphShape::Circle, {std::max(kPlayerBaseX + 240.0f, bossCenter.x - 360.0f), RandomLaneY()}, bossCenter, 138.0f, 0.0f, 0.86f, 62.0f + threat * 3.8f, D2D1::ColorF(0xFF8B60));
        AddTelegraph(TelegraphKind::MarsMeteor, TelegraphShape::Circle, {std::max(kPlayerBaseX + 380.0f, bossCenter.x - 180.0f), RandomLaneY()}, bossCenter, 112.0f, 0.0f, 1.06f, 48.0f + threat * 3.0f, D2D1::ColorF(0xFFB08B));
        return;
    case EnemyUnit::Storm:
        SetMessage(L"폭풍 보스: 중력 소용돌이.");
        AddTelegraph(TelegraphKind::JupiterGravity, TelegraphShape::Circle, {std::max(kPlayerBaseX + 430.0f, bossCenter.x - 260.0f), kLaneY}, bossCenter, 330.0f, 0.0f, 1.12f, 18.0f + threat * 1.8f, D2D1::ColorF(0xD8A66A));
        return;
    case EnemyUnit::Ring:
        SetMessage(L"고리 보스: 고리 창 소환.");
        AddTelegraph(TelegraphKind::SaturnReinforce, TelegraphShape::Circle, {std::max(kPlayerBaseX + 390.0f, bossCenter.x - 300.0f), RandomLaneY()}, bossCenter, 120.0f, 0.0f, 0.88f, 0.0f, D2D1::ColorF(0xE6D392));
        AddTelegraph(TelegraphKind::BossFlareLine, TelegraphShape::Line, bossCenter, {std::max(kPlayerBaseX + 80.0f, bossCenter.x - 560.0f), bossCenter.y - 54.0f}, 0.0f, 62.0f, 1.05f, 46.0f + threat * 2.5f, D2D1::ColorF(0xE6D392));
        return;
    case EnemyUnit::Frost:
        SetMessage(L"빙결 보스: 얼음 가름.");
        AddTelegraph(TelegraphKind::UranusIce, TelegraphShape::Line, {bossCenter.x, bossCenter.y - 96.0f}, {std::max(kPlayerBaseX + 70.0f, bossCenter.x - 680.0f), bossCenter.y + 86.0f}, 0.0f, 104.0f, 0.88f, 34.0f + threat * 2.5f, D2D1::ColorF(0xD9FFF8));
        return;
    case EnemyUnit::Tide:
        SetMessage(L"파도 보스: 심해 밀물.");
        AddTelegraph(TelegraphKind::NeptuneTide, TelegraphShape::FullLane, {std::max(kPlayerBaseX + 90.0f, bossCenter.x - 620.0f), kLaneY + 28.0f}, {bossCenter.x + 120.0f, kLaneY + 28.0f}, 380.0f, 0.0f, 0.90f, 34.0f + threat * 2.4f, D2D1::ColorF(0x75A7FF));
        return;
    case EnemyUnit::Quake:
        SetMessage(L"지진 보스: 공허 균열.");
        AddTelegraph(TelegraphKind::PlutoVoid, TelegraphShape::Circle, {std::max(kPlayerBaseX + 420.0f, bossCenter.x - 260.0f), kLaneY}, bossCenter, 250.0f, 0.0f, 1.02f, 54.0f + threat * 3.2f, D2D1::ColorF(0xC8B7FF));
        return;
    case EnemyUnit::Boss:
        SetMessage(L"태양 관문: 삼중 플레어.");
        AddTelegraph(TelegraphKind::SolarFlare, TelegraphShape::Line, bossCenter, {std::max(kPlayerBaseX + 80.0f, bossCenter.x - 760.0f), bossCenter.y - 92.0f}, 0.0f, 94.0f, 0.88f, 72.0f + threat * 4.0f, D2D1::ColorF(0xFFB347));
        AddTelegraph(TelegraphKind::SolarFlare, TelegraphShape::Line, {bossCenter.x, bossCenter.y + 50.0f}, {std::max(kPlayerBaseX + 80.0f, bossCenter.x - 720.0f), bossCenter.y + 96.0f}, 0.0f, 84.0f, 1.06f, 58.0f + threat * 3.3f, D2D1::ColorF(0xFFF4B8));
        return;
    default:
        break;
    }

    if (pattern == 0)
    {
        SetMessage(L"보스 플레어 라인.");
        const Vec2 end = {std::max(kPlayerBaseX + 70.0f, boss.pos.x - 620.0f), boss.pos.y + std::sin(m_stageTime * 1.7f) * 72.0f};
        AddTelegraph(TelegraphKind::BossFlareLine, TelegraphShape::Line, boss.pos, end, 0.0f, 82.0f, 1.18f, 72.0f + ThreatLevel() * 4.0f, D2D1::ColorF(0xFFB347));
    }
    else if (pattern == 1)
    {
        SetMessage(L"보스 코어 펄스.");
        AddTelegraph(TelegraphKind::BossPulseCircle, TelegraphShape::Circle, boss.pos, boss.pos, 210.0f, 0.0f, 1.05f, 48.0f + ThreatLevel() * 3.0f, D2D1::ColorF(bossColor.r, bossColor.g, bossColor.b, 0.92f));
    }
    else
    {
        SetMessage(L"보스 지원군 호출.");
        const Vec2 gate = {std::max(kPlayerBaseX + 420.0f, boss.pos.x - 240.0f), RandomLaneY()};
        AddTelegraph(TelegraphKind::BossReinforce, TelegraphShape::Circle, gate, gate, 86.0f, 0.0f, 1.00f, 0.0f, D2D1::ColorF(0xFF9BA8));
    }
}

float PawlineGameImpl::EffectiveUnitRange(const Unit& unit) const
{
    float range = unit.range;
    if (m_selectedStage == 1 && unit.ranged)
    {
        range *= unit.team == Team::Player ? 0.74f : 0.88f;
    }
    if (m_selectedStage == 8 && unit.team == Team::Enemy)
    {
        range *= 1.08f;
    }
    if (m_selectedStage == 9 && unit.team == Team::Enemy)
    {
        range *= 1.10f;
    }
    return range;
}

float PawlineGameImpl::StageMoveSpeedModifier(const Unit& unit) const
{
    float modifier = 1.0f;
    if (m_selectedStage == 4)
    {
        const float gravityWave = std::sin(m_stageTime * 0.92f + unit.pos.y * 0.018f);
        modifier *= 0.92f + gravityWave * 0.16f;
    }
    else if (m_selectedStage == 6)
    {
        modifier *= static_cast<EnemyUnit>(unit.kind) == EnemyUnit::Frost && unit.team == Team::Enemy ? 1.08f : 0.88f;
    }
    else if (m_selectedStage == 7)
    {
        modifier *= 0.96f + std::sin(m_stageTime * 1.15f + unit.pos.x * 0.006f) * 0.10f;
    }
    else if (m_selectedStage == 9 && unit.team == Team::Enemy)
    {
        modifier *= 1.08f;
    }
    return std::max(0.55f, modifier);
}

void PawlineGameImpl::UpdateStageGimmicks(float dt)
{
    m_stageGimmickTimer -= dt;
    m_stageAmbientTimer -= dt;
    if (m_stageAmbientTimer <= 0.0f)
    {
        m_stageAmbientTimer = m_selectedStage >= 8 ? 0.07f : 0.12f;
        std::uniform_real_distribution<float> xDist(m_cameraX + 70.0f, m_cameraX + kWidth - 70.0f);
        std::uniform_real_distribution<float> yDist(kBattleTop + 40.0f, kBattleBottom - 40.0f);
        const Vec2 pos = {std::max(40.0f, std::min(kWorldWidth - 40.0f, xDist(m_rng))), yDist(m_rng)};
        switch (m_selectedStage)
        {
        case 0:
            AddParticleEx(pos, {-28.0f, 6.0f}, 2.4f, 0.44f, D2D1::ColorF(0xC7A282, 0.34f), ParticleKind::Dust, -2.0f, 0.92f, 5.0f);
            break;
        case 3:
            AddParticleEx(pos, {-46.0f, 18.0f}, 2.8f, 0.46f, D2D1::ColorF(0xFF8B60, 0.48f), ParticleKind::Ember, 6.0f, 0.92f, -1.2f);
            break;
        case 6:
            AddParticleEx(pos, {-24.0f, 18.0f}, 3.0f, 0.68f, D2D1::ColorF(0xD9FFF8, 0.50f), ParticleKind::Snow, 0.0f, 0.96f, -0.8f);
            break;
        case 7:
            AddParticleEx(pos, {-18.0f, -16.0f}, 4.0f, 0.72f, D2D1::ColorF(0xBFD9FF, 0.38f), ParticleKind::Bubble, -12.0f, 0.94f, 3.0f);
            break;
        case 9:
            AddParticleEx(pos, {-78.0f, 24.0f}, 3.4f, 0.50f, D2D1::ColorF(0xFFB347, 0.56f), ParticleKind::Ember, -4.0f, 0.91f, -1.0f);
            break;
        default:
            break;
        }
    }

    if (m_stageGimmickTimer > 0.0f)
    {
        return;
    }

    TriggerStageGimmick();
    m_stageGimmickTimer += GimmickInterval();
}

void PawlineGameImpl::TriggerStageGimmick()
{
    const StageDefinition stage = CurrentStage();
    m_stageGimmickPulse = 1.35f;
    std::uniform_real_distribution<float> xDist(kPlayerBaseX + 260.0f, kEnemyBaseX - 260.0f);
    std::uniform_real_distribution<float> yDist(kLaneY - 70.0f, kLaneY + 70.0f);

    // 행성 기믹은 바로 터뜨리지 않고 먼저 예고 장판을 만든다.
    // 예고가 끝나면 ExecuteTelegraph()에서 실제 피해/회복/지원군을 처리한다.
    switch (m_selectedStage)
    {
    case 0:
        SetMessage(L"Mercury heat wave.");
        AddTelegraph(TelegraphKind::MercuryHeat, TelegraphShape::FullLane, {kPlayerBaseX + 40.0f, kLaneY - 86.0f}, {kEnemyBaseX - 30.0f, kLaneY + 68.0f}, kWorldWidth, 92.0f, 0.90f, 12.0f + ThreatLevel() * 1.6f, D2D1::ColorF(0xCFA27B));
        break;
    case 1:
        SetMessage(L"Venus acid fog: ranged units lose range.");
        AddTelegraph(TelegraphKind::VenusFog, TelegraphShape::FullLane, {m_cameraX + 64.0f, kLaneY}, {m_cameraX + kWidth - 64.0f, kLaneY}, 280.0f, 0.0f, 0.85f, 8.0f + ThreatLevel(), D2D1::ColorF(0xE0B16D));
        break;
    case 2:
        SetMessage(L"Earth supply bloom.");
        AddTelegraph(TelegraphKind::EarthBloom, TelegraphShape::Circle, {kPlayerBaseX + 72.0f, kLaneY}, {kPlayerBaseX + 72.0f, kLaneY}, 150.0f, 0.0f, 0.70f, 0.0f, D2D1::ColorF(0xB8FF89));
        break;
    case 3:
    {
        SetMessage(L"Mars meteor impact.");
        const Vec2 impact = {xDist(m_rng), yDist(m_rng)};
        AddTelegraph(TelegraphKind::MarsMeteor, TelegraphShape::Circle, impact, impact, 154.0f, 0.0f, 1.10f, 64.0f + ThreatLevel() * 4.0f, D2D1::ColorF(0xFF8B60));
        break;
    }
    case 4:
        SetMessage(L"Jupiter gravity surge.");
        AddTelegraph(TelegraphKind::JupiterGravity, TelegraphShape::Circle, {m_cameraX + 640.0f, kLaneY}, {m_cameraX + 640.0f, kLaneY}, 340.0f, 0.0f, 0.95f, 0.0f, D2D1::ColorF(0xD8A66A));
        break;
    case 5:
        SetMessage(L"Saturn ring reinforcement.");
        AddTelegraph(TelegraphKind::SaturnReinforce, TelegraphShape::Circle, {kEnemyBaseX - 330.0f, kLaneY}, {kEnemyBaseX - 330.0f, kLaneY}, 124.0f, 0.0f, 0.95f, 0.0f, D2D1::ColorF(0xE6D392));
        break;
    case 6:
        SetMessage(L"Uranus ice gust.");
        AddTelegraph(TelegraphKind::UranusIce, TelegraphShape::Line, {m_cameraX + 58.0f, kLaneY - 92.0f}, {m_cameraX + 1210.0f, kLaneY + 76.0f}, 0.0f, 94.0f, 0.85f, 0.0f, D2D1::ColorF(0xD9FFF8));
        break;
    case 7:
        SetMessage(L"Neptune tide surge.");
        AddTelegraph(TelegraphKind::NeptuneTide, TelegraphShape::FullLane, {m_cameraX + 64.0f, kLaneY + 32.0f}, {m_cameraX + kWidth - 64.0f, kLaneY + 32.0f}, 320.0f, 0.0f, 0.80f, 0.0f, D2D1::ColorF(0x75A7FF));
        break;
    case 8:
        SetMessage(L"Pluto void eclipse.");
        AddTelegraph(TelegraphKind::PlutoVoid, TelegraphShape::Circle, {m_cameraX + 640.0f, kLaneY}, {m_cameraX + 640.0f, kLaneY}, 230.0f, 0.0f, 0.95f, 34.0f + ThreatLevel() * 2.2f, D2D1::ColorF(0xC8B7FF));
        break;
    default:
        SetMessage(L"Solar flare.");
        AddTelegraph(TelegraphKind::SolarFlare, TelegraphShape::Line, {m_cameraX + 20.0f, kBattleTop + 44.0f}, {m_cameraX + kWidth - 20.0f, kBattleBottom - 58.0f}, 330.0f, 128.0f, 1.05f, 52.0f + ThreatLevel() * 3.0f, D2D1::ColorF(0xFFB347));
        break;
    }
}

void PawlineGameImpl::AddTelegraph(TelegraphKind kind, TelegraphShape shape, Vec2 start, Vec2 end, float radius, float width, float windup, float damage, D2D1_COLOR_F color)
{
    Telegraph telegraph;
    telegraph.kind = kind;
    telegraph.shape = shape;
    telegraph.start = start;
    telegraph.end = end;
    telegraph.radius = radius;
    telegraph.width = width;
    telegraph.life = windup;
    telegraph.maxLife = windup;
    telegraph.damage = damage;
    telegraph.color = color;
    m_telegraphs.push_back(telegraph);
}

void PawlineGameImpl::UpdateTelegraphs(float dt)
{
    for (Telegraph& telegraph : m_telegraphs)
    {
        telegraph.life -= dt;
        if (telegraph.life <= 0.0f)
        {
            ExecuteTelegraph(telegraph);
        }
    }

    m_telegraphs.erase(
        std::remove_if(m_telegraphs.begin(), m_telegraphs.end(), [](const Telegraph& telegraph) {
            return telegraph.life <= 0.0f;
        }),
        m_telegraphs.end());
}

void PawlineGameImpl::ExecuteTelegraph(const Telegraph& telegraph)
{
    // 예고가 끝나는 순간 실제 스테이지 효과와 보스 패턴을 처리한다.
    switch (telegraph.kind)
    {
    case TelegraphKind::MercuryHeat:
        AddBeam(telegraph.start, telegraph.end, 10.0f, 0.30f, D2D1::ColorF(0xCFA27B, 0.52f));
        ApplyAreaDamage({(kPlayerBaseX + kEnemyBaseX) * 0.5f, kLaneY}, kWorldWidth, telegraph.damage, D2D1::ColorF(0xCFA27B));
        AddCameraTrauma(0.20f);
        break;
    case TelegraphKind::VenusFog:
        for (Unit& unit : m_units)
        {
            if (unit.alive && unit.ranged)
            {
                DamageUnit(unit, telegraph.damage, unit.team == Team::Player ? Team::Enemy : Team::Player);
            }
        }
        AddRing({m_cameraX + 640.0f, kLaneY}, 280.0f, 0.60f, D2D1::ColorF(0xE0B16D, 0.38f), 3.2f);
        break;
    case TelegraphKind::EarthBloom:
        m_energy = std::min(MaxEnergy(), m_energy + 76.0f);
        m_playerBaseHp = std::min(m_playerBaseMaxHp, m_playerBaseHp + 42.0f);
        for (Unit& unit : m_units)
        {
            if (unit.alive && unit.team == Team::Player)
            {
                unit.hp = std::min(unit.maxHp, unit.hp + 26.0f);
                AddParticleEx(unit.pos, {0.0f, -22.0f}, 7.0f, 0.42f, D2D1::ColorF(0xB8FF89, 0.46f), ParticleKind::Glow, 0.0f, 0.92f, 18.0f);
            }
        }
        AddRing(telegraph.start, 150.0f, 0.48f, D2D1::ColorF(0xB8FF89, 0.44f), 3.0f);
        break;
    case TelegraphKind::MarsMeteor:
        AddBeam({telegraph.start.x + 260.0f, kBattleTop + 8.0f}, telegraph.start, 11.0f, 0.22f, D2D1::ColorF(0xFF8B60, 0.72f));
        ApplyAreaDamage(telegraph.start, telegraph.radius, telegraph.damage, D2D1::ColorF(0xFF8B60));
        AddDustPuff({telegraph.start.x, telegraph.start.y + 18.0f}, D2D1::ColorF(0xDD7666, 0.42f), 22);
        AddCameraTrauma(0.48f);
        break;
    case TelegraphKind::JupiterGravity:
        for (Unit& unit : m_units)
        {
            if (unit.alive)
            {
                const float pull = unit.pos.y < kLaneY ? 22.0f : -22.0f;
                unit.pos.y = std::max(kLaneY - 82.0f, std::min(kLaneY + 82.0f, unit.pos.y + pull));
                ShakeUnit(unit, 0.14f);
            }
        }
        AddRing(telegraph.start, 340.0f, 0.64f, D2D1::ColorF(0xD8A66A, 0.42f), 4.0f);
        AddCameraTrauma(0.30f);
        break;
    case TelegraphKind::SaturnReinforce:
        SpawnStageReinforcement(EnemyUnit::Ring, 330.0f);
        if (ThreatLevel() > 2.0f)
        {
            SpawnStageReinforcement(EnemyUnit::Skitter, 410.0f);
        }
        AddRing(telegraph.start, 124.0f, 0.52f, D2D1::ColorF(0xE6D392, 0.48f), 3.0f);
        break;
    case TelegraphKind::UranusIce:
        for (Unit& unit : m_units)
        {
            if (unit.alive)
            {
                unit.pos.x += unit.team == Team::Player ? -24.0f : 24.0f;
                unit.hitFlash = std::max(unit.hitFlash, 0.10f);
            }
        }
        AddBeam(telegraph.start, telegraph.end, 12.0f, 0.28f, D2D1::ColorF(0xD9FFF8, 0.46f));
        AddCameraTrauma(0.22f);
        break;
    case TelegraphKind::NeptuneTide:
        for (Unit& unit : m_units)
        {
            if (unit.alive)
            {
                unit.pos.x += unit.team == Team::Player ? 18.0f : -18.0f;
            }
        }
        AddRing(telegraph.start, 320.0f, 0.66f, D2D1::ColorF(0x75A7FF, 0.40f), 4.0f);
        break;
    case TelegraphKind::PlutoVoid:
        m_energy = std::max(0.0f, m_energy - 38.0f);
        ApplyAreaDamage(telegraph.start, telegraph.radius, telegraph.damage, D2D1::ColorF(0xC8B7FF));
        AddRing(telegraph.start, 230.0f, 0.58f, D2D1::ColorF(0xC8B7FF, 0.42f), 3.4f);
        break;
    case TelegraphKind::SolarFlare:
        AddBeam(telegraph.start, telegraph.end, 15.0f, 0.28f, D2D1::ColorF(0xFFB347, 0.62f));
        ApplyLineDamage(telegraph.start, telegraph.end, telegraph.width, telegraph.damage, D2D1::ColorF(0xFFB347));
        SpawnStageReinforcement(EnemyUnit::Flare, 250.0f);
        AddCameraTrauma(0.44f);
        break;
    case TelegraphKind::BossFlareLine:
        AddBeam(telegraph.start, telegraph.end, 13.0f, 0.24f, D2D1::ColorF(0xFFB347, 0.72f));
        ApplyLineDamage(telegraph.start, telegraph.end, telegraph.width, telegraph.damage, D2D1::ColorF(0xFFB347));
        AddCameraTrauma(0.36f);
        break;
    case TelegraphKind::BossPulseCircle:
        ApplyAreaDamage(telegraph.start, telegraph.radius, telegraph.damage, telegraph.color);
        AddRing(telegraph.start, telegraph.radius, 0.52f, D2D1::ColorF(telegraph.color.r, telegraph.color.g, telegraph.color.b, 0.48f), 4.0f);
        AddCameraTrauma(0.42f);
        break;
    case TelegraphKind::BossReinforce:
        SpawnStageReinforcement(m_selectedStage == 9 ? EnemyUnit::Comet : StageBossType(), kEnemyBaseX - telegraph.start.x, false);
        AddRing(telegraph.start, telegraph.radius, 0.42f, D2D1::ColorF(0xFF9BA8, 0.48f), 3.0f);
        break;
    }
}

void PawlineGameImpl::ApplyAreaDamage(Vec2 center, float radius, float damage, D2D1_COLOR_F color)
{
    for (Unit& unit : m_units)
    {
        if (!unit.alive || Distance(unit.pos, center) > radius)
        {
            continue;
        }
        DamageUnit(unit, damage, unit.team == Team::Player ? Team::Enemy : Team::Player);
        AddParticleEx(unit.pos, {0.0f, -24.0f}, 7.0f, 0.32f, D2D1::ColorF(color.r, color.g, color.b, 0.36f), ParticleKind::Glow, 0.0f, 0.90f, 20.0f);
    }
    AddRing(center, std::min(radius, 360.0f), 0.45f, D2D1::ColorF(color.r, color.g, color.b, 0.34f), 3.0f);
}

void PawlineGameImpl::ApplyLineDamage(Vec2 start, Vec2 end, float width, float damage, D2D1_COLOR_F color)
{
    // 직선형 경고는 선분과 유닛 사이의 최단거리로 판정한다.
    const Vec2 line = end - start;
    const float lengthSq = std::max(1.0f, line.x * line.x + line.y * line.y);
    for (Unit& unit : m_units)
    {
        if (!unit.alive)
        {
            continue;
        }

        const Vec2 fromStart = unit.pos - start;
        const float t = Clamp01((fromStart.x * line.x + fromStart.y * line.y) / lengthSq);
        const Vec2 closest = start + line * t;
        if (Distance(unit.pos, closest) <= width * 0.5f + unit.radius)
        {
            DamageUnit(unit, damage, unit.team == Team::Player ? Team::Enemy : Team::Player);
            AddParticleEx(unit.pos, {0.0f, -26.0f}, 8.0f, 0.34f, D2D1::ColorF(color.r, color.g, color.b, 0.42f), ParticleKind::Glow, 0.0f, 0.90f, 22.0f);
        }
    }
}

void PawlineGameImpl::SpawnStageReinforcement(EnemyUnit type, float forwardOffset, bool elite)
{
    SpawnEnemy(type, elite);
    if (m_units.empty())
    {
        return;
    }

    Unit& unit = m_units.back();
    unit.pos.x = std::max(kPlayerBaseX + 360.0f, kEnemyBaseX - forwardOffset);
    unit.pos.y = RandomLaneY();
    AddRing(unit.pos, elite ? 108.0f : 78.0f, 0.42f, D2D1::ColorF(GetEnemyStats(type, ThreatLevel()).accent.r, GetEnemyStats(type, ThreatLevel()).accent.g, GetEnemyStats(type, ThreatLevel()).accent.b, 0.44f), elite ? 3.6f : 2.6f);
}

float PawlineGameImpl::ThreatLevel() const
{
    return (1.0f + m_stageTime / 34.0f) * CurrentStage().threatScale * DifficultyThreatMultiplier();
}

float PawlineGameImpl::MaxEnergy() const
{
    return 520.0f + static_cast<float>(m_walletLevel - 1) * 230.0f;
}

float PawlineGameImpl::EnergyRegen() const
{
    const float medicBoost = HasLoadoutUnit(PlayerUnit::Mint) ? 4.0f : 0.0f;
    return 34.0f + static_cast<float>(m_walletLevel - 1) * 15.0f + medicBoost;
}

int PawlineGameImpl::WalletUpgradeCost() const
{
    if (m_walletLevel >= 5)
    {
        return 0;
    }
    return 180 + m_walletLevel * 145;
}

float PawlineGameImpl::WalletUnitBoost() const
{
    return 1.0f + static_cast<float>(m_walletLevel - 1) * 0.05f;
}

int PawlineGameImpl::UnitEnergyCost(PlayerUnit unit) const
{
    const UnitStats stats = PlayerStats(unit);
    const float discount = std::max(0.78f, 1.0f - static_cast<float>(m_walletLevel - 1) * 0.045f);
    return std::max(20, static_cast<int>(std::ceil(static_cast<float>(stats.cost) * discount)));
}

float PawlineGameImpl::UnitCooldown(PlayerUnit unit) const
{
    const UnitStats stats = PlayerStats(unit);
    const float cooldownScale = std::max(0.76f, 1.0f - static_cast<float>(m_walletLevel - 1) * 0.055f);
    return stats.cooldown * cooldownScale;
}

float PawlineGameImpl::WalletPulseInterval() const
{
    return std::max(8.0f, 18.0f - static_cast<float>(m_walletLevel - 1) * 2.2f);
}

bool PawlineGameImpl::HasLoadoutUnit(PlayerUnit unit) const
{
    for (PlayerUnit item : m_loadout)
    {
        if (item == unit)
        {
            return true;
        }
    }
    return false;
}

float PawlineGameImpl::SynergyHpMultiplier(PlayerUnit unit) const
{
    // 방어 조합: Box + Frost가 있으면 전열 유닛이 더 오래 버틴다.
    const bool guardWall = HasLoadoutUnit(PlayerUnit::Box) && HasLoadoutUnit(PlayerUnit::Frost);
    if (guardWall && (unit == PlayerUnit::Box || unit == PlayerUnit::Frost || unit == PlayerUnit::Titan))
    {
        return 1.10f;
    }
    return 1.0f;
}

float PawlineGameImpl::SynergyDamageMultiplier(PlayerUnit unit) const
{
    // 원거리 조합: Spark + Prism은 빔/저격 계열 피해를 올린다.
    const bool arcFocus = HasLoadoutUnit(PlayerUnit::Spark) && HasLoadoutUnit(PlayerUnit::Prism);
    if (arcFocus && (unit == PlayerUnit::Spark || unit == PlayerUnit::Prism || unit == PlayerUnit::Orbit || unit == PlayerUnit::Nebula))
    {
        return 1.08f;
    }

    // 돌격 조합: Dash + Comet은 빠른 근접 유닛의 공격력을 올린다.
    const bool rushPack = HasLoadoutUnit(PlayerUnit::Dash) && HasLoadoutUnit(PlayerUnit::Comet);
    if (rushPack && (unit == PlayerUnit::Dash || unit == PlayerUnit::Comet || unit == PlayerUnit::Drill))
    {
        return 1.07f;
    }
    return 1.0f;
}

float PawlineGameImpl::SynergyRangeMultiplier(PlayerUnit unit) const
{
    const bool orbitScope = HasLoadoutUnit(PlayerUnit::Orbit) && HasLoadoutUnit(PlayerUnit::Nebula);
    if (orbitScope && GetPlayerStats(unit).ranged)
    {
        return 1.06f;
    }
    return 1.0f;
}

float PawlineGameImpl::SynergySpeedMultiplier(PlayerUnit unit) const
{
    const bool rushPack = HasLoadoutUnit(PlayerUnit::Dash) && HasLoadoutUnit(PlayerUnit::Comet);
    if (rushPack && (unit == PlayerUnit::Dash || unit == PlayerUnit::Comet))
    {
        return 1.07f;
    }
    return 1.0f;
}

std::wstring PawlineGameImpl::SynergySummary() const
{
    std::vector<std::wstring> lines;
    if (HasLoadoutUnit(PlayerUnit::Box) && HasLoadoutUnit(PlayerUnit::Frost))
    {
        lines.push_back(L"GUARD WALL  FRONT HP +10%");
    }
    if (HasLoadoutUnit(PlayerUnit::Spark) && HasLoadoutUnit(PlayerUnit::Prism))
    {
        lines.push_back(L"ARC FOCUS  RANGED DMG +8%");
    }
    if (HasLoadoutUnit(PlayerUnit::Dash) && HasLoadoutUnit(PlayerUnit::Comet))
    {
        lines.push_back(L"RUSH PACK  SPEED/DMG +7%");
    }
    if (HasLoadoutUnit(PlayerUnit::Orbit) && HasLoadoutUnit(PlayerUnit::Nebula))
    {
        lines.push_back(L"STAR SCOPE  RANGE +6%");
    }
    if (HasLoadoutUnit(PlayerUnit::Mint))
    {
        lines.push_back(L"MINT SUPPLY  ENERGY REGEN +4");
    }
    if (HasLoadoutUnit(PlayerUnit::Solar) && HasLoadoutUnit(PlayerUnit::Bell))
    {
        lines.push_back(L"SUN CHIME  CANNON CHARGE +12%");
    }
    if (lines.empty())
    {
        return L"NO ACTIVE SYNERGY";
    }

    std::wstring text;
    for (int i = 0; i < static_cast<int>(lines.size()); ++i)
    {
        if (i > 0)
        {
            text += L" / ";
        }
        text += lines[i];
    }
    return text;
}

std::wstring PawlineGameImpl::GrowthRecommendation() const
{
    for (int i = 0; i < kRosterCount; ++i)
    {
        const PlayerUnit unit = static_cast<PlayerUnit>(i);
        if (!IsUnitUnlocked(unit) && m_lumen >= UnitUnlockCost(unit))
        {
            return GetPlayerStats(unit).name + L" 구매 가능";
        }
    }
    for (int i = 0; i < kRosterCount; ++i)
    {
        const PlayerUnit unit = static_cast<PlayerUnit>(i);
        if (IsUnitUnlocked(unit) && UnitLevel(unit) < kMaxUnitLevel && m_lumen >= UnitUpgradeCost(unit))
        {
            return UnitLevel(unit) == kMaxUnitLevel - 1 ? GetPlayerStats(unit).name + L" 진화 가능" : GetPlayerStats(unit).name + L" 강화 가능";
        }
    }
    return L"다음 유닛을 위해 LUMEN을 모아줘";
}

void PawlineGameImpl::UpdateWalletPulse(float dt)
{
    m_walletPulseTimer -= dt;
    if (m_walletPulseTimer > 0.0f)
    {
        return;
    }

    TriggerWalletPulse(false);
    m_walletPulseTimer = WalletPulseInterval();
}

void PawlineGameImpl::TriggerWalletPulse(bool upgradeBurst)
{
    const float energyGain = (upgradeBurst ? 72.0f : 34.0f) + static_cast<float>(m_walletLevel) * (upgradeBurst ? 18.0f : 14.0f);
    const float heal = ((upgradeBurst ? 14.0f : 5.0f) + static_cast<float>(m_walletLevel) * 6.0f) * (HasLoadoutUnit(PlayerUnit::Mint) ? 1.18f : 1.0f);
    m_energy = std::min(MaxEnergy(), m_energy + energyGain);
    m_cannonCharge = std::min(100.0f, m_cannonCharge + 3.0f + static_cast<float>(m_walletLevel) * 1.4f);

    for (Unit& unit : m_units)
    {
        if (unit.team != Team::Player || !unit.alive)
        {
            continue;
        }

        unit.hp = std::min(unit.maxHp, unit.hp + heal);
        unit.hitFlash = std::max(unit.hitFlash, 0.16f);
        AddRing(unit.pos, 32.0f + static_cast<float>(m_walletLevel) * 5.0f, 0.24f, D2D1::ColorF(0xB8FF89, 0.34f), 1.7f);
        AddSparkLines(unit.pos, D2D1::ColorF(0xB8FF89, 0.72f), upgradeBurst ? 4 : 2);
    }

    AddFloatText({kPlayerBaseX + 92.0f, kLaneY - 116.0f}, L"+" + ToWideInt(static_cast<int>(std::round(energyGain))) + L" ENERGY", D2D1::ColorF(0xB8FF89), 0.95f);
    AddRing({kPlayerBaseX + 58.0f, kLaneY}, 92.0f + static_cast<float>(m_walletLevel) * 16.0f, 0.48f, D2D1::ColorF(0xB8FF89, upgradeBurst ? 0.56f : 0.36f), 3.0f);
    AddBurst({kPlayerBaseX + 58.0f, kLaneY - 18.0f}, D2D1::ColorF(0xB8FF89), upgradeBurst ? 24 : 12);
}

void PawlineGameImpl::SpawnPlayer(PlayerUnit type)
{
    UnitStats stats = PlayerStats(type);
    const float walletBoost = WalletUnitBoost();
    Unit unit;
    unit.id = m_nextUnitId++;
    unit.team = Team::Player;
    unit.kind = static_cast<int>(type);
    unit.pos = {kPlayerBaseX + 54.0f, RandomLaneY()};
    unit.hp = stats.hp * walletBoost;
    unit.maxHp = unit.hp;
    unit.damage = stats.damage * walletBoost;
    unit.range = stats.range;
    unit.attackDelay = stats.attackDelay * std::max(0.84f, 1.0f - static_cast<float>(m_walletLevel - 1) * 0.035f);
    unit.attackTimer = 0.18f;
    unit.speed = stats.speed;
    unit.radius = stats.radius;
    unit.shakePhase = static_cast<float>(unit.id) * 0.37f;
    unit.ranged = stats.ranged;
    unit.reward = 0;
    unit.animState = UnitAnimState::Move;
    m_units.push_back(unit);
    AddBurst(unit.pos, stats.accent, 10);
    AddRing(unit.pos, 42.0f, 0.28f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.38f), 2.0f);
    if (m_walletLevel > 1)
    {
        AddRing(unit.pos, 52.0f + static_cast<float>(m_walletLevel) * 5.0f, 0.34f, D2D1::ColorF(0xB8FF89, 0.28f), 2.0f);
        AddFloatText(unit.pos + Vec2{0.0f, -unit.radius - 34.0f}, L"Wallet +" + ToWideInt(static_cast<int>(std::round((walletBoost - 1.0f) * 100.0f))) + L"%", D2D1::ColorF(0xB8FF89), 0.82f);
    }
}

void PawlineGameImpl::SpawnEnemy(EnemyUnit type, bool elite)
{
    UnitStats stats = GetEnemyStats(type, ThreatLevel());
    Unit unit;
    unit.id = m_nextUnitId++;
    unit.team = Team::Enemy;
    unit.kind = static_cast<int>(type);
    unit.pos = {kEnemyBaseX - 58.0f, RandomLaneY()};
    unit.hp = elite ? stats.hp * 1.85f : stats.hp;
    unit.maxHp = unit.hp;
    unit.damage = elite ? stats.damage * 1.25f : stats.damage;
    unit.range = stats.range;
    unit.attackDelay = stats.attackDelay;
    unit.attackTimer = 0.25f;
    unit.speed = elite ? stats.speed * 0.92f : stats.speed;
    unit.radius = elite ? stats.radius + 4.0f : stats.radius;
    unit.shakePhase = static_cast<float>(unit.id) * 0.37f;
    unit.ranged = stats.ranged;
    unit.reward = elite ? stats.reward * 2 : stats.reward;
    unit.elite = elite;
    unit.animState = elite ? UnitAnimState::Windup : UnitAnimState::Move;
    m_units.push_back(unit);
    AddBurst(unit.pos, stats.accent, elite || type == EnemyUnit::Boss ? 20 : 8);
    AddRing(unit.pos, elite ? 86.0f : 38.0f, elite ? 0.46f : 0.25f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, elite ? 0.48f : 0.28f), elite ? 3.2f : 1.8f);
}

float PawlineGameImpl::RandomLaneY()
{
    std::uniform_real_distribution<float> dist(-58.0f, 58.0f);
    return kLaneY + dist(m_rng);
}

void PawlineGameImpl::UpdateUnits(float dt)
{
    // Unit animation is state-driven. Combat still owns the decisions, but
    // rendering can now read a clear Idle/Move/Windup/Attack/Recover/Hit state.
    for (Unit& unit : m_units)
    {
        unit.attackTimer = std::max(0.0f, unit.attackTimer - dt);
        unit.hitFlash = std::max(0.0f, unit.hitFlash - dt);
        unit.shakeTimer = std::max(0.0f, unit.shakeTimer - dt);
        unit.attackAnim = std::max(0.0f, unit.attackAnim - dt);
        unit.stateTime += dt;
    }

    for (int i = 0; i < static_cast<int>(m_units.size()); ++i)
    {
        Unit& unit = m_units[i];
        if (!unit.alive)
        {
            continue;
        }

        const Vec2 before = unit.pos;
        const int targetIndex = FindTargetIndex(unit);
        const bool baseInRange = IsEnemyBaseInRange(unit);
        const bool hasTarget = targetIndex >= 0 || baseInRange;

        if (hasTarget && unit.attackTimer <= 0.0f)
        {
            if (targetIndex >= 0)
            {
                AttackUnit(unit, m_units[targetIndex]);
            }
            else
            {
                AttackBase(unit);
            }
            unit.attackTimer = unit.attackDelay;
        }

        if (!hasTarget && !IsBlocked(unit))
        {
            const float dir = unit.team == Team::Player ? 1.0f : -1.0f;
            unit.pos.x += dir * unit.speed * StageMoveSpeedModifier(unit) * dt;
        }

        const bool moved = Distance(before, unit.pos) > 0.01f;
        if (moved)
        {
            unit.walkCycle += dt * (4.4f + unit.speed * 0.045f);
        }

        if (unit.hitFlash > 0.0f)
        {
            SetUnitAnimState(unit, UnitAnimState::Hit);
        }
        else if (unit.attackAnim > 0.0f)
        {
            SetUnitAnimState(unit, ResolveAttackAnimState(unit));
        }
        else
        {
            SetUnitAnimState(unit, moved ? UnitAnimState::Move : UnitAnimState::Idle);
        }
    }
}

void PawlineGameImpl::SetUnitAnimState(Unit& unit, UnitAnimState state)
{
    if (unit.animState == state)
    {
        return;
    }

    unit.animState = state;
    unit.stateTime = 0.0f;
}

UnitAnimState PawlineGameImpl::ResolveAttackAnimState(const Unit& unit) const
{
    const float progress = AttackProgress(unit);
    if (progress < 0.40f)
    {
        return UnitAnimState::Windup;
    }
    if (progress < 0.64f)
    {
        return UnitAnimState::Attack;
    }
    return UnitAnimState::Recover;
}

int PawlineGameImpl::FindTargetIndex(const Unit& unit) const
{
    int bestIndex = -1;
    float bestDistance = 100000.0f;
    const float dir = unit.team == Team::Player ? 1.0f : -1.0f;

    for (int i = 0; i < static_cast<int>(m_units.size()); ++i)
    {
        const Unit& other = m_units[i];
        if (!other.alive || other.team == unit.team)
        {
            continue;
        }

        const float forward = (other.pos.x - unit.pos.x) * dir;
        const float laneDelta = std::abs(other.pos.y - unit.pos.y);
        if (forward < -8.0f || forward > EffectiveUnitRange(unit) + other.radius || laneDelta > kLaneHalfHeight)
        {
            continue;
        }

        if (forward < bestDistance)
        {
            bestDistance = forward;
            bestIndex = i;
        }
    }

    return bestIndex;
}

bool PawlineGameImpl::IsBlocked(const Unit& unit) const
{
    const float dir = unit.team == Team::Player ? 1.0f : -1.0f;
    for (const Unit& other : m_units)
    {
        if (!other.alive || other.team == unit.team)
        {
            continue;
        }

        const float forward = (other.pos.x - unit.pos.x) * dir;
        const float laneDelta = std::abs(other.pos.y - unit.pos.y);
        if (forward >= 0.0f && forward <= unit.radius + other.radius + 7.0f && laneDelta < 42.0f)
        {
            return true;
        }
    }
    return false;
}

bool PawlineGameImpl::IsEnemyBaseInRange(const Unit& unit) const
{
    if (unit.team == Team::Player)
    {
        return kEnemyBaseX - unit.pos.x <= EffectiveUnitRange(unit) + 52.0f;
    }
    return unit.pos.x - kPlayerBaseX <= EffectiveUnitRange(unit) + 52.0f;
}

void PawlineGameImpl::BeginAttack(Unit& attacker, Vec2 targetPos)
{
    attacker.attackDir = targetPos.x >= attacker.pos.x ? 1.0f : -1.0f;
    float duration = attacker.ranged ? 0.42f : 0.34f;
    if (attacker.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(attacker.kind))
        {
        case PlayerUnit::Paw:
            duration = 0.32f;
            break;
        case PlayerUnit::Box:
            duration = 0.52f;
            break;
        case PlayerUnit::Spark:
            duration = 0.62f;
            break;
        case PlayerUnit::Titan:
        case PlayerUnit::Solar:
            duration = 0.72f;
            break;
        case PlayerUnit::Drill:
            duration = 0.58f;
            break;
        case PlayerUnit::Dash:
        case PlayerUnit::Comet:
            duration = 0.24f;
            break;
        case PlayerUnit::Bell:
        case PlayerUnit::Mint:
            duration = 0.48f;
            break;
        case PlayerUnit::Orbit:
            duration = 0.66f;
            break;
        case PlayerUnit::Prism:
        case PlayerUnit::Nebula:
            duration = 0.76f;
            break;
        default:
            break;
        }
        if (IsUnitEvolved(static_cast<PlayerUnit>(attacker.kind)))
        {
            duration *= 0.92f;
        }
    }
    else
    {
        switch (static_cast<EnemyUnit>(attacker.kind))
        {
        case EnemyUnit::Boss:
            duration = 0.78f;
            break;
        case EnemyUnit::Quake:
            duration = 0.70f;
            break;
        case EnemyUnit::Storm:
            duration = 0.66f;
            break;
        case EnemyUnit::Brute:
            duration = 0.58f;
            break;
        case EnemyUnit::Sulfur:
        case EnemyUnit::Spore:
        case EnemyUnit::Ring:
        case EnemyUnit::Tide:
        case EnemyUnit::Void:
            duration = 0.54f;
            break;
        case EnemyUnit::Comet:
        case EnemyUnit::Flare:
        case EnemyUnit::Skitter:
            duration = 0.24f;
            break;
        case EnemyUnit::Frost:
            duration = 0.30f;
            break;
        case EnemyUnit::Rust:
        case EnemyUnit::Mirror:
            duration = 0.42f;
            break;
        default:
            break;
        }
    }

    // 공격 딜레이보다 애니메이션이 길어지지 않게 막아, 빠른 유닛은 빠른 리듬을 유지한다.
    duration = std::max(0.18f, std::min(duration, attacker.attackDelay * 0.86f));
    attacker.attackAnimMax = duration;
    attacker.attackAnim = attacker.attackAnimMax;
    SetUnitAnimState(attacker, UnitAnimState::Windup);
}

void PawlineGameImpl::AddAttackVfx(const Unit& attacker, Vec2 targetPos, D2D1_COLOR_F color)
{
    const Vec2 dir = Normalize(targetPos - attacker.pos);
    const Vec2 muzzle = attacker.pos + dir * (attacker.radius + 10.0f);

    if (attacker.team == Team::Player)
    {
        const PlayerUnit type = static_cast<PlayerUnit>(attacker.kind);
        switch (type)
        {
        case PlayerUnit::Paw:
            AddSparkLines(targetPos, D2D1::ColorF(0xEAF7FF), 5);
            AddBeam(attacker.pos + dir * 18.0f, targetPos, 2.6f, 0.10f, D2D1::ColorF(0xEAF7FF, 0.48f));
            AddRing(targetPos, 34.0f, 0.18f, D2D1::ColorF(0x65B8FF, 0.28f), 1.8f);
            break;
        case PlayerUnit::Spark:
            AddBeam(muzzle + Vec2{0.0f, -26.0f}, targetPos, 4.8f, 0.18f, D2D1::ColorF(0xF6FF83, 0.76f));
            AddSparkLines(targetPos, D2D1::ColorF(0xF6FF83), 16);
            AddRing(attacker.pos + Vec2{0.0f, -34.0f}, 42.0f, 0.22f, D2D1::ColorF(0xF6FF83, 0.34f), 2.0f);
            break;
        case PlayerUnit::Dash:
            AddBeam(attacker.pos - dir * 34.0f, targetPos, 4.8f, 0.13f, D2D1::ColorF(0xB8FF89, 0.66f));
            AddSparkLines(targetPos, D2D1::ColorF(0xB8FF89), 9);
            break;
        case PlayerUnit::Comet:
            AddBeam(attacker.pos - dir * 58.0f, targetPos + dir * 16.0f, 6.2f, 0.18f, D2D1::ColorF(0xFFCA7A, 0.72f));
            AddRing(targetPos, 48.0f, 0.22f, D2D1::ColorF(0xFFB347, 0.34f), 2.4f);
            AddSparkLines(targetPos, D2D1::ColorF(0xFFCA7A), 12);
            break;
        case PlayerUnit::Box:
            AddRing(targetPos, 62.0f, 0.30f, D2D1::ColorF(0xFFF0B5, 0.38f), 3.6f);
            AddBurst(targetPos, D2D1::ColorF(0xDCA85B), 8);
            break;
        case PlayerUnit::Titan:
            AddRing(targetPos, 92.0f, 0.36f, D2D1::ColorF(0xFFF0B5, 0.48f), 5.0f);
            AddSparkLines(targetPos, D2D1::ColorF(0xFFF0B5), 14);
            AddBurst(targetPos, D2D1::ColorF(0xDCA85B), 14);
            break;
        case PlayerUnit::Bell:
            AddRing(attacker.pos, 62.0f, 0.30f, D2D1::ColorF(0xF6FF83, 0.30f), 2.2f);
            AddRing(targetPos, 74.0f, 0.32f, D2D1::ColorF(0xF6FF83, 0.34f), 2.4f);
            AddBeam(muzzle, targetPos, 2.8f, 0.14f, D2D1::ColorF(0xF6FF83, 0.42f));
            break;
        case PlayerUnit::Orbit:
            AddRing(attacker.pos, 86.0f, 0.34f, D2D1::ColorF(0xC7D8FF, 0.36f), 2.2f);
            AddBeam(muzzle, targetPos, 4.2f, 0.18f, D2D1::ColorF(0xC7D8FF, 0.70f));
            AddSparkLines(targetPos, D2D1::ColorF(0xC7D8FF), 8);
            break;
        case PlayerUnit::Nebula:
            AddRing(attacker.pos, 104.0f, 0.40f, D2D1::ColorF(0xC8B7FF, 0.38f), 2.8f);
            AddBeam(muzzle, targetPos, 5.8f, 0.22f, D2D1::ColorF(0xC8B7FF, 0.76f));
            AddRing(targetPos, 78.0f, 0.34f, D2D1::ColorF(0xF7D6FF, 0.34f), 2.8f);
            break;
        case PlayerUnit::Frost:
            AddBeam(muzzle, targetPos, 4.8f, 0.18f, D2D1::ColorF(0xD9FFF8, 0.72f));
            AddRing(targetPos, 62.0f, 0.32f, D2D1::ColorF(0xB9FFF5, 0.40f), 2.7f);
            break;
        case PlayerUnit::Drill:
            AddBeam(attacker.pos, targetPos + dir * 18.0f, 7.0f, 0.15f, D2D1::ColorF(0xFFF0C8, 0.76f));
            AddSparkLines(targetPos, D2D1::ColorF(0xFFF0C8), 14);
            break;
        case PlayerUnit::Prism:
            AddBeam(muzzle, targetPos, 7.5f, 0.22f, D2D1::ColorF(0xF7D6FF, 0.80f));
            AddRing(targetPos, 66.0f, 0.30f, D2D1::ColorF(0xF7D6FF, 0.42f), 2.5f);
            break;
        case PlayerUnit::Solar:
            AddBeam(attacker.pos, targetPos, 8.0f, 0.20f, D2D1::ColorF(0xFFB347, 0.82f));
            AddRing(targetPos, 92.0f, 0.38f, D2D1::ColorF(0xFFE66D, 0.46f), 4.5f);
            AddRing(attacker.pos, 74.0f, 0.28f, D2D1::ColorF(0xFFE66D, 0.34f), 3.4f);
            break;
        case PlayerUnit::Mint:
            AddBeam(muzzle, targetPos, 3.2f, 0.16f, D2D1::ColorF(0xD8FFF3, 0.58f));
            AddRing(targetPos, 48.0f, 0.25f, D2D1::ColorF(0xD8FFF3, 0.35f), 2.0f);
            AddSparkLines(targetPos, D2D1::ColorF(0xD8FFF3), 7);
            break;
        default:
            AddBeam(attacker.pos, targetPos, 3.8f, 0.12f, D2D1::ColorF(color.r, color.g, color.b, 0.62f));
            AddSparkLines(targetPos, color, 6);
            break;
        }
        return;
    }

    const EnemyUnit type = static_cast<EnemyUnit>(attacker.kind);
    switch (type)
    {
    case EnemyUnit::Dust:
        AddSparkLines(targetPos, D2D1::ColorF(0xFF9BA8), 5);
        AddBeam(muzzle, targetPos, 2.4f, 0.10f, D2D1::ColorF(0xFF9BA8, 0.42f));
        break;
    case EnemyUnit::Brute:
        AddRing(targetPos, 70.0f, 0.32f, D2D1::ColorF(0xD8A66A, 0.38f), 4.0f);
        AddBurst(targetPos, D2D1::ColorF(0xD8A66A), 9);
        break;
    case EnemyUnit::Skitter:
        AddBeam(attacker.pos - dir * 22.0f, targetPos, 3.4f, 0.12f, D2D1::ColorF(0xFFB6C2, 0.60f));
        AddSparkLines(targetPos, D2D1::ColorF(0xFFB6C2), 8);
        break;
    case EnemyUnit::Sulfur:
        AddRing(targetPos, 58.0f, 0.35f, D2D1::ColorF(0xFFD27A, 0.34f), 2.8f);
        AddBurst(targetPos, D2D1::ColorF(0xFFD27A), 12);
        break;
    case EnemyUnit::Moss:
        AddRing(targetPos, 54.0f, 0.30f, D2D1::ColorF(0xB8FF89, 0.30f), 2.4f);
        AddSparkLines(targetPos, D2D1::ColorF(0x6BAA5C), 8);
        break;
    case EnemyUnit::Rust:
        AddBeam(muzzle, targetPos, 4.0f, 0.13f, D2D1::ColorF(0xD77A5C, 0.64f));
        AddSparkLines(targetPos, D2D1::ColorF(0xD77A5C), 9);
        break;
    case EnemyUnit::Storm:
        AddRing(attacker.pos, 76.0f, 0.34f, D2D1::ColorF(0xF1D09A, 0.32f), 2.8f);
        AddRing(targetPos, 68.0f, 0.34f, D2D1::ColorF(0xF1D09A, 0.32f), 3.0f);
        break;
    case EnemyUnit::Ring:
        AddBeam(muzzle, targetPos, 4.6f, 0.18f, D2D1::ColorF(0xE6D392, 0.72f));
        AddRing(targetPos, 58.0f, 0.26f, D2D1::ColorF(0xE6D392, 0.30f), 2.4f);
        break;
    case EnemyUnit::Frost:
        AddBeam(muzzle, targetPos, 4.2f, 0.16f, D2D1::ColorF(0xD9FFF8, 0.68f));
        AddRing(targetPos, 56.0f, 0.30f, D2D1::ColorF(0xB9FFF5, 0.34f), 2.4f);
        break;
    case EnemyUnit::Tide:
        AddRing(targetPos, 70.0f, 0.34f, D2D1::ColorF(0x75A7FF, 0.30f), 3.0f);
        AddBeam(muzzle, targetPos, 4.0f, 0.18f, D2D1::ColorF(0xBFD9FF, 0.58f));
        break;
    case EnemyUnit::Void:
        AddRing(attacker.pos, 92.0f, 0.38f, D2D1::ColorF(0xC8B7FF, 0.32f), 2.8f);
        AddBeam(muzzle, targetPos, 5.2f, 0.22f, D2D1::ColorF(0xC8B7FF, 0.70f));
        break;
    case EnemyUnit::Flare:
        AddBeam(attacker.pos - dir * 40.0f, targetPos, 5.4f, 0.16f, D2D1::ColorF(0xFFDB7A, 0.70f));
        AddSparkLines(targetPos, D2D1::ColorF(0xFFDB7A), 10);
        break;
    case EnemyUnit::Spore:
        AddRing(targetPos, 62.0f, 0.34f, D2D1::ColorF(0xFFB6E8, 0.30f), 2.6f);
        AddBurst(targetPos, D2D1::ColorF(0xFFB6E8), 10);
        break;
    case EnemyUnit::Quake:
        AddRing(targetPos, type == EnemyUnit::Boss ? 118.0f : 82.0f, 0.38f, D2D1::ColorF(color.r, color.g, color.b, 0.46f), 4.0f);
        AddBurst(targetPos, color, 9);
        break;
    case EnemyUnit::Mirror:
        AddBeam(muzzle, targetPos, 3.6f, 0.16f, D2D1::ColorF(0xEAF7FF, 0.52f));
        AddRing(attacker.pos, 58.0f, 0.28f, D2D1::ColorF(0xEAF7FF, 0.28f), 2.2f);
        break;
    case EnemyUnit::Comet:
        AddBeam(attacker.pos - dir * 56.0f, targetPos + dir * 18.0f, 5.8f, 0.17f, D2D1::ColorF(0xFFDB7A, 0.72f));
        AddSparkLines(targetPos, D2D1::ColorF(0xFFDB7A), 11);
        break;
    case EnemyUnit::Boss:
        AddRing(targetPos, 122.0f, 0.42f, D2D1::ColorF(0xFFB347, 0.44f), 5.0f);
        AddRing(attacker.pos, 128.0f, 0.44f, D2D1::ColorF(0xFF9BA8, 0.34f), 3.4f);
        AddBeam(muzzle, targetPos, 8.0f, 0.24f, D2D1::ColorF(0xFFB347, 0.78f));
        AddBurst(targetPos, D2D1::ColorF(0xFFB347), 18);
        break;
    }
}

void PawlineGameImpl::AttackUnit(Unit& attacker, Unit& target)
{
    const D2D1_COLOR_F hitColor = attacker.team == Team::Player ? D2D1::ColorF(0x65B8FF) : D2D1::ColorF(0xFF9BA8);
    BeginAttack(attacker, target.pos);
    AddAttackVfx(attacker, target.pos, hitColor);
    ShakeUnit(attacker, 0.15f);
    if (attacker.ranged)
    {
        FireProjectile(attacker, target);
        return;
    }

    DamageUnit(target, attacker.damage, attacker.team);
    AddBeam(attacker.pos, target.pos, 4.0f, 0.11f, hitColor);
    AddHitEffects(target.pos, hitColor);
}

void PawlineGameImpl::AttackBase(Unit& attacker)
{
    const Vec2 baseHit = attacker.team == Team::Player ? Vec2{kEnemyBaseX - 44.0f, attacker.pos.y} : Vec2{kPlayerBaseX + 44.0f, attacker.pos.y};
    const D2D1_COLOR_F hitColor = attacker.team == Team::Player ? D2D1::ColorF(0x65B8FF) : D2D1::ColorF(0xFF9BA8);
    BeginAttack(attacker, baseHit);
    AddAttackVfx(attacker, baseHit, hitColor);
    ShakeUnit(attacker, 0.15f);
    if (attacker.ranged)
    {
        FireProjectileAtBase(attacker);
        return;
    }

    AddBeam(attacker.pos, baseHit, 4.6f, 0.13f, hitColor);
    DamageBase(attacker.team == Team::Player ? Team::Enemy : Team::Player, attacker.damage, attacker.pos);
}

void PawlineGameImpl::FireProjectile(Unit& attacker, const Unit& target)
{
    BeginAttack(attacker, target.pos);
    Projectile projectile;
    projectile.pos = attacker.pos;
    projectile.lastPos = attacker.pos;
    projectile.targetId = target.id;
    projectile.sourceId = attacker.id;
    projectile.team = attacker.team;
    projectile.targetBase = false;
    projectile.damage = attacker.damage;
    projectile.speed = 420.0f;
    projectile.radius = 5.0f;
    projectile.color = attacker.team == Team::Player ? D2D1::ColorF(0xBA7BFF) : D2D1::ColorF(0xFF9BA8);
    m_projectiles.push_back(projectile);
    const Vec2 dir = Normalize(target.pos - attacker.pos);
    AddBeam(attacker.pos + dir * 8.0f, attacker.pos + dir * 58.0f, 2.4f, 0.10f, FadeColor(projectile.color, 0.72f));
    AddRing(attacker.pos, 28.0f, 0.16f, FadeColor(projectile.color, 0.34f), 1.6f);
}

void PawlineGameImpl::FireProjectileAtBase(Unit& attacker)
{
    const Vec2 targetPos = attacker.team == Team::Player ? Vec2{kEnemyBaseX - 40.0f, kLaneY} : Vec2{kPlayerBaseX + 40.0f, kLaneY};
    BeginAttack(attacker, targetPos);
    Projectile projectile;
    projectile.pos = attacker.pos;
    projectile.lastPos = attacker.pos;
    projectile.targetId = -1;
    projectile.sourceId = attacker.id;
    projectile.team = attacker.team;
    projectile.targetBase = true;
    projectile.damage = attacker.damage;
    projectile.speed = 430.0f;
    projectile.radius = 5.0f;
    projectile.color = attacker.team == Team::Player ? D2D1::ColorF(0xBA7BFF) : D2D1::ColorF(0xFF9BA8);
    m_projectiles.push_back(projectile);
    const Vec2 dir = Normalize(targetPos - attacker.pos);
    AddBeam(attacker.pos + dir * 8.0f, attacker.pos + dir * 62.0f, 2.6f, 0.11f, FadeColor(projectile.color, 0.72f));
    AddRing(attacker.pos, 30.0f, 0.16f, FadeColor(projectile.color, 0.34f), 1.8f);
}

void PawlineGameImpl::UpdateProjectiles(float dt)
{
    // Projectiles keep a target id instead of a raw pointer because m_units can
    // be compacted after deaths. FindUnitById validates the target each frame.
    for (Projectile& projectile : m_projectiles)
    {
        projectile.life -= dt;
        if (projectile.life <= 0.0f)
        {
            continue;
        }

        projectile.lastPos = projectile.pos;
        Vec2 targetPos = {};
        Unit* target = nullptr;
        if (projectile.targetBase)
        {
            targetPos = projectile.team == Team::Player ? Vec2{kEnemyBaseX - 40.0f, kLaneY} : Vec2{kPlayerBaseX + 40.0f, kLaneY};
        }
        else
        {
            target = FindUnitById(projectile.targetId);
            if (!target || !target->alive)
            {
                projectile.life = 0.0f;
                continue;
            }
            targetPos = target->pos;
        }

        const Vec2 toTarget = targetPos - projectile.pos;
        const float dist = Length(toTarget);
        const float step = projectile.speed * dt;
        if (dist <= step + projectile.radius + 6.0f)
        {
            projectile.pos = targetPos;
            if (projectile.targetBase)
            {
                ShakeUnitById(projectile.sourceId, 0.15f);
                AddBeam(projectile.lastPos, projectile.pos, projectile.radius * 1.7f, 0.13f, FadeColor(projectile.color, 0.88f));
                DamageBase(projectile.team == Team::Player ? Team::Enemy : Team::Player, projectile.damage, projectile.pos);
            }
            else if (target)
            {
                ShakeUnitById(projectile.sourceId, 0.15f);
                AddBeam(projectile.lastPos, projectile.pos, projectile.radius * 1.7f, 0.13f, FadeColor(projectile.color, 0.88f));
                DamageUnit(*target, projectile.damage, projectile.team);
                AddHitEffects(projectile.pos, projectile.color);
            }
            projectile.life = 0.0f;
        }
        else
        {
            projectile.pos = projectile.pos + Normalize(toTarget) * step;
        }
    }
}

Unit* PawlineGameImpl::FindUnitById(int id)
{
    for (Unit& unit : m_units)
    {
        if (unit.id == id)
        {
            return &unit;
        }
    }
    return nullptr;
}

void PawlineGameImpl::DamageUnit(Unit& target, float damage, Team sourceTeam)
{
    if (!target.alive)
    {
        return;
    }

    target.hp -= damage;
    target.hitFlash = 0.12f;
    ShakeUnit(target, 0.18f);
    AddFloatText(target.pos + Vec2{0.0f, -target.radius - 22.0f}, ToWideInt(static_cast<int>(std::round(damage))),
                 sourceTeam == Team::Player ? D2D1::ColorF(0xBBD7FF) : D2D1::ColorF(0xFFB6C2), 0.58f);

    if (target.hp <= 0.0f)
    {
        SetUnitAnimState(target, UnitAnimState::Death);
        target.alive = false;
        if (target.team == Team::Enemy)
        {
            m_energy = std::min(MaxEnergy(), m_energy + static_cast<float>(target.reward));
            m_score += target.reward * 10;
            AddFloatText(target.pos, L"+" + ToWideInt(target.reward), D2D1::ColorF(0xB8FF89), 0.9f);
        }
        AddDeathBurst(target);
    }
}

void PawlineGameImpl::DamageBase(Team baseTeam, float damage, Vec2 source)
{
    if (baseTeam == Team::Enemy)
    {
        m_enemyBaseHp -= damage;
        if (m_hitShakeEnabled)
        {
            m_enemyBaseShake = 0.20f;
        }
        AddCameraTrauma(0.28f);
        AddFloatText({kEnemyBaseX - 46.0f, kLaneY - 92.0f}, ToWideInt(static_cast<int>(std::round(damage))), D2D1::ColorF(0xBBD7FF), 0.65f);
        AddHitEffects({kEnemyBaseX - 44.0f, source.y}, D2D1::ColorF(0x65B8FF));
    }
    else
    {
        m_playerBaseHp -= damage;
        if (m_hitShakeEnabled)
        {
            m_playerBaseShake = 0.20f;
        }
        AddCameraTrauma(0.28f);
        AddFloatText({kPlayerBaseX + 46.0f, kLaneY - 92.0f}, ToWideInt(static_cast<int>(std::round(damage))), D2D1::ColorF(0xFFB6C2), 0.65f);
        AddHitEffects({kPlayerBaseX + 44.0f, source.y}, D2D1::ColorF(0xFF9BA8));
    }
}

void PawlineGameImpl::ShakeUnit(Unit& unit, float duration)
{
    if (!m_hitShakeEnabled)
    {
        return;
    }

    unit.shakeTimer = std::max(unit.shakeTimer, duration);
}

void PawlineGameImpl::ShakeUnitById(int id, float duration)
{
    Unit* unit = FindUnitById(id);
    if (unit)
    {
        ShakeUnit(*unit, duration);
    }
}

void PawlineGameImpl::AddCameraTrauma(float amount)
{
    if (!m_hitShakeEnabled)
    {
        return;
    }

    m_cameraTrauma = std::min(1.0f, std::max(m_cameraTrauma, amount));
}

void PawlineGameImpl::UpdateParticles(float dt)
{
    // Gameplay and visual effects share delta-time cleanup, so speed changes and
    // pause/menu transitions do not leave stale particles or beams behind.
    for (Particle& particle : m_particles)
    {
        particle.life -= dt;
        particle.pos = particle.pos + particle.vel * dt;
        particle.vel = particle.vel * std::pow(std::max(0.0f, particle.drag), dt * 60.0f);
        particle.vel.y += particle.gravity * dt;
        particle.radius = std::max(0.5f, particle.radius + particle.growth * dt);
        particle.spin += dt * (2.5f + std::abs(particle.vel.x) * 0.015f);
    }

    for (RingEffect& ring : m_rings)
    {
        ring.life -= dt;
        const float pct = 1.0f - Clamp01(ring.life / ring.maxLife);
        ring.radius = 8.0f + (ring.maxRadius - 8.0f) * pct;
    }

    for (BeamEffect& beam : m_beams)
    {
        beam.life -= dt;
    }

    for (SparkLine& line : m_sparkLines)
    {
        line.life -= dt;
        const Vec2 drift = (line.end - line.start) * (2.5f * dt);
        line.start = line.start + drift;
        line.end = line.end + drift;
    }

    for (FloatText& text : m_floatTexts)
    {
        text.life -= dt;
        text.pos.y -= 26.0f * dt;
    }

    for (UiPulse& pulse : m_uiPulses)
    {
        pulse.life -= dt;
        const float pct = 1.0f - Clamp01(pulse.life / pulse.maxLife);
        pulse.radius = 8.0f + (pulse.maxRadius - 8.0f) * pct;
    }
}

void PawlineGameImpl::CleanupEntities()
{
    m_units.erase(
        std::remove_if(m_units.begin(), m_units.end(), [](const Unit& unit) {
            return !unit.alive;
        }),
        m_units.end());

    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(), [](const Projectile& projectile) {
            return projectile.life <= 0.0f;
        }),
        m_projectiles.end());

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(), [](const Particle& particle) {
            return particle.life <= 0.0f;
        }),
        m_particles.end());

    m_rings.erase(
        std::remove_if(m_rings.begin(), m_rings.end(), [](const RingEffect& ring) {
            return ring.life <= 0.0f;
        }),
        m_rings.end());

    m_beams.erase(
        std::remove_if(m_beams.begin(), m_beams.end(), [](const BeamEffect& beam) {
            return beam.life <= 0.0f;
        }),
        m_beams.end());

    m_sparkLines.erase(
        std::remove_if(m_sparkLines.begin(), m_sparkLines.end(), [](const SparkLine& line) {
            return line.life <= 0.0f;
        }),
        m_sparkLines.end());

        m_floatTexts.erase(
            std::remove_if(m_floatTexts.begin(), m_floatTexts.end(), [](const FloatText& text) {
                return text.life <= 0.0f;
            }),
            m_floatTexts.end());

        m_uiPulses.erase(
            std::remove_if(m_uiPulses.begin(), m_uiPulses.end(), [](const UiPulse& pulse) {
                return pulse.life <= 0.0f;
            }),
            m_uiPulses.end());
}

void PawlineGameImpl::TrySpawnPlayer(int index)
{
    if (index < 0 || index >= kLoadoutSize || m_screen != GameScreen::Playing || m_gameOver || m_victory)
    {
        return;
    }

    PlayerUnit type = m_loadout[index];
    if (!IsUnitUnlocked(type))
    {
        SetMessage(L"잠긴 유닛이야. 상점에서 먼저 구매해줘.");
        return;
    }

    UnitStats stats = PlayerStats(type);
    const int cost = UnitEnergyCost(type);
    if (m_cardCooldowns[index] > 0.0f)
    {
        SetMessage(stats.name + L" 재소환 대기 중.");
        return;
    }
    if (m_energy < static_cast<float>(cost))
    {
        SetMessage(stats.name + L" 소환 에너지가 부족해.");
        return;
    }

    m_energy -= static_cast<float>(cost);
    m_cardCooldowns[index] = UnitCooldown(type);
    SpawnPlayer(type);
    SetMessage(stats.name + L" 출격.");
}

void PawlineGameImpl::TryUpgradeWallet()
{
    if (m_walletLevel >= 5)
    {
        SetMessage(L"Wallet is already max level.");
        return;
    }

    const int cost = WalletUpgradeCost();
    if (m_energy < static_cast<float>(cost))
    {
        SetMessage(L"Not enough energy to upgrade wallet.");
        return;
    }

    m_energy -= static_cast<float>(cost);
    ++m_walletLevel;
    m_walletPulseTimer = std::min(m_walletPulseTimer, 1.0f);
    TriggerWalletPulse(true);
    AddFloatText({640.0f, 570.0f}, L"Wallet Lv." + ToWideInt(m_walletLevel), D2D1::ColorF(0xB8FF89), 1.1f);
    AddRing({kPlayerBaseX + 58.0f, kLaneY}, 152.0f, 0.58f, D2D1::ColorF(0xB8FF89, 0.48f), 4.0f);
    SetMessage(L"Wallet upgraded. Cheaper summons, stronger units, supply pulse.");
}

void PawlineGameImpl::TryFireCannon()
{
    if (m_cannonCharge < 100.0f)
    {
        SetMessage(L"Cannon is still charging.");
        return;
    }

    m_cannonCharge = 0.0f;
    m_cannonFlash = 0.42f;
    m_screenFlash = 0.12f;
    AddCameraTrauma(0.55f);
    const float damage = 210.0f + static_cast<float>(m_walletLevel) * 48.0f;
    AddBeam({116.0f, kLaneY}, {kEnemyBaseX - 2.0f, kLaneY}, 20.0f, 0.34f, D2D1::ColorF(0xF6FF83, 0.92f));
    AddBeam({136.0f, kLaneY - 34.0f}, {kEnemyBaseX - 40.0f, kLaneY + 30.0f}, 8.0f, 0.28f, D2D1::ColorF(0xFFF4B8, 0.68f));
    AddSparkLines({(kPlayerBaseX + kEnemyBaseX) * 0.5f, kLaneY}, D2D1::ColorF(0xFFF4B8), 32);
    for (Unit& unit : m_units)
    {
        if (unit.team == Team::Enemy && unit.alive)
        {
            DamageUnit(unit, damage, Team::Player);
            unit.pos.x = std::min(unit.pos.x + 42.0f, kEnemyBaseX - 64.0f);
        }
    }
    AddBurst({(kPlayerBaseX + kEnemyBaseX) * 0.5f, kLaneY}, D2D1::ColorF(0xF6FF83), 40);
    AddRing({(kPlayerBaseX + kEnemyBaseX) * 0.5f, kLaneY}, 620.0f, 0.72f, D2D1::ColorF(0xF6FF83, 0.55f), 6.0f);
    SetMessage(L"Moonbeam cannon fired.");
}

void PawlineGameImpl::DecreaseGameSpeed()
{
    m_gameSpeed = std::max(0.5f, m_gameSpeed - 0.5f);
    SetMessage(L"Game speed x" + ToWideFloat(m_gameSpeed));
}

void PawlineGameImpl::IncreaseGameSpeed()
{
    m_gameSpeed = std::min(3.0f, m_gameSpeed + 0.5f);
    SetMessage(L"Game speed x" + ToWideFloat(m_gameSpeed));
}

void PawlineGameImpl::AddParticle(Vec2 pos, Vec2 vel, float radius, float life, D2D1_COLOR_F color)
{
    AddParticleEx(pos, vel, radius, life, color, ParticleKind::Dot, 18.0f, 0.82f, 0.0f);
}

void PawlineGameImpl::AddParticleEx(Vec2 pos, Vec2 vel, float radius, float life, D2D1_COLOR_F color, ParticleKind kind, float gravity, float drag, float growth)
{
    Particle particle;
    particle.pos = pos;
    particle.vel = vel;
    particle.radius = radius;
    particle.life = life;
    particle.maxLife = life;
    particle.color = color;
    particle.kind = kind;
    particle.gravity = gravity;
    particle.drag = drag;
    particle.growth = growth;
    particle.spin = Hash01(pos.x, pos.y, m_uiTime) * kPi * 2.0f;
    m_particles.push_back(particle);
}

void PawlineGameImpl::AddRing(Vec2 pos, float maxRadius, float life, D2D1_COLOR_F color, float width)
{
    RingEffect ring;
    ring.pos = pos;
    ring.radius = 8.0f;
    ring.maxRadius = maxRadius;
    ring.life = life;
    ring.maxLife = life;
    ring.width = width;
    ring.color = color;
    m_rings.push_back(ring);
}

void PawlineGameImpl::AddBeam(Vec2 start, Vec2 end, float width, float life, D2D1_COLOR_F color)
{
    BeamEffect beam;
    beam.start = start;
    beam.end = end;
    beam.width = width;
    beam.life = life;
    beam.maxLife = life;
    beam.color = color;
    m_beams.push_back(beam);
}

void PawlineGameImpl::AddSparkLines(Vec2 pos, D2D1_COLOR_F color, int count)
{
    std::uniform_real_distribution<float> angleDist(0.0f, kPi * 2.0f);
    std::uniform_real_distribution<float> lengthDist(22.0f, 68.0f);
    std::uniform_real_distribution<float> widthDist(1.0f, 3.0f);
    std::uniform_real_distribution<float> lifeDist(0.12f, 0.28f);
    for (int i = 0; i < count; ++i)
    {
        const float angle = angleDist(m_rng);
        const float length = lengthDist(m_rng);
        const Vec2 dir = {std::cos(angle), std::sin(angle)};
        SparkLine line;
        line.start = pos + dir * (length * 0.18f);
        line.end = pos + dir * length;
        line.width = widthDist(m_rng);
        line.life = lifeDist(m_rng);
        line.maxLife = line.life;
        line.color = color;
        m_sparkLines.push_back(line);
    }
}

void PawlineGameImpl::AddBurst(Vec2 pos, D2D1_COLOR_F color, int count)
{
    std::uniform_real_distribution<float> angleDist(0.0f, kPi * 2.0f);
    std::uniform_real_distribution<float> speedDist(42.0f, 176.0f);
    std::uniform_real_distribution<float> radiusDist(2.0f, 6.5f);
    for (int i = 0; i < count; ++i)
    {
        const float angle = angleDist(m_rng);
        const float speed = speedDist(m_rng);
        AddParticle(pos, {std::cos(angle) * speed, std::sin(angle) * speed}, radiusDist(m_rng), 0.58f, color);
    }
}

void PawlineGameImpl::AddDustPuff(Vec2 pos, D2D1_COLOR_F color, int count)
{
    std::uniform_real_distribution<float> angleDist(-kPi, 0.0f);
    std::uniform_real_distribution<float> speedDist(18.0f, 92.0f);
    std::uniform_real_distribution<float> radiusDist(5.0f, 14.0f);
    std::uniform_real_distribution<float> lifeDist(0.36f, 0.82f);
    for (int i = 0; i < count; ++i)
    {
        const float angle = angleDist(m_rng);
        const float speed = speedDist(m_rng);
        const Vec2 vel = {std::cos(angle) * speed, std::sin(angle) * speed * 0.38f};
        AddParticleEx(pos, vel, radiusDist(m_rng), lifeDist(m_rng), color, ParticleKind::Dust, -4.0f, 0.88f, 11.0f);
    }
}

void PawlineGameImpl::AddDeathBurst(const Unit& unit)
{
    const D2D1_COLOR_F color = unit.team == Team::Enemy ? D2D1::ColorF(0xFF9BA8) : D2D1::ColorF(0xBBD7FF);
    const D2D1_COLOR_F smoke = unit.team == Team::Enemy ? D2D1::ColorF(0x7D5260, 0.46f) : D2D1::ColorF(0x8FB7D8, 0.42f);
    std::uniform_real_distribution<float> angleDist(0.0f, kPi * 2.0f);
    std::uniform_real_distribution<float> speedDist(48.0f, 210.0f);
    std::uniform_real_distribution<float> shardDist(2.4f, 6.8f);

    const int shardCount = unit.elite ? 26 : 15;
    for (int i = 0; i < shardCount; ++i)
    {
        const float angle = angleDist(m_rng);
        const float speed = speedDist(m_rng);
        AddParticleEx(unit.pos, {std::cos(angle) * speed, std::sin(angle) * speed * 0.78f}, shardDist(m_rng), 0.52f, color, ParticleKind::Shard, 42.0f, 0.86f, -1.2f);
    }
    AddDustPuff({unit.pos.x, unit.pos.y + unit.radius * 0.78f}, smoke, unit.elite ? 18 : 10);
    AddRing(unit.pos, unit.elite ? 92.0f : 56.0f, 0.30f, D2D1::ColorF(color.r, color.g, color.b, 0.44f), unit.elite ? 3.8f : 2.5f);
    AddParticleEx(unit.pos, {0.0f, -18.0f}, unit.radius * 1.1f, 0.40f, D2D1::ColorF(color.r, color.g, color.b, 0.40f), ParticleKind::Glow, 0.0f, 0.92f, 42.0f);
}

void PawlineGameImpl::AddHitEffects(Vec2 pos, D2D1_COLOR_F color)
{
    AddBurst(pos, color, 14);
    AddSparkLines(pos, FadeColor(color, 0.9f), 8);
    AddRing(pos, 54.0f, 0.28f, D2D1::ColorF(color.r, color.g, color.b, 0.48f), 2.6f);
    AddDustPuff({pos.x, pos.y + 12.0f}, D2D1::ColorF(color.r, color.g, color.b, 0.28f), 4);
}

void PawlineGameImpl::AddFloatText(Vec2 pos, const std::wstring& text, D2D1_COLOR_F color, float life)
{
    FloatText item;
    item.pos = pos;
    item.text = text;
    item.color = color;
    item.life = life;
    item.maxLife = life;
    m_floatTexts.push_back(item);
}

void PawlineGameImpl::AddUiPulse(Vec2 pos, D2D1_COLOR_F color)
{
    UiPulse pulse;
    pulse.pos = pos;
    pulse.radius = 8.0f;
    pulse.maxRadius = 52.0f;
    pulse.life = 0.34f;
    pulse.maxLife = pulse.life;
    pulse.color = color;
    m_uiPulses.push_back(pulse);
}
