#include "PawlineGameImpl.h"

// 전투 피드백 전용 파일.
// 판정 자체는 Combat.cpp가 맡고, 여기서는 히트스톱, 넉백, 보스 페이즈처럼
// 플레이어가 "맞았다/강하다/위험하다"를 느끼게 만드는 연출 상태를 관리한다.

void PawlineGameImpl::ResetCombatFeedbackState()
{
    m_hitStopTimer = 0.0f;
    m_hitStopMax = 0.0f;
    m_slowMoTimer = 0.0f;
    m_slowMoMax = 0.0f;
    m_slowMoScale = 1.0f;
    m_postFxPulse = 0.0f;
    m_bossBannerTimer = 0.0f;
    m_bossWarningTimer = 0.0f;
    m_bossPhaseBannerTimer = 0.0f;
    m_bossPhaseBannerMax = 0.0f;
    m_bossPhaseBannerLevel = 0;
    m_bossSpawned = false;
    m_bossPhaseTwoTriggered = false;
    m_bossPhaseThreeTriggered = false;
}

void PawlineGameImpl::UpdateCombatFeedbackTimers(float dt)
{
    m_hitStopTimer = std::max(0.0f, m_hitStopTimer - dt);
    m_slowMoTimer = std::max(0.0f, m_slowMoTimer - dt);
    m_postFxPulse = std::max(0.0f, m_postFxPulse - dt);
    m_bossPhaseBannerTimer = std::max(0.0f, m_bossPhaseBannerTimer - dt);

    if (m_hitStopTimer <= 0.0f)
    {
        m_hitStopMax = 0.0f;
    }
    if (m_slowMoTimer <= 0.0f)
    {
        m_slowMoMax = 0.0f;
        m_slowMoScale = 1.0f;
    }
    if (m_bossPhaseBannerTimer <= 0.0f)
    {
        m_bossPhaseBannerMax = 0.0f;
        m_bossPhaseBannerLevel = 0;
    }
}

float PawlineGameImpl::CombatTimeScale() const
{
    if (m_hitStopTimer > 0.0f)
    {
        return m_reduceFlashes ? 0.12f : 0.035f;
    }
    if (m_slowMoTimer > 0.0f)
    {
        return std::max(m_reduceFlashes ? 0.62f : 0.24f, std::min(1.0f, m_slowMoScale));
    }
    return 1.0f;
}

float PawlineGameImpl::PostFxFeedbackIntensity() const
{
    const float hit = m_hitStopMax > 0.0f ? Clamp01(m_hitStopTimer / m_hitStopMax) : 0.0f;
    const float slow = m_slowMoMax > 0.0f ? Clamp01(m_slowMoTimer / m_slowMoMax) : 0.0f;
    const float phase = m_bossPhaseBannerMax > 0.0f ? Clamp01(m_bossPhaseBannerTimer / m_bossPhaseBannerMax) : 0.0f;
    const float pulse = Clamp01(m_postFxPulse / 0.70f);
    return Clamp01(hit + slow * 0.42f + phase * 0.58f + pulse * 0.36f);
}

void PawlineGameImpl::TriggerHitStop(float holdDuration, float slowScale, float slowDuration)
{
    if (m_screen != GameScreen::Playing)
    {
        return;
    }
    if (m_reduceFlashes)
    {
        holdDuration *= 0.58f;
        slowDuration *= 0.72f;
        slowScale = std::max(slowScale, 0.62f);
    }

    holdDuration = std::max(0.0f, std::min(0.12f, holdDuration));
    slowDuration = std::max(0.0f, std::min(0.72f, slowDuration));
    slowScale = std::max(0.18f, std::min(1.0f, slowScale));

    m_hitStopTimer = std::max(m_hitStopTimer, holdDuration);
    m_hitStopMax = std::max(m_hitStopMax, holdDuration);
    if (slowDuration >= m_slowMoTimer)
    {
        m_slowMoTimer = slowDuration;
        m_slowMoMax = slowDuration;
        m_slowMoScale = slowScale;
    }
    m_postFxPulse = std::max(m_postFxPulse, std::min(0.90f, holdDuration + slowDuration));
}

void PawlineGameImpl::TriggerBossEntrance(Unit& boss, D2D1_COLOR_F color)
{
    boss.boss = true;
    boss.stunTimer = 0.0f;
    boss.knockbackTimer = 0.0f;
    boss.knockbackVelocity = 0.0f;
    boss.nextKnockbackPct = 0.82f;
    m_bossFocusX = std::max(0.0f, std::min(kCameraMaxX, boss.pos.x - 760.0f));
    m_bossBannerTimer = 3.15f;
    m_bossWarningTimer = 1.20f;
    AddCameraTrauma(0.78f);
    TriggerHitStop(0.095f, 0.34f, 0.44f);
    PlayMusicStinger(L"assets\\music\\stinger_boss.wav", 1.05f);
    PlaySfxAt(SfxKind::Boss, boss.pos.x, 0.35f, 1.25f);

    const std::wstring name = GetEnemyStats(static_cast<EnemyUnit>(boss.kind), ThreatLevel()).name;
    SetMessage(name + L" 강림. 전선 충격!");
    AddRing(boss.pos, 320.0f, 0.82f, FadeColor(color, 0.54f), 6.0f);
    AddRing({boss.pos.x - 84.0f, kLaneY}, 430.0f, 0.92f, D2D1::ColorF(0xFFB347, 0.34f), 5.5f);
    AddBeam({boss.pos.x - 260.0f, kBattleTop + 18.0f}, {boss.pos.x + 58.0f, kBattleBottom - 24.0f}, 13.0f, 0.30f, FadeColor(color, 0.62f));
    AddSparkLines(boss.pos, FadeColor(color, 0.95f), 28);
    AddDustPuff({boss.pos.x, boss.pos.y + boss.radius * 0.74f}, D2D1::ColorF(color.r, color.g, color.b, 0.26f), 24);

    for (Unit& unit : m_units)
    {
        if (unit.team != Team::Player || !unit.alive)
        {
            continue;
        }
        TriggerUnitKnockback(unit, Team::Enemy, 430.0f, 0.86f, true);
    }
}

void PawlineGameImpl::TriggerBossPhaseChange(Unit& boss, int phase)
{
    const D2D1_COLOR_F color = GetEnemyStats(static_cast<EnemyUnit>(boss.kind), ThreatLevel()).accent;
    const bool finalPhase = phase >= 3;
    m_bossPhaseBannerLevel = phase;
    m_bossPhaseBannerMax = finalPhase ? 2.35f : 1.95f;
    m_bossPhaseBannerTimer = m_bossPhaseBannerMax;
    m_bossFocusX = std::max(0.0f, std::min(kCameraMaxX, boss.pos.x - 720.0f));
    m_bossPatternTimer = finalPhase ? 0.55f : 0.95f;

    // 페이즈 전환은 능력치와 패턴 템포가 함께 바뀌어야 플레이어가 위기감을 느낀다.
    boss.damage *= finalPhase ? 1.13f : 1.08f;
    boss.attackDelay = std::max(0.52f, boss.attackDelay * (finalPhase ? 0.86f : 0.93f));
    boss.speed *= finalPhase ? 1.06f : 1.03f;
    boss.hp = std::min(boss.maxHp, boss.hp + boss.maxHp * (finalPhase ? 0.055f : 0.035f));
    boss.hitFlash = std::max(boss.hitFlash, 0.28f);
    AddFloatText(boss.pos + Vec2{0.0f, -boss.radius - 34.0f}, L"PHASE SHIELD", D2D1::ColorF(0xF6FF83), 0.86f);

    SetMessage(finalPhase ? L"보스 최종 페이즈. 전선 붕괴 주의!" : L"보스 2페이즈. 패턴 강화!");
    AddCameraTrauma(finalPhase ? 0.82f : 0.62f);
    TriggerHitStop(finalPhase ? 0.11f : 0.075f, finalPhase ? 0.26f : 0.36f, finalPhase ? 0.58f : 0.36f);
    PlaySfxAt(SfxKind::Boss, boss.pos.x, finalPhase ? 0.45f : 0.32f, finalPhase ? 1.16f : 0.96f);
    AddRing(boss.pos, finalPhase ? 270.0f : 190.0f, finalPhase ? 0.78f : 0.62f, FadeColor(color, finalPhase ? 0.62f : 0.48f), finalPhase ? 6.0f : 4.8f);
    AddRing({boss.pos.x - 92.0f, kLaneY}, finalPhase ? 520.0f : 340.0f, finalPhase ? 0.86f : 0.66f, D2D1::ColorF(0xFFB347, finalPhase ? 0.40f : 0.30f), finalPhase ? 5.6f : 4.2f);
    AddBeam({boss.pos.x - 220.0f, kBattleTop + 26.0f}, {boss.pos.x + 30.0f, kBattleBottom - 32.0f}, finalPhase ? 15.0f : 10.0f, 0.30f, FadeColor(color, 0.70f));
    AddSparkLines(boss.pos, FadeColor(color, 0.95f), finalPhase ? 34 : 22);
    AddTelegraph(TelegraphKind::BossPulseCircle, TelegraphShape::Circle, boss.pos, boss.pos, finalPhase ? 260.0f : 190.0f, 0.0f, finalPhase ? 0.82f : 0.96f, finalPhase ? 58.0f : 34.0f, color);

    const EnemyUnit helper = m_selectedStage == 9 ? EnemyUnit::Flare : StageBossType();
    SpawnStageReinforcement(helper, finalPhase ? 260.0f : 360.0f, finalPhase);

    for (Unit& unit : m_units)
    {
        if (unit.team != Team::Player || !unit.alive)
        {
            continue;
        }
        if (unit.pos.x > boss.pos.x - (finalPhase ? 780.0f : 620.0f))
        {
            TriggerUnitKnockback(unit, Team::Enemy, finalPhase ? 370.0f : 285.0f, finalPhase ? 0.72f : 0.48f, true);
        }
    }
}

float PawlineGameImpl::UnitKnockbackStep(const Unit& unit) const
{
    if (unit.boss)
    {
        return 0.22f;
    }
    if (unit.elite)
    {
        return 0.26f;
    }
    if (unit.team == Team::Player)
    {
        const PlayerUnit type = static_cast<PlayerUnit>(unit.kind);
        if (type == PlayerUnit::Box || type == PlayerUnit::Titan || type == PlayerUnit::Frost || type == PlayerUnit::Solar)
        {
            return 0.30f;
        }
        if (type == PlayerUnit::Dash || type == PlayerUnit::Comet)
        {
            return 0.24f;
        }
    }
    return 0.32f;
}

void PawlineGameImpl::CheckUnitKnockback(Unit& target, Team sourceTeam)
{
    if (!target.alive || target.maxHp <= 0.0f || target.hp <= 0.0f || target.nextKnockbackPct <= 0.0f)
    {
        return;
    }

    const float hpPct = Clamp01(target.hp / target.maxHp);
    if (hpPct > target.nextKnockbackPct)
    {
        return;
    }

    const float crossed = target.nextKnockbackPct;
    const float step = UnitKnockbackStep(target);
    while (target.nextKnockbackPct > 0.0f && hpPct <= target.nextKnockbackPct)
    {
        target.nextKnockbackPct -= step;
    }

    const float weight = target.boss ? 0.72f : (target.elite ? 0.86f : 1.0f);
    const float strength = (target.team == Team::Player ? 310.0f : 270.0f) * weight;
    const float stun = target.boss ? 0.58f : (target.elite ? 0.48f : 0.36f);
    TriggerHitStop(target.boss ? 0.060f : 0.036f, target.boss ? 0.40f : 0.54f, target.boss ? 0.22f : 0.12f);
    TriggerUnitKnockback(target, sourceTeam, strength, stun + (crossed <= 0.20f ? 0.10f : 0.0f), false);
}

void PawlineGameImpl::TriggerUnitKnockback(Unit& unit, Team sourceTeam, float strength, float stunDuration, bool force)
{
    if (!unit.alive)
    {
        return;
    }

    const float dir = sourceTeam == Team::Player ? 1.0f : -1.0f;
    const D2D1_COLOR_F color = sourceTeam == Team::Player ? D2D1::ColorF(0xBBD7FF) : D2D1::ColorF(0xFFB6C2);
    const float heavy = unit.boss ? 1.42f : (unit.elite ? 1.18f : 1.0f);
    unit.stunTimer = std::max(unit.stunTimer, stunDuration);
    unit.knockbackTimer = std::max(unit.knockbackTimer, force ? 0.34f : 0.24f);
    unit.knockbackVelocity = dir * strength;
    unit.attackAnim = 0.0f;
    unit.attackTimer = std::max(unit.attackTimer, stunDuration * 0.45f);
    unit.hitFlash = std::max(unit.hitFlash, 0.18f);
    ShakeUnit(unit, force ? 0.28f : 0.20f);
    SetUnitAnimState(unit, UnitAnimState::Hit);

    const Vec2 ground = {unit.pos.x, unit.pos.y + unit.radius * 0.72f};
    AddRing(unit.pos, unit.radius * (3.0f + heavy), force ? 0.38f : 0.27f, FadeColor(color, force ? 0.56f : 0.42f), 2.8f + heavy);
    AddBeam(unit.pos - Vec2{dir * (unit.radius + 46.0f), 0.0f}, unit.pos + Vec2{dir * (unit.radius + 8.0f), -8.0f}, 5.0f + heavy * 1.2f, 0.14f, FadeColor(color, 0.62f));
    AddSparkLines(unit.pos, FadeColor(color, 0.86f), force ? 12 : 7);
    AddDustPuff(ground, D2D1::ColorF(color.r, color.g, color.b, 0.24f), force ? 12 : 6);
    AddFloatText(unit.pos + Vec2{0.0f, -unit.radius - 38.0f}, force ? L"SHOCK" : L"STUN", color, force ? 0.86f : 0.64f);
}
