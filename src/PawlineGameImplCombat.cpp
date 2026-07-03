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
        m_nextBossTime += 44.0f / stage.threatScale;
        m_enemyTimer = std::min(m_enemyTimer, 1.1f);
        SetMessage(GetEnemyStats(bossType, threat).name + L" incoming.");
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
    const float interval = std::max(0.56f, stage.enemyInterval - threat * 0.11f);
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
        return L"Dustling / Needle / Tin";
    case 1:
        return L"Sulfur / Mirror / Needle";
    case 2:
        return L"Moss / Spore / Tin";
    case 3:
        return L"Rust / Dust / Tin";
    case 4:
        return L"Storm / Quake / Sulfur";
    case 5:
        return L"Ring / Needle / Storm";
    case 6:
        return L"Cryo / Ring / Tide";
    case 7:
        return L"Tide / Cryo / Void";
    case 8:
        return L"Void / Quake / Rust";
    default:
        return L"Flare / Comet / Solar";
    }
}

float PawlineGameImpl::ThreatLevel() const
{
    return (1.0f + m_stageTime / 34.0f) * CurrentStage().threatScale;
}

float PawlineGameImpl::MaxEnergy() const
{
    return 520.0f + static_cast<float>(m_walletLevel - 1) * 230.0f;
}

float PawlineGameImpl::EnergyRegen() const
{
    return 34.0f + static_cast<float>(m_walletLevel - 1) * 15.0f;
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
    const float heal = (upgradeBurst ? 14.0f : 5.0f) + static_cast<float>(m_walletLevel) * 6.0f;
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
            unit.pos.x += dir * unit.speed * dt;
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
        if (forward < -8.0f || forward > unit.range + other.radius || laneDelta > kLaneHalfHeight)
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
        return kEnemyBaseX - unit.pos.x <= unit.range + 52.0f;
    }
    return unit.pos.x - kPlayerBaseX <= unit.range + 52.0f;
}

void PawlineGameImpl::BeginAttack(Unit& attacker, Vec2 targetPos)
{
    attacker.attackDir = targetPos.x >= attacker.pos.x ? 1.0f : -1.0f;
    float duration = attacker.ranged ? 0.42f : 0.34f;
    if (attacker.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(attacker.kind))
        {
        case PlayerUnit::Titan:
        case PlayerUnit::Solar:
        case PlayerUnit::Drill:
            duration = 0.46f;
            break;
        case PlayerUnit::Dash:
        case PlayerUnit::Comet:
            duration = 0.30f;
            break;
        case PlayerUnit::Prism:
        case PlayerUnit::Nebula:
            duration = 0.50f;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (static_cast<EnemyUnit>(attacker.kind))
        {
        case EnemyUnit::Boss:
        case EnemyUnit::Quake:
        case EnemyUnit::Storm:
            duration = 0.50f;
            break;
        case EnemyUnit::Comet:
        case EnemyUnit::Flare:
        case EnemyUnit::Skitter:
            duration = 0.30f;
            break;
        default:
            break;
        }
    }

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
        SetMessage(L"That unit is locked. Visit the shop.");
        return;
    }

    UnitStats stats = PlayerStats(type);
    const int cost = UnitEnergyCost(type);
    if (m_cardCooldowns[index] > 0.0f)
    {
        SetMessage(stats.name + L" is recharging.");
        return;
    }
    if (m_energy < static_cast<float>(cost))
    {
        SetMessage(L"Not enough energy for " + stats.name + L".");
        return;
    }

    m_energy -= static_cast<float>(cost);
    m_cardCooldowns[index] = UnitCooldown(type);
    SpawnPlayer(type);
    SetMessage(stats.name + L" deployed.");
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
