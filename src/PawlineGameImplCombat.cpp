#include "PawlineGameImpl.h"

namespace
{
// 전투 시스템은 유닛 종류만 알고, 렌더러는 ImageVfxKind를 받아 실제 PNG 시트를 고른다.
// 이렇게 분리하면 새 이펙트를 추가해도 공격 판정 코드를 거의 건드리지 않아도 된다.
ImageVfxKind UnitImageVfxKind(const Unit& unit)
{
    if (unit.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(unit.kind))
        {
        case PlayerUnit::Spark:
            return ImageVfxKind::Thunder;
        case PlayerUnit::Frost:
            return ImageVfxKind::Ice;
        case PlayerUnit::Comet:
        case PlayerUnit::Solar:
            return ImageVfxKind::Fire;
        case PlayerUnit::Bell:
        case PlayerUnit::Orbit:
        case PlayerUnit::Prism:
            return ImageVfxKind::Holy;
        case PlayerUnit::Nebula:
            return ImageVfxKind::Dark;
        case PlayerUnit::Mint:
            return ImageVfxKind::Heal;
        case PlayerUnit::Dash:
            return ImageVfxKind::Wind;
        case PlayerUnit::Drill:
            return ImageVfxKind::Thrust;
        case PlayerUnit::Box:
        case PlayerUnit::Titan:
            return ImageVfxKind::Earth;
        case PlayerUnit::Paw:
            return ImageVfxKind::HitFlash;
        default:
            return ImageVfxKind::Slash;
        }
    }

    switch (static_cast<EnemyUnit>(unit.kind))
    {
    case EnemyUnit::Sulfur:
    case EnemyUnit::Spore:
        return ImageVfxKind::Acid;
    case EnemyUnit::Moss:
        return ImageVfxKind::Wood;
    case EnemyUnit::Frost:
        return ImageVfxKind::Ice;
    case EnemyUnit::Tide:
        return ImageVfxKind::Water;
    case EnemyUnit::Void:
    case EnemyUnit::Boss:
        return ImageVfxKind::Dark;
    case EnemyUnit::Flare:
    case EnemyUnit::Comet:
        return ImageVfxKind::Fire;
    case EnemyUnit::Ring:
    case EnemyUnit::Mirror:
        return ImageVfxKind::Holy;
    case EnemyUnit::Brute:
    case EnemyUnit::Rust:
    case EnemyUnit::Storm:
    case EnemyUnit::Quake:
        return ImageVfxKind::Earth;
    case EnemyUnit::Skitter:
        return ImageVfxKind::Smear;
    case EnemyUnit::Dust:
        return ImageVfxKind::HitFlash;
    default:
        return ImageVfxKind::EnemySlash;
    }
}

// 투사체는 발사체 모양과 충돌 이미지가 함께 읽혀야 하므로 ProjectileVisual에서 VFX를 결정한다.
ImageVfxKind ProjectileImageVfxKind(ProjectileVisual visual, Team team)
{
    switch (visual)
    {
    case ProjectileVisual::Bolt:
        return ImageVfxKind::Thunder;
    case ProjectileVisual::BellWave:
    case ProjectileVisual::OrbitStar:
    case ProjectileVisual::PrismShard:
    case ProjectileVisual::MirrorShard:
    case ProjectileVisual::SolarSpark:
        return ImageVfxKind::Holy;
    case ProjectileVisual::NebulaOrb:
    case ProjectileVisual::VoidOrb:
        return ImageVfxKind::Dark;
    case ProjectileVisual::MintPulse:
        return ImageVfxKind::Heal;
    case ProjectileVisual::FrostShard:
        return ImageVfxKind::Ice;
    case ProjectileVisual::AcidGlob:
    case ProjectileVisual::SporeSeed:
        return ImageVfxKind::Acid;
    case ProjectileVisual::TideWave:
        return ImageVfxKind::Water;
    default:
        return team == Team::Player ? ImageVfxKind::WindHit : ImageVfxKind::HitFlash;
    }
}
}

// Gameplay simulation, combat resolution, and short-lived VFX spawning.
void PawlineGameImpl::UpdateEnemyDirector(float dt)
{
    // The director scales spawn timing from the selected stage and elapsed time,
    // giving later planets denser waves without hard-coding every spawn.
    UpdateDirectorPressure(dt);
    m_enemyTimer -= dt;
    const float threat = ThreatLevel();
    const StageDefinition stage = CurrentStage();

    // 보스는 시간표가 아니라 적 기지가 충분히 밀렸을 때 한 번만 나온다.
    const float enemyHpPct = m_enemyBaseMaxHp > 0.0f ? Clamp01(m_enemyBaseHp / m_enemyBaseMaxHp) : 1.0f;
    if (!m_bossSpawned && enemyHpPct <= BossTriggerHpRatio())
    {
        const EnemyUnit bossType = StageBossType();
        SpawnEnemy(bossType, true);
        if (!m_units.empty())
        {
            Unit& boss = m_units.back();
            boss.boss = true;
            TriggerBossEntrance(boss, GetEnemyStats(bossType, threat).accent);
        }
        m_bossSpawned = true;
        m_enemyTimer = std::min(m_enemyTimer, 1.1f);
    }

    if (m_enemyTimer > 0.0f)
    {
        return;
    }

    const int phase = static_cast<int>(m_stageTime / 22.0f);
    std::uniform_int_distribution<int> roll(0, 99);
    const int value = roll(m_rng);
    const EnemyUnit type = PickStageEnemy(value, phase);

    const bool directorElite = m_directorPressure > 0.62f && value > 88 && m_stageTime > 28.0f && type != EnemyUnit::Boss;
    SpawnEnemy(type, directorElite);
    const float difficultyInterval = m_difficulty == Difficulty::Easy ? 1.15f : (m_difficulty == Difficulty::Hard ? 0.86f : 1.0f);
    const float interval = std::max(0.48f, (stage.enemyInterval - threat * 0.11f) * difficultyInterval * DirectorSpawnMultiplier());
    m_enemyTimer += interval;
}

void PawlineGameImpl::UpdateDirectorPressure(float dt)
{
    // AI 디렉터는 현재 전선 상태를 보고 적 웨이브 압력을 미세 조정한다.
    // 플레이어가 압도하면 조금 더 빠르게, 기지가 위험하면 조금 늦게 보내서 흐름을 부드럽게 만든다.
    int playerCount = 0;
    int enemyCount = 0;
    for (const Unit& unit : m_units)
    {
        if (!unit.alive)
        {
            continue;
        }
        if (unit.team == Team::Player)
        {
            ++playerCount;
        }
        else
        {
            ++enemyCount;
        }
    }

    const float playerHpPct = m_playerBaseMaxHp > 0.0f ? Clamp01(m_playerBaseHp / m_playerBaseMaxHp) : 0.0f;
    const float enemyHpPct = m_enemyBaseMaxHp > 0.0f ? Clamp01(m_enemyBaseHp / m_enemyBaseMaxHp) : 0.0f;
    const float baseLead = (playerHpPct - enemyHpPct) * 0.68f;
    const float laneLead = static_cast<float>(playerCount - enemyCount) * 0.065f;
    const float lateGameNudge = Clamp01((m_stageTime - 48.0f) / 92.0f) * 0.16f;
    float target = std::max(-1.0f, std::min(1.0f, baseLead + laneLead + lateGameNudge));

    if (playerHpPct < 0.34f)
    {
        target -= 0.42f;
    }
    if (enemyHpPct < 0.26f && playerHpPct > 0.55f)
    {
        target += 0.24f;
    }
    target = std::max(-1.0f, std::min(1.0f, target));

    const float follow = 1.0f - std::pow(0.001f, std::min(dt, 0.05f) * 0.55f);
    m_directorPressure = Lerp(m_directorPressure, target, follow);
}

float PawlineGameImpl::DirectorSpawnMultiplier() const
{
    return std::max(0.84f, std::min(1.16f, 1.0f - m_directorPressure * 0.12f));
}

float PawlineGameImpl::BossTriggerHpRatio() const
{
    // 후반 행성은 전투 템포가 빠르므로 보스가 조금 더 일찍 개입한다.
    return std::min(0.72f, 0.66f + static_cast<float>(m_selectedStage) * 0.0065f);
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
        return value < 82 ? EnemyUnit::Void : EnemyUnit::Quake;
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
        return L"먼지졸병 / 가시러너 / 철갑병";
    case 1:
        return L"산성사수 / 거울사수 / 가시러너";
    case 2:
        return L"포자병 / 포자포병 / 철갑병";
    case 3:
        return L"녹슨망치 / 먼지졸병 / 철갑병";
    case 4:
        return L"중력방패 / 지진돌격 / 산성사수";
    case 5:
        return L"고리사수 / 가시러너 / 중력방패";
    case 6:
        return L"얼음러너 / 고리사수 / 해류사수";
    case 7:
        return L"해류사수 / 얼음러너 / 공허장갑";
    case 8:
        return L"공허 / 지진 / 녹슨";
    default:
        return L"플레어 / 혜성 / 태양";
    }
}

float PawlineGameImpl::StageThreatRating() const
{
    const StageDefinition stage = CurrentStage();
    const float intervalPressure = std::max(0.0f, 2.55f - stage.enemyInterval) * 16.0f;
    const float bossPressure = std::max(0.0f, 48.0f - stage.bossFirstTime) * 0.92f;
    const float hpPressure = stage.enemyHp * 0.010f;
    const float difficulty = m_difficulty == Difficulty::Easy ? 0.86f : (m_difficulty == Difficulty::Hard ? 1.17f : 1.0f);
    return (48.0f + hpPressure + stage.threatScale * 28.0f + intervalPressure + bossPressure) * difficulty;
}

float PawlineGameImpl::LoadoutPowerRating() const
{
    float total = 0.0f;
    for (PlayerUnit unit : m_loadout)
    {
        const UnitStats stats = PlayerStats(unit);
        const float level = 1.0f + static_cast<float>(UnitLevel(unit) - 1) * 0.135f;
        const float durability = stats.hp * 0.030f;
        const float damage = stats.damage * (stats.ranged ? 0.92f : 0.78f);
        const float control = stats.range * 0.090f + stats.speed * 0.070f;
        const float cadence = std::max(0.0f, 2.4f - stats.attackDelay) * 8.0f;
        const float costPenalty = static_cast<float>(stats.cost) * 0.018f;
        total += std::max(8.0f, durability + damage + control + cadence - costPenalty) * level;
    }

    float synergyBonus = 0.0f;
    if (HasLoadoutUnit(PlayerUnit::Box) && HasLoadoutUnit(PlayerUnit::Frost))
    {
        synergyBonus += 8.0f;
    }
    if (HasLoadoutUnit(PlayerUnit::Spark) && HasLoadoutUnit(PlayerUnit::Prism))
    {
        synergyBonus += 9.0f;
    }
    if (HasLoadoutUnit(PlayerUnit::Dash) && HasLoadoutUnit(PlayerUnit::Comet))
    {
        synergyBonus += 7.0f;
    }
    if (HasLoadoutUnit(PlayerUnit::Orbit) && HasLoadoutUnit(PlayerUnit::Nebula))
    {
        synergyBonus += 8.0f;
    }
    if (HasLoadoutUnit(PlayerUnit::Mint))
    {
        synergyBonus += 6.0f;
    }
    if (HasLoadoutUnit(PlayerUnit::Solar) && HasLoadoutUnit(PlayerUnit::Bell))
    {
        synergyBonus += 7.0f;
    }
    return total / static_cast<float>(kLoadoutSize) + synergyBonus;
}

std::wstring PawlineGameImpl::BalanceAdvice() const
{
    const float threat = StageThreatRating();
    const float power = LoadoutPowerRating();
    if (power < threat * 0.86f)
    {
        return L"상점 강화 추천";
    }
    if (power < threat * 1.02f)
    {
        return L"보통 난이도 적정";
    }
    if (power > threat * 1.30f)
    {
        return L"어려움 도전 가능";
    }
    return L"안정적인 편성";
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

std::optional<std::reference_wrapper<Unit>> PawlineGameImpl::FindBossUnit()
{
    std::optional<std::reference_wrapper<Unit>> boss;
    for (Unit& unit : m_units)
    {
        if (unit.team == Team::Enemy && unit.alive && unit.boss)
        {
            if (!boss || unit.maxHp > boss->get().maxHp)
            {
                boss = unit;
            }
        }
    }
    return boss;
}

std::optional<std::reference_wrapper<const Unit>> PawlineGameImpl::FindBossUnit() const
{
    std::optional<std::reference_wrapper<const Unit>> boss;
    for (const Unit& unit : m_units)
    {
        if (unit.team == Team::Enemy && unit.alive && unit.boss)
        {
            if (!boss || unit.maxHp > boss->get().maxHp)
            {
                boss = unit;
            }
        }
    }
    return boss;
}

void PawlineGameImpl::UpdateBossPatterns(float dt)
{
    auto bossRef = FindBossUnit();
    if (!bossRef)
    {
        m_bossPatternTimer = std::max(4.5f, 8.0f - static_cast<float>(m_selectedStage) * 0.25f);
        m_bossPhaseTwoTriggered = false;
        m_bossPhaseThreeTriggered = false;
        return;
    }

    Unit& boss = bossRef->get();
    const float hpPct = boss.maxHp > 0.0f ? Clamp01(boss.hp / boss.maxHp) : 0.0f;
    if (!m_bossPhaseTwoTriggered && hpPct <= 0.50f)
    {
        m_bossPhaseTwoTriggered = true;
        TriggerBossPhaseChange(boss, 2);
    }
    if (!m_bossPhaseThreeTriggered && hpPct <= 0.25f)
    {
        m_bossPhaseThreeTriggered = true;
        TriggerBossPhaseChange(boss, 3);
    }

    m_bossPatternTimer -= dt;
    if (m_bossPatternTimer > 0.0f)
    {
        return;
    }

    TriggerBossPattern(boss);
    const float baseDelay = m_bossPhaseThreeTriggered ? 4.4f : (m_bossPhaseTwoTriggered ? 5.8f : 7.4f);
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
        SetMessage(L"철갑 보스: 압축 파동.");
        AddTelegraph(TelegraphKind::BossPulseCircle, TelegraphShape::Circle, bossCenter, bossCenter, 190.0f, 0.0f, 1.08f, 58.0f + threat * 3.6f, D2D1::ColorF(0xCFA27B));
        return;
    case EnemyUnit::Sulfur:
        SetMessage(L"산성 보스: 산성 장막.");
        AddTelegraph(TelegraphKind::VenusFog, TelegraphShape::FullLane, {std::max(kPlayerBaseX + 90.0f, bossCenter.x - 620.0f), kLaneY}, {bossCenter.x + 120.0f, kLaneY}, 360.0f, 0.0f, 0.95f, 16.0f + threat * 1.3f, D2D1::ColorF(0xE0B16D));
        return;
    case EnemyUnit::Moss:
        SetMessage(L"포자 보스: 포자 증식.");
        AddTelegraph(TelegraphKind::BossReinforce, TelegraphShape::Circle, {std::max(kPlayerBaseX + 360.0f, bossCenter.x - 260.0f), RandomLaneY()}, bossCenter, 112.0f, 0.0f, 0.90f, 0.0f, D2D1::ColorF(0xB8FF89));
        return;
    case EnemyUnit::Rust:
        SetMessage(L"녹슨 보스: 붉은 낙하.");
        AddTelegraph(TelegraphKind::MarsMeteor, TelegraphShape::Circle, {std::max(kPlayerBaseX + 240.0f, bossCenter.x - 360.0f), RandomLaneY()}, bossCenter, 138.0f, 0.0f, 0.86f, 62.0f + threat * 3.8f, D2D1::ColorF(0xFF8B60));
        AddTelegraph(TelegraphKind::MarsMeteor, TelegraphShape::Circle, {std::max(kPlayerBaseX + 380.0f, bossCenter.x - 180.0f), RandomLaneY()}, bossCenter, 112.0f, 0.0f, 1.06f, 48.0f + threat * 3.0f, D2D1::ColorF(0xFFB08B));
        return;
    case EnemyUnit::Storm:
        SetMessage(L"중력 보스: 중력 소용돌이.");
        AddTelegraph(TelegraphKind::JupiterGravity, TelegraphShape::Circle, {std::max(kPlayerBaseX + 430.0f, bossCenter.x - 260.0f), kLaneY}, bossCenter, 330.0f, 0.0f, 1.12f, 18.0f + threat * 1.8f, D2D1::ColorF(0xD8A66A));
        return;
    case EnemyUnit::Ring:
        SetMessage(L"고리 보스: 고리탄 소환.");
        AddTelegraph(TelegraphKind::SaturnReinforce, TelegraphShape::Circle, {std::max(kPlayerBaseX + 390.0f, bossCenter.x - 300.0f), RandomLaneY()}, bossCenter, 120.0f, 0.0f, 0.88f, 0.0f, D2D1::ColorF(0xE6D392));
        AddTelegraph(TelegraphKind::BossFlareLine, TelegraphShape::Line, bossCenter, {std::max(kPlayerBaseX + 80.0f, bossCenter.x - 560.0f), bossCenter.y - 54.0f}, 0.0f, 62.0f, 1.05f, 46.0f + threat * 2.5f, D2D1::ColorF(0xE6D392));
        return;
    case EnemyUnit::Frost:
        SetMessage(L"얼음 보스: 얼음 가름.");
        AddTelegraph(TelegraphKind::UranusIce, TelegraphShape::Line, {bossCenter.x, bossCenter.y - 96.0f}, {std::max(kPlayerBaseX + 70.0f, bossCenter.x - 680.0f), bossCenter.y + 86.0f}, 0.0f, 104.0f, 0.88f, 34.0f + threat * 2.5f, D2D1::ColorF(0xD9FFF8));
        return;
    case EnemyUnit::Tide:
        SetMessage(L"해류 보스: 심해 밀물.");
        AddTelegraph(TelegraphKind::NeptuneTide, TelegraphShape::FullLane, {std::max(kPlayerBaseX + 90.0f, bossCenter.x - 620.0f), kLaneY + 28.0f}, {bossCenter.x + 120.0f, kLaneY + 28.0f}, 380.0f, 0.0f, 0.90f, 34.0f + threat * 2.4f, D2D1::ColorF(0x75A7FF));
        return;
    case EnemyUnit::Quake:
        SetMessage(L"지진 보스: 공허 균열.");
        AddTelegraph(TelegraphKind::PlutoVoid, TelegraphShape::Circle, {std::max(kPlayerBaseX + 420.0f, bossCenter.x - 260.0f), kLaneY}, bossCenter, 250.0f, 0.0f, 1.02f, 54.0f + threat * 3.2f, D2D1::ColorF(0xC8B7FF));
        return;
    case EnemyUnit::Boss:
        SetMessage(L"태양문지기: 삼중 플레어.");
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
        SetMessage(L"수성 열파.");
        AddTelegraph(TelegraphKind::MercuryHeat, TelegraphShape::FullLane, {kPlayerBaseX + 40.0f, kLaneY - 86.0f}, {kEnemyBaseX - 30.0f, kLaneY + 68.0f}, kWorldWidth, 92.0f, 0.90f, 12.0f + ThreatLevel() * 1.6f, D2D1::ColorF(0xCFA27B));
        break;
    case 1:
        SetMessage(L"금성 산성 안개: 원거리 사거리 감소.");
        AddTelegraph(TelegraphKind::VenusFog, TelegraphShape::FullLane, {m_cameraX + 64.0f, kLaneY}, {m_cameraX + kWidth - 64.0f, kLaneY}, 280.0f, 0.0f, 0.85f, 8.0f + ThreatLevel(), D2D1::ColorF(0xE0B16D));
        break;
    case 2:
        SetMessage(L"지구 보급 개화.");
        AddTelegraph(TelegraphKind::EarthBloom, TelegraphShape::Circle, {kPlayerBaseX + 72.0f, kLaneY}, {kPlayerBaseX + 72.0f, kLaneY}, 150.0f, 0.0f, 0.70f, 0.0f, D2D1::ColorF(0xB8FF89));
        break;
    case 3:
    {
        SetMessage(L"화성 운석 낙하.");
        const Vec2 impact = {xDist(m_rng), yDist(m_rng)};
        AddTelegraph(TelegraphKind::MarsMeteor, TelegraphShape::Circle, impact, impact, 154.0f, 0.0f, 1.10f, 64.0f + ThreatLevel() * 4.0f, D2D1::ColorF(0xFF8B60));
        break;
    }
    case 4:
        SetMessage(L"목성 중력 파동.");
        AddTelegraph(TelegraphKind::JupiterGravity, TelegraphShape::Circle, {m_cameraX + 640.0f, kLaneY}, {m_cameraX + 640.0f, kLaneY}, 340.0f, 0.0f, 0.95f, 0.0f, D2D1::ColorF(0xD8A66A));
        break;
    case 5:
        SetMessage(L"토성 고리 증원.");
        AddTelegraph(TelegraphKind::SaturnReinforce, TelegraphShape::Circle, {kEnemyBaseX - 330.0f, kLaneY}, {kEnemyBaseX - 330.0f, kLaneY}, 124.0f, 0.0f, 0.95f, 0.0f, D2D1::ColorF(0xE6D392));
        break;
    case 6:
        SetMessage(L"천왕성 얼음 돌풍.");
        AddTelegraph(TelegraphKind::UranusIce, TelegraphShape::Line, {m_cameraX + 58.0f, kLaneY - 92.0f}, {m_cameraX + 1210.0f, kLaneY + 76.0f}, 0.0f, 94.0f, 0.85f, 0.0f, D2D1::ColorF(0xD9FFF8));
        break;
    case 7:
        SetMessage(L"해왕성 밀물 쇄도.");
        AddTelegraph(TelegraphKind::NeptuneTide, TelegraphShape::FullLane, {m_cameraX + 64.0f, kLaneY + 32.0f}, {m_cameraX + kWidth - 64.0f, kLaneY + 32.0f}, 320.0f, 0.0f, 0.80f, 0.0f, D2D1::ColorF(0x75A7FF));
        break;
    case 8:
        SetMessage(L"명왕성 공허 일식.");
        AddTelegraph(TelegraphKind::PlutoVoid, TelegraphShape::Circle, {m_cameraX + 640.0f, kLaneY}, {m_cameraX + 640.0f, kLaneY}, 230.0f, 0.0f, 0.95f, 34.0f + ThreatLevel() * 2.2f, D2D1::ColorF(0xC8B7FF));
        break;
    default:
        SetMessage(L"태양 플레어.");
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
        lines.push_back(L"가드 월: 전열 HP +10%");
    }
    if (HasLoadoutUnit(PlayerUnit::Spark) && HasLoadoutUnit(PlayerUnit::Prism))
    {
        lines.push_back(L"아크 포커스: 원거리 피해 +8%");
    }
    if (HasLoadoutUnit(PlayerUnit::Dash) && HasLoadoutUnit(PlayerUnit::Comet))
    {
        lines.push_back(L"러시 팩: 돌격 속도/피해 +7%");
    }
    if (HasLoadoutUnit(PlayerUnit::Orbit) && HasLoadoutUnit(PlayerUnit::Nebula))
    {
        lines.push_back(L"스타 스코프: 원거리 사거리 +6%");
    }
    if (HasLoadoutUnit(PlayerUnit::Mint))
    {
        lines.push_back(L"민트 보급: 에너지 회복 +4");
    }
    if (HasLoadoutUnit(PlayerUnit::Solar) && HasLoadoutUnit(PlayerUnit::Bell))
    {
        lines.push_back(L"태양 종소리: 캐논 충전 +12%");
    }
    if (lines.empty())
    {
        struct SynergyHint
        {
            PlayerUnit first;
            PlayerUnit second;
            const wchar_t* effect;
        };
        const std::array<SynergyHint, 5> hints = {{
            {PlayerUnit::Box, PlayerUnit::Frost, L"전열 HP +10%"},
            {PlayerUnit::Spark, PlayerUnit::Prism, L"원거리 피해 +8%"},
            {PlayerUnit::Dash, PlayerUnit::Comet, L"돌격 속도/피해 +7%"},
            {PlayerUnit::Orbit, PlayerUnit::Nebula, L"원거리 사거리 +6%"},
            {PlayerUnit::Solar, PlayerUnit::Bell, L"캐논 충전 +12%"}
        }};

        for (const SynergyHint& hint : hints)
        {
            const bool hasFirst = HasLoadoutUnit(hint.first);
            const bool hasSecond = HasLoadoutUnit(hint.second);
            if (hasFirst == hasSecond)
            {
                continue;
            }

            const PlayerUnit missing = hasFirst ? hint.second : hint.first;
            return L"활성 시너지 없음\n후보: " + GetPlayerStats(hint.first).name + L" + " +
                   GetPlayerStats(hint.second).name + L" - " + hint.effect +
                   L"\n필요: " + GetPlayerStats(missing).name;
        }

        return L"활성 시너지 없음\n추천: 방패냥 + 얼음방패냥 - 전열 HP +10%";
    }

    std::wstring text;
    for (int i = 0; i < static_cast<int>(lines.size()); ++i)
    {
        if (i > 0)
        {
            text += L"\n";
        }
        text += lines[i];
    }
    return text;
}

std::wstring PawlineGameImpl::GrowthRecommendation() const
{
    // 1순위: 현재 편성에 들어간 유닛을 강화하면 바로 체감된다.
    for (PlayerUnit unit : m_loadout)
    {
        if (IsUnitUnlocked(unit) && UnitLevel(unit) < kMaxUnitLevel && m_lumen >= UnitUpgradeCost(unit))
        {
            return GetPlayerStats(unit).name + L" 강화 가능 - 현재 편성 전력 상승";
        }
    }

    // 2순위: 하나만 더 사면 시너지가 켜지는 유닛을 우선 추천한다.
    const std::array<std::pair<PlayerUnit, PlayerUnit>, 5> pairs = {{
        {PlayerUnit::Box, PlayerUnit::Frost},
        {PlayerUnit::Spark, PlayerUnit::Prism},
        {PlayerUnit::Dash, PlayerUnit::Comet},
        {PlayerUnit::Orbit, PlayerUnit::Nebula},
        {PlayerUnit::Solar, PlayerUnit::Bell}
    }};
    for (const auto& [first, second] : pairs)
    {
        const bool hasFirst = HasLoadoutUnit(first);
        const bool hasSecond = HasLoadoutUnit(second);
        if (hasFirst == hasSecond)
        {
            continue;
        }
        const PlayerUnit missing = hasFirst ? second : first;
        if (!IsUnitUnlocked(missing) && m_lumen >= UnitUnlockCost(missing))
        {
            return GetPlayerStats(missing).name + L" 구매 가능 - 시너지 완성";
        }
    }

    for (int i = 0; i < kRosterCount; ++i)
    {
        const PlayerUnit unit = static_cast<PlayerUnit>(i);
        if (!IsUnitUnlocked(unit) && m_lumen >= UnitUnlockCost(unit))
        {
            return GetPlayerStats(unit).name + L" 구매 가능 - 캐릭터 풀 확장";
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

    int bestNeed = 999999;
    std::wstring target = L"다음 유닛";
    for (PlayerUnit unit : m_loadout)
    {
        if (IsUnitUnlocked(unit) && UnitLevel(unit) < kMaxUnitLevel)
        {
            const int need = std::max(0, UnitUpgradeCost(unit) - m_lumen);
            if (need < bestNeed)
            {
                bestNeed = need;
                target = GetPlayerStats(unit).name + L" 강화";
            }
        }
    }
    for (int i = 0; i < kRosterCount; ++i)
    {
        const PlayerUnit unit = static_cast<PlayerUnit>(i);
        if (!IsUnitUnlocked(unit))
        {
            const int need = std::max(0, UnitUnlockCost(unit) - m_lumen);
            if (need < bestNeed)
            {
                bestNeed = need;
                target = GetPlayerStats(unit).name + L" 구매";
            }
        }
    }
    return target + L"까지 LUMEN " + ToWideInt(bestNeed) + L" 더 필요";
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
        AddImageVfx(upgradeBurst ? ImageVfxKind::Heal : ImageVfxKind::HealSoft, unit.pos + Vec2{0.0f, -8.0f},
                    72.0f + static_cast<float>(m_walletLevel) * 5.0f, 0.42f, D2D1::ColorF(0xB8FF89, 0.86f), 1.0f);
        AddParticleEx(unit.pos + Vec2{0.0f, -16.0f}, {0.0f, -28.0f}, 5.5f, 0.34f, D2D1::ColorF(0xB8FF89, 0.56f), ParticleKind::Glow, -4.0f, 0.92f, 14.0f);
    }

    AddFloatText({kPlayerBaseX + 92.0f, kLaneY - 116.0f}, L"+" + ToWideInt(static_cast<int>(std::round(energyGain))) + L" ENERGY", D2D1::ColorF(0xB8FF89), 0.95f);
    AddRing({kPlayerBaseX + 58.0f, kLaneY}, 92.0f + static_cast<float>(m_walletLevel) * 16.0f, 0.48f, D2D1::ColorF(0xB8FF89, upgradeBurst ? 0.56f : 0.36f), 3.0f);
    AddImageVfx(upgradeBurst ? ImageVfxKind::Heal : ImageVfxKind::HealSoft, {kPlayerBaseX + 58.0f, kLaneY - 18.0f},
                upgradeBurst ? 128.0f : 104.0f, 0.48f, D2D1::ColorF(0xB8FF89, 0.90f), 1.0f);
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
    unit.nextKnockbackPct = (type == PlayerUnit::Box || type == PlayerUnit::Titan || type == PlayerUnit::Frost || type == PlayerUnit::Solar) ? 0.36f : 0.46f;
    unit.animState = UnitAnimState::Move;
    m_units.push_back(unit);
    AddBurst(unit.pos, stats.accent, 10);
    AddRing(unit.pos, 42.0f, 0.28f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.38f), 2.0f);
    if (m_walletLevel > 1)
    {
        AddRing(unit.pos, 52.0f + static_cast<float>(m_walletLevel) * 5.0f, 0.34f, D2D1::ColorF(0xB8FF89, 0.28f), 2.0f);
        AddImageVfx(ImageVfxKind::HealSoft, unit.pos + Vec2{0.0f, -10.0f}, 72.0f, 0.36f, D2D1::ColorF(0xB8FF89, 0.76f), 1.0f);
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
    unit.boss = elite && type == EnemyUnit::Boss;
    unit.nextKnockbackPct = unit.boss ? 0.78f : (elite ? 0.64f : 0.44f);
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
        if (!unit.alive)
        {
            continue;
        }
        unit.attackTimer = std::max(0.0f, unit.attackTimer - dt);
        unit.hitFlash = std::max(0.0f, unit.hitFlash - dt);
        unit.shakeTimer = std::max(0.0f, unit.shakeTimer - dt);
        unit.attackAnim = std::max(0.0f, unit.attackAnim - dt);
        unit.stunTimer = std::max(0.0f, unit.stunTimer - dt);
        if (unit.knockbackTimer > 0.0f)
        {
            unit.knockbackTimer = std::max(0.0f, unit.knockbackTimer - dt);
            unit.pos.x += unit.knockbackVelocity * dt;
            unit.knockbackVelocity *= std::pow(0.025f, dt);
            if (unit.knockbackTimer <= 0.0f)
            {
                unit.knockbackVelocity = 0.0f;
            }
        }
        unit.pos.x = std::max(kPlayerBaseX + 42.0f, std::min(kEnemyBaseX - 42.0f, unit.pos.x));
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
        if (unit.stunTimer > 0.0f || unit.knockbackTimer > 0.0f)
        {
            unit.walkCycle += dt * 9.0f;
            SetUnitAnimState(unit, UnitAnimState::Hit);
            continue;
        }

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
    // 공격이 시작되는 순간의 공통 이펙트 진입점.
    // 여기에서 이미지 VFX, 링, 빔, 파편을 함께 생성하고 각 객체는 UpdateParticles에서 수명을 소모한다.
    const Vec2 dir = Normalize(targetPos - attacker.pos);
    const Vec2 muzzle = attacker.pos + dir * (attacker.radius + 10.0f);
    const ImageVfxKind imageKind = UnitImageVfxKind(attacker);
    const Vec2 imagePos = attacker.ranged ? muzzle : targetPos;
    AddImageVfx(imageKind, imagePos, attacker.ranged ? 72.0f : 82.0f + attacker.radius * 0.65f,
                attacker.ranged ? 0.26f : 0.22f, FadeColor(color, 0.82f), attacker.attackDir);

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
    AddMeleeClashVfx(attacker, target.pos, hitColor);
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

    AddMeleeClashVfx(attacker, baseHit, hitColor);
    AddBeam(attacker.pos, baseHit, 4.6f, 0.13f, hitColor);
    DamageBase(attacker.team == Team::Player ? Team::Enemy : Team::Player, attacker.damage, attacker.pos);
}

void PawlineGameImpl::FireProjectile(Unit& attacker, const Unit& target)
{
    BeginAttack(attacker, target.pos);
    const Vec2 muzzle = ProjectileMuzzle(attacker, target.pos);
    Projectile projectile;
    projectile.pos = muzzle;
    projectile.lastPos = muzzle;
    projectile.targetId = target.id;
    projectile.sourceId = attacker.id;
    projectile.team = attacker.team;
    projectile.targetBase = false;
    projectile.damage = attacker.damage;
    ConfigureProjectileVisual(projectile, attacker);
    m_projectiles.push_back(projectile);
    const Vec2 dir = Normalize(target.pos - muzzle);
    AddBeam(muzzle - dir * 8.0f, muzzle + dir * (58.0f + projectile.radius * 2.0f), 2.4f + projectile.radius * 0.22f, 0.13f, FadeColor(projectile.color, 0.72f));
    AddRing(muzzle, 30.0f + projectile.radius * 2.0f, 0.18f, FadeColor(projectile.color, 0.36f), 1.8f);
    AddParticleEx(muzzle, {-dir.y * 28.0f, dir.x * 28.0f}, 4.0f, 0.24f, FadeColor(projectile.color, 0.80f), ParticleKind::Glow, 0.0f, 0.88f, 16.0f);
}

void PawlineGameImpl::FireProjectileAtBase(Unit& attacker)
{
    const Vec2 targetPos = attacker.team == Team::Player ? Vec2{kEnemyBaseX - 40.0f, kLaneY} : Vec2{kPlayerBaseX + 40.0f, kLaneY};
    BeginAttack(attacker, targetPos);
    const Vec2 muzzle = ProjectileMuzzle(attacker, targetPos);
    Projectile projectile;
    projectile.pos = muzzle;
    projectile.lastPos = muzzle;
    projectile.targetId = -1;
    projectile.sourceId = attacker.id;
    projectile.team = attacker.team;
    projectile.targetBase = true;
    projectile.damage = attacker.damage;
    ConfigureProjectileVisual(projectile, attacker);
    projectile.speed += 18.0f;
    m_projectiles.push_back(projectile);
    const Vec2 dir = Normalize(targetPos - muzzle);
    AddBeam(muzzle - dir * 8.0f, muzzle + dir * (62.0f + projectile.radius * 2.0f), 2.6f + projectile.radius * 0.22f, 0.13f, FadeColor(projectile.color, 0.72f));
    AddRing(muzzle, 32.0f + projectile.radius * 2.0f, 0.18f, FadeColor(projectile.color, 0.36f), 1.8f);
    AddParticleEx(muzzle, {dir.x * 14.0f, dir.y * 14.0f - 18.0f}, 4.4f, 0.26f, FadeColor(projectile.color, 0.82f), ParticleKind::Glow, 0.0f, 0.88f, 18.0f);
}

Vec2 PawlineGameImpl::ProjectileMuzzle(const Unit& attacker, Vec2 targetPos) const
{
    const Vec2 dir = Normalize(targetPos - attacker.pos);
    const float side = attacker.team == Team::Player ? 1.0f : -1.0f;
    const Vec2 normal = {-dir.y, dir.x};
    float lift = -attacker.radius * 0.38f;
    float sideOffset = 0.0f;
    float reach = attacker.radius + 18.0f;

    if (attacker.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(attacker.kind))
        {
        case PlayerUnit::Spark:
            lift = -attacker.radius - 26.0f;
            sideOffset = 6.0f;
            reach += 8.0f;
            break;
        case PlayerUnit::Bell:
            lift = -attacker.radius - 4.0f;
            sideOffset = -10.0f;
            break;
        case PlayerUnit::Orbit:
            lift = -attacker.radius * 0.25f;
            sideOffset = 20.0f;
            reach += 12.0f;
            break;
        case PlayerUnit::Prism:
            lift = -attacker.radius - 24.0f;
            sideOffset = 14.0f;
            reach += 16.0f;
            break;
        case PlayerUnit::Nebula:
            lift = -attacker.radius * 0.20f;
            reach += 20.0f;
            break;
        case PlayerUnit::Mint:
            lift = -attacker.radius - 20.0f;
            reach += 6.0f;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (static_cast<EnemyUnit>(attacker.kind))
        {
        case EnemyUnit::Sulfur:
            lift = -attacker.radius * 0.15f;
            sideOffset = 10.0f;
            break;
        case EnemyUnit::Ring:
            lift = -attacker.radius * 0.55f;
            sideOffset = -16.0f;
            reach += 10.0f;
            break;
        case EnemyUnit::Frost:
            lift = -attacker.radius - 14.0f;
            reach += 8.0f;
            break;
        case EnemyUnit::Tide:
            lift = -attacker.radius * 0.18f;
            reach += 12.0f;
            break;
        case EnemyUnit::Void:
        case EnemyUnit::Boss:
            lift = -attacker.radius * 0.28f;
            reach += 20.0f;
            break;
        case EnemyUnit::Mirror:
            lift = -attacker.radius - 10.0f;
            sideOffset = 12.0f;
            break;
        case EnemyUnit::Flare:
        case EnemyUnit::Comet:
            lift = -attacker.radius * 0.35f;
            reach += 16.0f;
            break;
        case EnemyUnit::Spore:
            lift = -attacker.radius * 0.70f;
            sideOffset = -12.0f;
            break;
        default:
            break;
        }
    }

    return attacker.pos + dir * reach + normal * sideOffset * side + Vec2{0.0f, lift};
}

void PawlineGameImpl::ConfigureProjectileVisual(Projectile& projectile, const Unit& attacker)
{
    // 공격자의 종류를 탄의 실루엣과 속도에 연결해, 같은 원거리라도
    // 화면에서 역할이 바로 읽히게 만든다.
    projectile.age = 0.0f;
    projectile.spin = Hash01(attacker.pos.x, attacker.pos.y, m_stageTime) * kPi * 2.0f;
    projectile.wobble = Hash01(attacker.pos.y, attacker.pos.x, m_uiTime) * kPi * 2.0f;
    projectile.life = 2.8f;

    if (attacker.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(attacker.kind))
        {
        case PlayerUnit::Spark:
            projectile.visual = ProjectileVisual::Bolt;
            projectile.color = D2D1::ColorF(0xF6FF83);
            projectile.speed = 560.0f;
            projectile.radius = 5.4f;
            break;
        case PlayerUnit::Bell:
            projectile.visual = ProjectileVisual::BellWave;
            projectile.color = D2D1::ColorF(0xF2C94C);
            projectile.speed = 330.0f;
            projectile.radius = 7.8f;
            break;
        case PlayerUnit::Orbit:
            projectile.visual = ProjectileVisual::OrbitStar;
            projectile.color = D2D1::ColorF(0x88A8FF);
            projectile.speed = 405.0f;
            projectile.radius = 7.0f;
            break;
        case PlayerUnit::Prism:
            projectile.visual = ProjectileVisual::PrismShard;
            projectile.color = D2D1::ColorF(0xE19BFF);
            projectile.speed = 620.0f;
            projectile.radius = 6.0f;
            break;
        case PlayerUnit::Nebula:
            projectile.visual = ProjectileVisual::NebulaOrb;
            projectile.color = D2D1::ColorF(0x9D83FF);
            projectile.speed = 360.0f;
            projectile.radius = 10.5f;
            break;
        case PlayerUnit::Mint:
            projectile.visual = ProjectileVisual::MintPulse;
            projectile.color = D2D1::ColorF(0x61E6B0);
            projectile.speed = 440.0f;
            projectile.radius = 6.8f;
            break;
        default:
            projectile.visual = ProjectileVisual::Bolt;
            projectile.color = D2D1::ColorF(0x65B8FF);
            projectile.speed = 430.0f;
            projectile.radius = 5.0f;
            break;
        }
        return;
    }

    switch (static_cast<EnemyUnit>(attacker.kind))
    {
    case EnemyUnit::Sulfur:
        projectile.visual = ProjectileVisual::AcidGlob;
        projectile.color = D2D1::ColorF(0xFFD27A);
        projectile.speed = 330.0f;
        projectile.radius = 8.0f;
        break;
    case EnemyUnit::Ring:
        projectile.visual = ProjectileVisual::OrbitStar;
        projectile.color = D2D1::ColorF(0xE6D392);
        projectile.speed = 390.0f;
        projectile.radius = 7.0f;
        break;
    case EnemyUnit::Frost:
        projectile.visual = ProjectileVisual::FrostShard;
        projectile.color = D2D1::ColorF(0xB9FFF5);
        projectile.speed = 460.0f;
        projectile.radius = 6.0f;
        break;
    case EnemyUnit::Tide:
        projectile.visual = ProjectileVisual::TideWave;
        projectile.color = D2D1::ColorF(0x75A7FF);
        projectile.speed = 350.0f;
        projectile.radius = 8.0f;
        break;
    case EnemyUnit::Void:
    case EnemyUnit::Boss:
        projectile.visual = ProjectileVisual::VoidOrb;
        projectile.color = attacker.kind == static_cast<int>(EnemyUnit::Boss) ? D2D1::ColorF(0xFFB347) : D2D1::ColorF(0xC8B7FF);
        projectile.speed = attacker.kind == static_cast<int>(EnemyUnit::Boss) ? 390.0f : 340.0f;
        projectile.radius = attacker.kind == static_cast<int>(EnemyUnit::Boss) ? 12.0f : 10.0f;
        break;
    case EnemyUnit::Flare:
    case EnemyUnit::Comet:
        projectile.visual = ProjectileVisual::SolarSpark;
        projectile.color = D2D1::ColorF(0xFFDB7A);
        projectile.speed = 520.0f;
        projectile.radius = 6.8f;
        break;
    case EnemyUnit::Spore:
        projectile.visual = ProjectileVisual::SporeSeed;
        projectile.color = D2D1::ColorF(0xFFB6E8);
        projectile.speed = 315.0f;
        projectile.radius = 8.5f;
        break;
    case EnemyUnit::Mirror:
        projectile.visual = ProjectileVisual::MirrorShard;
        projectile.color = D2D1::ColorF(0xEAF7FF);
        projectile.speed = 590.0f;
        projectile.radius = 5.6f;
        break;
    default:
        projectile.visual = ProjectileVisual::Bolt;
        projectile.color = D2D1::ColorF(0xFF9BA8);
        projectile.speed = 420.0f;
        projectile.radius = 5.2f;
        break;
    }
}

void PawlineGameImpl::AddProjectileImpact(const Projectile& projectile)
{
    const Vec2 pos = projectile.pos;
    AddBeam(projectile.lastPos, pos, projectile.radius * 1.6f, 0.13f, FadeColor(projectile.color, 0.88f));
    const ImageVfxKind imageKind = ProjectileImageVfxKind(projectile.visual, projectile.team);
    const bool healingVisual = imageKind == ImageVfxKind::Heal || imageKind == ImageVfxKind::HealSoft;
    const bool largeVisual = imageKind == ImageVfxKind::Water || imageKind == ImageVfxKind::Fire ||
                             imageKind == ImageVfxKind::Dark || imageKind == ImageVfxKind::Wind ||
                             imageKind == ImageVfxKind::Thrust;
    AddImageVfx(imageKind, pos, healingVisual ? 92.0f : (largeVisual ? 92.0f : 66.0f + projectile.radius * 3.0f),
                healingVisual ? 0.38f : 0.24f, projectile.color, projectile.team == Team::Player ? 1.0f : -1.0f);

    switch (projectile.visual)
    {
    case ProjectileVisual::Bolt:
        AddSparkLines(pos, D2D1::ColorF(0xF6FF83), 16);
        AddRing(pos, 58.0f, 0.22f, FadeColor(projectile.color, 0.42f), 2.4f);
        break;
    case ProjectileVisual::BellWave:
        AddRing(pos, 104.0f, 0.42f, FadeColor(projectile.color, 0.46f), 3.2f);
        AddRing(pos, 66.0f, 0.32f, D2D1::ColorF(0xFFF4B8, 0.32f), 2.2f);
        AddSparkLines(pos, D2D1::ColorF(0xFFF4B8), 8);
        break;
    case ProjectileVisual::OrbitStar:
        AddRing(pos, 82.0f, 0.32f, FadeColor(projectile.color, 0.42f), 2.8f);
        AddBurst(pos, projectile.color, 12);
        break;
    case ProjectileVisual::PrismShard:
    case ProjectileVisual::MirrorShard:
        AddSparkLines(pos, projectile.visual == ProjectileVisual::MirrorShard ? D2D1::ColorF(0xEAF7FF) : D2D1::ColorF(0xF7D6FF), 18);
        AddBurst(pos, projectile.color, 12);
        AddRing(pos, 62.0f, 0.26f, FadeColor(projectile.color, 0.44f), 2.6f);
        break;
    case ProjectileVisual::NebulaOrb:
    case ProjectileVisual::VoidOrb:
        AddRing(pos, 96.0f, 0.38f, FadeColor(projectile.color, 0.46f), 3.3f);
        AddRing(pos, 46.0f, 0.25f, D2D1::ColorF(0x061019, 0.48f), 4.0f);
        AddBurst(pos, projectile.color, 16);
        break;
    case ProjectileVisual::MintPulse:
        AddRing(pos, 72.0f, 0.30f, FadeColor(projectile.color, 0.44f), 2.4f);
        AddParticleEx(pos, {0.0f, -32.0f}, 10.0f, 0.42f, D2D1::ColorF(0xD8FFF3, 0.66f), ParticleKind::Glow, 0.0f, 0.90f, 22.0f);
        break;
    case ProjectileVisual::FrostShard:
        AddSparkLines(pos, D2D1::ColorF(0xD9FFF8), 14);
        AddRing(pos, 64.0f, 0.34f, D2D1::ColorF(0xB9FFF5, 0.42f), 2.4f);
        AddDustPuff({pos.x, pos.y + 12.0f}, D2D1::ColorF(0xD9FFF8, 0.22f), 7);
        break;
    case ProjectileVisual::AcidGlob:
        AddBurst(pos, D2D1::ColorF(0xFFD27A), 18);
        AddRing(pos, 58.0f, 0.33f, D2D1::ColorF(0xB8FF89, 0.36f), 2.6f);
        break;
    case ProjectileVisual::TideWave:
        AddRing(pos, 88.0f, 0.36f, D2D1::ColorF(0x75A7FF, 0.40f), 3.0f);
        for (int i = -2; i <= 2; ++i)
        {
            AddParticleEx({pos.x + static_cast<float>(i) * 10.0f, pos.y + 8.0f}, {static_cast<float>(i) * 12.0f, -34.0f}, 5.2f, 0.46f, D2D1::ColorF(0xBFD9FF, 0.70f), ParticleKind::Bubble, -18.0f, 0.90f, 4.0f);
        }
        break;
    case ProjectileVisual::SolarSpark:
        AddSparkLines(pos, D2D1::ColorF(0xFFDB7A), 20);
        AddRing(pos, 76.0f, 0.30f, D2D1::ColorF(0xFFB347, 0.46f), 3.0f);
        AddBurst(pos, D2D1::ColorF(0xFF6A3D), 12);
        break;
    case ProjectileVisual::SporeSeed:
        AddRing(pos, 66.0f, 0.36f, D2D1::ColorF(0xFFB6E8, 0.38f), 2.8f);
        AddBurst(pos, D2D1::ColorF(0xFFB6E8), 14);
        break;
    }
}

void PawlineGameImpl::AddMeleeClashVfx(const Unit& attacker, Vec2 targetPos, D2D1_COLOR_F color)
{
    const Vec2 dir = Normalize(targetPos - attacker.pos);
    const Vec2 normal = {-dir.y, dir.x};
    const Vec2 clash = attacker.pos + dir * (attacker.radius + std::min(28.0f, Distance(attacker.pos, targetPos) * 0.45f));
    const float heavy = attacker.radius >= 23.0f ? 1.0f : 0.0f;

    AddRing(clash, 42.0f + attacker.radius * 1.2f + heavy * 22.0f, 0.24f + heavy * 0.08f, FadeColor(color, 0.46f), 2.4f + heavy * 1.4f);
    AddImageVfx(UnitImageVfxKind(attacker),
                clash, 82.0f + attacker.radius * 0.9f + heavy * 22.0f, 0.22f + heavy * 0.05f, FadeColor(color, 0.90f), attacker.attackDir);
    AddParticleEx(clash, normal * 22.0f + Vec2{0.0f, -18.0f}, 6.0f + heavy * 2.0f, 0.26f, FadeColor(color, 0.70f), ParticleKind::Glow, -2.0f, 0.90f, 18.0f + heavy * 8.0f);
    AddDustPuff({clash.x, clash.y + 18.0f}, D2D1::ColorF(color.r, color.g, color.b, 0.24f), attacker.radius >= 23.0f ? 8 : 4);
    AddParticleEx(clash, dir * 28.0f + Vec2{0.0f, -22.0f}, 5.0f + heavy * 2.0f, 0.24f, FadeColor(color, 0.82f), ParticleKind::Glow, 0.0f, 0.88f, 16.0f + heavy * 8.0f);

    if (attacker.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(attacker.kind))
        {
        case PlayerUnit::Box:
        case PlayerUnit::Titan:
            AddRing({clash.x, clash.y + 20.0f}, 82.0f + attacker.radius, 0.32f, D2D1::ColorF(0xFFF0B5, 0.34f), 4.2f);
            break;
        case PlayerUnit::Dash:
        case PlayerUnit::Comet:
            AddBeam(attacker.pos - dir * 52.0f, clash + dir * 24.0f, 4.8f, 0.14f, FadeColor(color, 0.72f));
            break;
        case PlayerUnit::Drill:
            for (int i = -1; i <= 1; ++i)
            {
                AddBeam(clash - dir * 18.0f + normal * static_cast<float>(i) * 8.0f, clash + dir * 34.0f + normal * static_cast<float>(i) * 8.0f, 2.4f, 0.12f, D2D1::ColorF(0xFFF0C8, 0.62f));
            }
            break;
        case PlayerUnit::Frost:
            AddRing(clash, 64.0f, 0.30f, D2D1::ColorF(0xB9FFF5, 0.38f), 2.6f);
            break;
        case PlayerUnit::Solar:
            AddRing(clash, 92.0f, 0.34f, D2D1::ColorF(0xFFE66D, 0.42f), 4.0f);
            AddBurst(clash, D2D1::ColorF(0xFFB347), 14);
            break;
        default:
            break;
        }
        return;
    }

    switch (static_cast<EnemyUnit>(attacker.kind))
    {
    case EnemyUnit::Brute:
    case EnemyUnit::Quake:
        AddRing({clash.x, clash.y + 24.0f}, 88.0f + attacker.radius, 0.34f, D2D1::ColorF(0xD8A66A, 0.34f), 4.0f);
        break;
    case EnemyUnit::Skitter:
        AddBeam(attacker.pos - dir * 34.0f, clash + dir * 18.0f, 3.6f, 0.12f, D2D1::ColorF(0xFFB6C2, 0.62f));
        break;
    case EnemyUnit::Moss:
        AddRing(clash, 58.0f, 0.30f, D2D1::ColorF(0xB8FF89, 0.32f), 2.4f);
        break;
    case EnemyUnit::Rust:
        AddBurst(clash, D2D1::ColorF(0xD77A5C), 10);
        break;
    default:
        break;
    }
}

void PawlineGameImpl::UpdateProjectiles(float dt)
{
    // Projectiles keep a target id instead of a raw pointer because m_units can
    // be compacted after deaths. FindUnitById validates the target each frame.
    for (Projectile& projectile : m_projectiles)
    {
        projectile.life -= dt;
        projectile.age += dt;
        projectile.wobble += dt * 7.5f;
        projectile.spin += dt * (projectile.team == Team::Player ? 6.0f : -5.0f);
        if (projectile.life <= 0.0f)
        {
            continue;
        }

        projectile.lastPos = projectile.pos;
        Vec2 targetPos = {};
        std::optional<std::reference_wrapper<Unit>> target;
        if (projectile.targetBase)
        {
            targetPos = projectile.team == Team::Player ? Vec2{kEnemyBaseX - 40.0f, kLaneY} : Vec2{kPlayerBaseX + 40.0f, kLaneY};
        }
        else
        {
            target = FindUnitById(projectile.targetId);
            if (!target || !target->get().alive)
            {
                projectile.life = 0.0f;
                continue;
            }
            targetPos = target->get().pos;
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
                AddProjectileImpact(projectile);
                DamageBase(projectile.team == Team::Player ? Team::Enemy : Team::Player, projectile.damage, projectile.pos);
            }
            else if (target)
            {
                ShakeUnitById(projectile.sourceId, 0.15f);
                AddProjectileImpact(projectile);
                DamageUnit(target->get(), projectile.damage, projectile.team);
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

std::optional<std::reference_wrapper<Unit>> PawlineGameImpl::FindUnitById(int id)
{
    for (Unit& unit : m_units)
    {
        if (unit.id == id)
        {
            return unit;
        }
    }
    return std::nullopt;
}

void PawlineGameImpl::ApplyImpactReaction(Unit& target, Team sourceTeam, float damage, D2D1_COLOR_F color)
{
    // 큰 넉백 문턱 사이에서도 타격감이 보이도록 아주 짧은 반동과 바닥 먼지를 준다.
    const float damageRatio = target.maxHp > 0.0f ? Clamp01(damage / std::max(1.0f, target.maxHp * 0.18f)) : 0.0f;
    const float armor = target.boss ? 0.34f : (target.elite ? 0.55f : 1.0f);
    const float dir = sourceTeam == Team::Player ? 1.0f : -1.0f;
    const float impulse = (18.0f + damageRatio * 56.0f) * armor;
    target.knockbackVelocity = dir * std::max(std::abs(target.knockbackVelocity), impulse);
    target.knockbackTimer = std::max(target.knockbackTimer, 0.055f + damageRatio * 0.055f);
    if (damageRatio > 0.42f && !target.boss)
    {
        target.stunTimer = std::max(target.stunTimer, 0.035f + damageRatio * 0.055f);
    }

    const Vec2 ground = {target.pos.x - dir * target.radius * 0.34f, target.pos.y + target.radius * 0.72f};
    AddDustPuff(ground, D2D1::ColorF(color.r, color.g, color.b, 0.18f + damageRatio * 0.16f), damageRatio > 0.55f ? 7 : 4);
    if (damageRatio > 0.58f)
    {
        AddRing(target.pos, target.radius * (2.0f + damageRatio * 1.4f), 0.18f, FadeColor(color, 0.32f), 2.2f);
    }
}

void PawlineGameImpl::DamageUnit(Unit& target, float damage, Team sourceTeam)
{
    if (!target.alive)
    {
        return;
    }

    const bool heavyHit = damage >= std::max(42.0f, target.maxHp * (target.boss ? 0.060f : 0.120f));
    target.hp -= damage;
    target.hitFlash = 0.12f;
    ShakeUnit(target, 0.18f);
    AddFloatText(target.pos + Vec2{0.0f, -target.radius - 22.0f}, ToWideInt(static_cast<int>(std::round(damage))),
                 sourceTeam == Team::Player ? D2D1::ColorF(0xBBD7FF) : D2D1::ColorF(0xFFB6C2), 0.58f);
    ApplyImpactReaction(target, sourceTeam, damage, sourceTeam == Team::Player ? D2D1::ColorF(0xBBD7FF) : D2D1::ColorF(0xFFB6C2));

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
        TriggerHitStop(target.boss ? 0.080f : 0.046f, target.boss ? 0.32f : 0.48f, target.boss ? 0.28f : 0.14f);
        AddDeathBurst(target);
    }
    else
    {
        if (heavyHit)
        {
            TriggerHitStop(target.boss ? 0.046f : 0.028f, target.boss ? 0.46f : 0.62f, target.boss ? 0.16f : 0.08f);
        }
        CheckUnitKnockback(target, sourceTeam);
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
        AddDustPuff({kEnemyBaseX - 46.0f, kLaneY + 42.0f}, D2D1::ColorF(0x65B8FF, 0.22f), 7);
        AddRing({kEnemyBaseX - 52.0f, source.y}, 82.0f, 0.22f, D2D1::ColorF(0x65B8FF, 0.30f), 3.2f);
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
        AddDustPuff({kPlayerBaseX + 46.0f, kLaneY + 42.0f}, D2D1::ColorF(0xFF9BA8, 0.22f), 7);
        AddRing({kPlayerBaseX + 52.0f, source.y}, 82.0f, 0.22f, D2D1::ColorF(0xFF9BA8, 0.30f), 3.2f);
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
    auto unit = FindUnitById(id);
    if (unit)
    {
        ShakeUnit(unit->get(), duration);
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

    for (ImageVfx& effect : m_imageVfx)
    {
        effect.life -= dt;
        effect.pos.y -= dt * 5.0f;
        effect.size += dt * 18.0f;
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

    m_imageVfx.erase(
        std::remove_if(m_imageVfx.begin(), m_imageVfx.end(), [](const ImageVfx& effect) {
            return effect.life <= 0.0f;
        }),
        m_imageVfx.end());

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
        SetMessage(L"WALLET은 이미 최대 레벨이야.");
        return;
    }

    const int cost = WalletUpgradeCost();
    if (m_energy < static_cast<float>(cost))
    {
        SetMessage(L"WALLET 강화 에너지가 부족해.");
        return;
    }

    m_energy -= static_cast<float>(cost);
    ++m_walletLevel;
    m_walletPulseTimer = std::min(m_walletPulseTimer, 1.0f);
    TriggerWalletPulse(true);
    AddFloatText({640.0f, 570.0f}, L"WALLET Lv." + ToWideInt(m_walletLevel), D2D1::ColorF(0xB8FF89), 1.1f);
    AddRing({kPlayerBaseX + 58.0f, kLaneY}, 152.0f, 0.58f, D2D1::ColorF(0xB8FF89, 0.48f), 4.0f);
    SetMessage(L"WALLET 강화. 소환 비용 감소, 아군 강화, 보급 펄스 활성.");
}

void PawlineGameImpl::TryFireCannon()
{
    if (m_cannonCharge < 100.0f)
    {
        SetMessage(L"문빔 캐논 충전 중.");
        return;
    }

    m_cannonCharge = 0.0f;
    m_cannonFlash = 0.42f;
    m_screenFlash = m_reduceFlashes ? 0.04f : 0.12f;
    AddCameraTrauma(0.55f);
    TriggerHitStop(0.070f, 0.38f, 0.30f);
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
    SetMessage(L"문빔 캐논 발사.");
}

void PawlineGameImpl::DecreaseGameSpeed()
{
    m_gameSpeed = std::max(0.5f, m_gameSpeed - 0.5f);
    SetMessage(L"게임 속도 x" + ToWideFloat(m_gameSpeed));
}

void PawlineGameImpl::IncreaseGameSpeed()
{
    m_gameSpeed = std::min(3.0f, m_gameSpeed + 0.5f);
    SetMessage(L"게임 속도 x" + ToWideFloat(m_gameSpeed));
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
    // 선처럼 보이는 스파크를 줄이고, 충돌 지점 주변의 작은 빛 파편으로 쓰기 위해 짧게 생성한다.
    std::uniform_real_distribution<float> angleDist(0.0f, kPi * 2.0f);
    std::uniform_real_distribution<float> lengthDist(10.0f, 34.0f);
    std::uniform_real_distribution<float> widthDist(0.8f, 1.7f);
    std::uniform_real_distribution<float> lifeDist(0.10f, 0.20f);
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

void PawlineGameImpl::AddImageVfx(ImageVfxKind kind, Vec2 pos, float size, float life, D2D1_COLOR_F color, float dir)
{
    // 외부 PNG 시트를 재생하는 고수준 VFX 객체를 만든다.
    // 개별 프레임 계산은 렌더러가 맡고, 게임플레이는 위치/크기/수명만 지정한다.
    ImageVfx effect;
    effect.kind = kind;
    effect.pos = pos;
    effect.size = size;
    effect.life = life;
    effect.maxLife = life;
    effect.color = color;
    effect.dir = dir >= 0.0f ? 1.0f : -1.0f;
    effect.frameOffset = Hash01(pos.x, pos.y, m_stageTime) * 0.08f;
    m_imageVfx.push_back(effect);

    if (m_imageVfx.size() > 180)
    {
        m_imageVfx.erase(m_imageVfx.begin(), m_imageVfx.begin() + static_cast<std::ptrdiff_t>(m_imageVfx.size() - 180));
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
    AddImageVfx(ImageVfxKind::Smoke, {unit.pos.x, unit.pos.y + unit.radius * 0.35f}, unit.elite ? 116.0f : 82.0f,
                unit.elite ? 0.46f : 0.34f, FadeColor(smoke, 0.84f), unit.attackDir);
    AddImageVfx(UnitImageVfxKind(unit), unit.pos, unit.elite ? 96.0f : 68.0f, 0.24f, FadeColor(color, 0.76f), unit.attackDir);
    AddParticleEx(unit.pos, {0.0f, -18.0f}, unit.radius * 1.1f, 0.40f, D2D1::ColorF(color.r, color.g, color.b, 0.40f), ParticleKind::Glow, 0.0f, 0.92f, 42.0f);
}

void PawlineGameImpl::AddHitEffects(Vec2 pos, D2D1_COLOR_F color)
{
    AddBurst(pos, color, 8);
    AddImageVfx(ImageVfxKind::Slash, pos, 72.0f, 0.20f, FadeColor(color, 0.86f), 1.0f);
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
