#pragma once

#include <windows.h>
#include "framework/DeltaTimer.h"
#include "GameData.h"

#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>
#include <mmsystem.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// Shared constants, light math helpers, and POD state used by the split
// PawlineGameImpl*.cpp files. Keeping them here lets combat, input, and render
// code share one object model without exposing it through the public game API.
constexpr float kWidth = 1280.0f;
constexpr float kHeight = 800.0f;
constexpr float kBattleTop = 96.0f;
constexpr float kBattleBottom = 608.0f;
constexpr float kLaneY = 358.0f;
constexpr float kLaneHalfHeight = 104.0f;
constexpr float kPlayerBaseX = 112.0f;
constexpr float kWorldWidth = 2480.0f;
constexpr float kEnemyBaseX = kWorldWidth - 112.0f;
constexpr float kCameraMaxX = kWorldWidth - kWidth;
constexpr float kPi = 3.14159265358979323846f;
constexpr int kMaxUnitLevel = 5;

template <typename T>
inline void SafeRelease(T** value)
{
    if (*value)
    {
        (*value)->Release();
        *value = nullptr;
    }
}

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(Vec2 a, float s) { return {a.x * s, a.y * s}; }

inline float Length(Vec2 v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline float Distance(Vec2 a, Vec2 b)
{
    return Length(a - b);
}

inline Vec2 Normalize(Vec2 v)
{
    const float len = Length(v);
    if (len <= 0.0001f)
    {
        return {};
    }
    return {v.x / len, v.y / len};
}

inline float Clamp01(float v)
{
    return std::max(0.0f, std::min(1.0f, v));
}

inline float Lerp(float a, float b, float t)
{
    return a + (b - a) * Clamp01(t);
}

inline float Hash01(float x, float y, float t)
{
    const float value = std::sin(x * 12.9898f + y * 78.233f + t * 37.719f) * 43758.5453f;
    return value - std::floor(value);
}

inline D2D1_COLOR_F FadeColor(D2D1_COLOR_F color, float alpha)
{
    color.a *= Clamp01(alpha);
    return color;
}

inline bool Contains(D2D1_RECT_F rect, Vec2 point)
{
    return point.x >= rect.left && point.x <= rect.right && point.y >= rect.top && point.y <= rect.bottom;
}

inline D2D1_POINT_2F Point(Vec2 v)
{
    return D2D1::Point2F(v.x, v.y);
}

inline D2D1_ELLIPSE Ellipse(Vec2 center, float radiusX, float radiusY)
{
    return D2D1::Ellipse(Point(center), radiusX, radiusY);
}

inline std::wstring ToWideInt(int value)
{
    return std::to_wstring(value);
}

inline std::wstring ToWideTime(float seconds)
{
    const int total = static_cast<int>(seconds);
    const int minutes = total / 60;
    const int remain = total % 60;
    std::wstringstream stream;
    stream << minutes << L":";
    if (remain < 10)
    {
        stream << L"0";
    }
    stream << remain;
    return stream.str();
}

inline std::wstring ToWideFloat(float value, int digits = 1)
{
    std::wstringstream stream;
    stream << std::fixed << std::setprecision(digits) << value;
    return stream.str();
}

enum class Team
{
    Player,
    Enemy
};


enum class GameScreen
{
    Title,
    Options,
    Menu,
    Shop,
    Briefing,
    Playing,
    Result
};

enum class UnitAnimState
{
    Idle,
    Move,
    Windup,
    Attack,
    Recover,
    Hit,
    Death
};

enum class ParticleKind
{
    Dot,
    Glow,
    Smoke,
    Dust,
    Shard,
    Ember,
    Snow,
    Bubble
};


// A live lane actor. Player and enemy units share one structure so combat code
// can resolve target search, movement, attack timing, and hit feedback uniformly.
struct Unit
{
    int id = 0;
    Team team = Team::Player;
    int kind = 0;
    Vec2 pos;
    float hp = 1.0f;
    float maxHp = 1.0f;
    float damage = 1.0f;
    float range = 24.0f;
    float attackDelay = 1.0f;
    float attackTimer = 0.0f;
    float speed = 40.0f;
    float radius = 16.0f;
    float hitFlash = 0.0f;
    float shakeTimer = 0.0f;
    float shakePhase = 0.0f;
    float attackAnim = 0.0f;
    float attackAnimMax = 0.0f;
    float attackDir = 1.0f;
    float stateTime = 0.0f;
    float walkCycle = 0.0f;
    int reward = 0;
    bool ranged = false;
    bool elite = false;
    bool alive = true;
    UnitAnimState animState = UnitAnimState::Idle;
};

// Ranged attacks are separate from units so they can travel over time, keep a
// short trail, and trigger impact VFX only when they actually reach the target.
struct Projectile
{
    Vec2 pos;
    Vec2 lastPos;
    int targetId = -1;
    int sourceId = -1;
    Team team = Team::Player;
    bool targetBase = false;
    float damage = 1.0f;
    float speed = 380.0f;
    float radius = 4.0f;
    float life = 2.2f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
};

struct Particle
{
    Vec2 pos;
    Vec2 vel;
    float radius = 3.0f;
    float life = 1.0f;
    float maxLife = 1.0f;
    float spin = 0.0f;
    float growth = 0.0f;
    float gravity = 18.0f;
    float drag = 0.82f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
    ParticleKind kind = ParticleKind::Dot;
};

struct RingEffect
{
    Vec2 pos;
    float radius = 8.0f;
    float maxRadius = 48.0f;
    float life = 0.35f;
    float maxLife = 0.35f;
    float width = 2.0f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
};

// Short-lived VFX primitives. They are updated with delta time like gameplay
// objects, then drawn as layered Direct2D strokes for a shader-like bloom feel.
struct BeamEffect
{
    Vec2 start;
    Vec2 end;
    float life = 0.2f;
    float maxLife = 0.2f;
    float width = 4.0f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
};

struct SparkLine
{
    Vec2 start;
    Vec2 end;
    float life = 0.22f;
    float maxLife = 0.22f;
    float width = 1.8f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
};

struct FloatText
{
    Vec2 pos;
    std::wstring text;
    float life = 1.0f;
    float maxLife = 1.0f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
};

struct UiPulse
{
    Vec2 pos;
    float radius = 10.0f;
    float maxRadius = 46.0f;
    float life = 0.32f;
    float maxLife = 0.32f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
};


class PawlineGameImpl
{
public:
    // Creates the window, initializes Direct2D/DWrite, and runs the message loop.
    PawlineGameImpl() = default;

    ~PawlineGameImpl();

    HRESULT Initialize();

    int Run();

private:
    // Core engine, combat, input/layout, and rendering methods are implemented
    // across PawlineGameImplCore/Combat/Input/Layout/Render.cpp.
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT CreateTextFormats();

    HRESULT CreateDeviceResources();

    void DiscardDeviceResources();

    const StageDefinition CurrentStage() const;

    int UnitIndex(PlayerUnit unit) const;

    bool IsUnitUnlocked(PlayerUnit unit) const;

    int UnitLevel(PlayerUnit unit) const;

    UnitStats PlayerStats(PlayerUnit unit) const;

    int UnitUnlockCost(PlayerUnit unit) const;

    int UnitUpgradeCost(PlayerUnit unit) const;

    int StageClearReward(bool firstClear) const;

    void GrantStageReward();

    std::wstring ProgressPath() const;

    void LoadProgress();

    void SaveProgress() const;

    void ResetToTitle();

    void ResetToMenu();

    void ResetGame();

    void Update(float dt);

    void UpdateCamera(float dt);

    void UpdateViewMetrics();

    void SetViewTransform(float worldCameraX = 0.0f, bool includeCameraShake = false);

    Vec2 ClientToVirtual(Vec2 pos) const;

    void UpdateEnemyDirector(float dt);

    EnemyUnit PickStageEnemy(int value, int phase) const;

    EnemyUnit StageBossType() const;

    std::wstring StageEnemySummary() const;

    float GimmickInterval() const;

    float EffectiveUnitRange(const Unit& unit) const;

    float StageMoveSpeedModifier(const Unit& unit) const;

    void UpdateStageGimmicks(float dt);

    void TriggerStageGimmick();

    void ApplyAreaDamage(Vec2 center, float radius, float damage, D2D1_COLOR_F color);

    void SpawnStageReinforcement(EnemyUnit type, float forwardOffset, bool elite = false);

    float ThreatLevel() const;

    float MaxEnergy() const;

    float EnergyRegen() const;

    int WalletUpgradeCost() const;

    float WalletUnitBoost() const;

    int UnitEnergyCost(PlayerUnit unit) const;

    float UnitCooldown(PlayerUnit unit) const;

    float WalletPulseInterval() const;

    void UpdateWalletPulse(float dt);

    void TriggerWalletPulse(bool upgradeBurst);

    void SpawnPlayer(PlayerUnit type);

    void SpawnEnemy(EnemyUnit type, bool elite = false);

    float RandomLaneY();

    void UpdateUnits(float dt);

    void SetUnitAnimState(Unit& unit, UnitAnimState state);

    UnitAnimState ResolveAttackAnimState(const Unit& unit) const;

    int FindTargetIndex(const Unit& unit) const;

    bool IsBlocked(const Unit& unit) const;

    bool IsEnemyBaseInRange(const Unit& unit) const;

    void AttackUnit(Unit& attacker, Unit& target);

    void AttackBase(Unit& attacker);

    void FireProjectile(Unit& attacker, const Unit& target);

    void FireProjectileAtBase(Unit& attacker);

    void BeginAttack(Unit& attacker, Vec2 targetPos);

    void AddAttackVfx(const Unit& attacker, Vec2 targetPos, D2D1_COLOR_F color);

    void UpdateProjectiles(float dt);

    Unit* FindUnitById(int id);

    void DamageUnit(Unit& target, float damage, Team sourceTeam);

    void DamageBase(Team baseTeam, float damage, Vec2 source);

    void ShakeUnit(Unit& unit, float duration);

    void ShakeUnitById(int id, float duration);

    void AddCameraTrauma(float amount);

    void UpdateParticles(float dt);

    void CleanupEntities();

    void TrySpawnPlayer(int index);

    void TryUpgradeWallet();

    void TryFireCannon();

    void DecreaseGameSpeed();

    void IncreaseGameSpeed();

    void AddParticle(Vec2 pos, Vec2 vel, float radius, float life, D2D1_COLOR_F color);

    void AddParticleEx(Vec2 pos, Vec2 vel, float radius, float life, D2D1_COLOR_F color, ParticleKind kind, float gravity, float drag, float growth);

    void AddRing(Vec2 pos, float maxRadius, float life, D2D1_COLOR_F color, float width);

    void AddBeam(Vec2 start, Vec2 end, float width, float life, D2D1_COLOR_F color);

    void AddSparkLines(Vec2 pos, D2D1_COLOR_F color, int count);

    void AddBurst(Vec2 pos, D2D1_COLOR_F color, int count);

    void AddDustPuff(Vec2 pos, D2D1_COLOR_F color, int count);

    void AddDeathBurst(const Unit& unit);

    void AddHitEffects(Vec2 pos, D2D1_COLOR_F color);

    void AddFloatText(Vec2 pos, const std::wstring& text, D2D1_COLOR_F color, float life);

    void AddUiPulse(Vec2 pos, D2D1_COLOR_F color);

    void SetMessage(const std::wstring& message);

    void OpenEscapeMenu();

    void CloseEscapeMenu();

    void AdjustEscapeMenuSpeed(float delta);

    void OnLeftClick(Vec2 pos);

    void OnMenuClick(Vec2 pos);

    void OnShopClick(Vec2 pos);

    void OnBriefingClick(Vec2 pos);

    void OnOptionsClick(Vec2 pos);

    void OnEscapeMenuClick(Vec2 pos);

    void OnResultClick(Vec2 pos);

    void TryBuyOrUpgradeUnit(PlayerUnit unit);

    void SetLoadoutUnit(PlayerUnit unit);

    void OnKeyDown(WPARAM key);

    D2D1_RECT_F CardRect(int index) const;

    D2D1_RECT_F TitleStartButtonRect() const;

    D2D1_RECT_F TitleOptionsButtonRect() const;

    D2D1_RECT_F TitleQuitButtonRect() const;

    D2D1_RECT_F OptionsShakeButtonRect() const;

    D2D1_RECT_F OptionsSpeedDownButtonRect() const;

    D2D1_RECT_F OptionsSpeedUpButtonRect() const;

    D2D1_RECT_F OptionsViewDownButtonRect() const;

    D2D1_RECT_F OptionsViewUpButtonRect() const;

    D2D1_RECT_F OptionsViewResetButtonRect() const;

    D2D1_RECT_F OptionsBackButtonRect() const;

    D2D1_RECT_F MenuStageRect(int index) const;

    D2D1_RECT_F MenuLoadoutSlotRect(int index) const;

    D2D1_RECT_F RosterCardRect(int index) const;

    D2D1_RECT_F StartGameButtonRect() const;

    D2D1_RECT_F MenuShopButtonRect() const;

    D2D1_RECT_F BriefingStartButtonRect() const;

    D2D1_RECT_F BriefingBackButtonRect() const;

    D2D1_RECT_F BriefingShopButtonRect() const;

    D2D1_RECT_F ShopBackButtonRect() const;

    D2D1_RECT_F ShopUnitRect(int index) const;

    D2D1_RECT_F SpeedDownButtonRect() const;

    D2D1_RECT_F SpeedUpButtonRect() const;

    D2D1_RECT_F ResultRetryButtonRect() const;

    D2D1_RECT_F ResultNextButtonRect() const;

    D2D1_RECT_F ResultMenuButtonRect() const;

    D2D1_RECT_F WalletButtonRect() const;

    D2D1_RECT_F CannonButtonRect() const;

    D2D1_RECT_F PauseButtonRect() const;

    D2D1_RECT_F RestartButtonRect() const;

    D2D1_RECT_F EscapeResumeButtonRect() const;

    D2D1_RECT_F EscapeShakeButtonRect() const;

    D2D1_RECT_F EscapeSpeedDownButtonRect() const;

    D2D1_RECT_F EscapeSpeedUpButtonRect() const;

    D2D1_RECT_F EscapeStageButtonRect() const;

    D2D1_RECT_F EscapeQuitButtonRect() const;

    void Render();

    void SetColor(D2D1_COLOR_F color);

    void FillRect(D2D1_RECT_F rect, D2D1_COLOR_F color);

    void StrokeRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float width = 1.0f);

    void FillRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color);

    void StrokeRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color, float width = 1.0f);

    void FillEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color);

    void StrokeEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color, float width = 1.0f);

    void DrawLine(Vec2 a, Vec2 b, D2D1_COLOR_F color, float width = 1.0f);

    void DrawString(const std::wstring& text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);

    void DrawOutlinedString(const std::wstring& text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color, float outlineAlpha = 0.82f);

    void DrawCartoonPanel(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F accent, bool hover = false);

    float PixelTextWidth(const std::wstring& text, float cell) const;

    void DrawPixelText(const std::wstring& text, Vec2 pos, float cell, D2D1_COLOR_F color, float alpha = 1.0f, bool shadow = true);

    void DrawPixelTextCentered(const std::wstring& text, D2D1_RECT_F rect, float cell, D2D1_COLOR_F color, float alpha = 1.0f);

    Vec2 WorldToScreen(Vec2 pos) const;

    D2D1_RECT_F WorldRect(float left, float top, float right, float bottom) const;

    void DrawPlayerIcon(PlayerUnit type, Vec2 center, float scale, bool enabled);

    bool IsUnitInLoadout(PlayerUnit unit) const;

    void DrawTitle();

    void DrawOptions();

    void DrawMenu();

    void DrawBriefing();

    void DrawShop();

    void DrawShopUnitCard(int index);

    void DrawStageCard(int index);

    void DrawLoadoutSlot(int index);

    void DrawRosterCard(int index);

    void DrawArena();

    void DrawSpaceBackdrop();

    void DrawCrater(Vec2 center, float rx, float ry, D2D1_COLOR_F rim, D2D1_COLOR_F shade);

    void DrawCloudCluster(Vec2 center, float scale, D2D1_COLOR_F color);

    void DrawStageDecorations();

    void DrawLongRangeDecorations();

    void DrawStageLanePattern();

    void DrawBases();

    void DrawBaseHp(Vec2 base, float hp, float maxHp, D2D1_COLOR_F color);

    void DrawUnitLighting();

    void DrawUnits();

    Vec2 UnitRenderPos(const Unit& unit) const;

    float AttackProgress(const Unit& unit) const;

    float AttackIntensity(const Unit& unit) const;

    float AttackWindup(const Unit& unit) const;

    float AttackStrike(const Unit& unit) const;

    float AttackRecoil(const Unit& unit) const;

    float AttackLungeDistance(const Unit& unit) const;

    void DrawUnitActionLines(const Unit& unit, Vec2 pos, D2D1_COLOR_F accent);

    void DrawPlayerWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil);

    void DrawEnemyWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil);

    void DrawPlayerUnit(const Unit& unit);

    void DrawEnemyUnit(const Unit& unit);

    void DrawUnitHp(const Unit& unit);

    void DrawProjectiles();

    void DrawBeams();

    void DrawSparkLines();

    void DrawRings();

    void DrawParticles();

    void DrawShaderPostProcess();

    void DrawScreenFlash();

    void DrawFloatTexts();

    void DrawUiPulses();

    void DrawStageGimmickOverlay();

    void DrawBossPresentation();

    void DrawTutorialTips();

    void DrawHeader();

    void DrawBattleLogo();

    void DrawCameraHud();

    void DrawTopStat(float x, const std::wstring& label, const std::wstring& value, D2D1_COLOR_F color);

    void DrawCommandBar();

    void DrawCombatHelpPanel();

    void DrawUnitCard(int index);

    void DrawWalletButton();

    void DrawCannonButton();

    void DrawButton(D2D1_RECT_F rect, const std::wstring& label, bool enabled, D2D1_COLOR_F fill);

    void DrawMessage();

    void DrawOverlay();

    void DrawEscapeMenuClean();

    void DrawResultScreen();

private:
    HWND m_hwnd = nullptr;
    ID2D1Factory* m_factory = nullptr;
    ID2D1HwndRenderTarget* m_renderTarget = nullptr;
    ID2D1SolidColorBrush* m_brush = nullptr;
    ID2D1StrokeStyle* m_roundStroke = nullptr;
    IDWriteFactory* m_writeFactory = nullptr;
    IDWriteTextFormat* m_titleFormat = nullptr;
    IDWriteTextFormat* m_headerFormat = nullptr;
    IDWriteTextFormat* m_bodyFormat = nullptr;
    IDWriteTextFormat* m_smallFormat = nullptr;
    IDWriteTextFormat* m_buttonFormat = nullptr;
    IDWriteTextFormat* m_centerFormat = nullptr;

    DeltaTimer m_timer;
    std::mt19937 m_rng{std::random_device{}()};

    std::vector<Unit> m_units;
    std::vector<Projectile> m_projectiles;
    std::vector<Particle> m_particles;
    std::vector<RingEffect> m_rings;
    std::vector<BeamEffect> m_beams;
    std::vector<SparkLine> m_sparkLines;
    std::vector<FloatText> m_floatTexts;
    std::vector<UiPulse> m_uiPulses;
    std::array<float, kLoadoutSize> m_cardCooldowns = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::array<PlayerUnit, kLoadoutSize> m_loadout = {PlayerUnit::Paw, PlayerUnit::Box, PlayerUnit::Spark, PlayerUnit::Dash, PlayerUnit::Bell};
    std::array<bool, kRosterCount> m_unitUnlocked = {
        true, true, true, true, true,
        false, false, false, false, false,
        false, false, false, false};
    std::array<int, kRosterCount> m_unitLevels = {
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1};
    std::array<bool, kStageCount> m_stageCleared = {
        false, false, false, false, false,
        false, false, false, false, false};

    Vec2 m_mouse = {};
    GameScreen m_screen = GameScreen::Title;
    int m_selectedStage = 0;
    int m_selectedLoadoutSlot = 0;
    int m_nextUnitId = 1;
    int m_walletLevel = 1;
    int m_score = 0;
    int m_resultScore = 0;
    int m_lumen = 0;
    int m_lastReward = 0;
    float m_energy = 0.0f;
    float m_playerBaseHp = 1.0f;
    float m_enemyBaseHp = 1.0f;
    float m_playerBaseMaxHp = 1.0f;
    float m_enemyBaseMaxHp = 1.0f;
    float m_stageTime = 0.0f;
    float m_gameSpeed = 1.0f;
    float m_defaultGameSpeed = 1.0f;
    float m_enemyTimer = 0.0f;
    float m_nextBossTime = 38.0f;
    float m_cannonCharge = 0.0f;
    float m_cannonFlash = 0.0f;
    float m_screenFlash = 0.0f;
    float m_uiTime = 0.0f;
    float m_walletPulseTimer = 0.0f;
    float m_stageGimmickTimer = 0.0f;
    float m_stageGimmickPulse = 0.0f;
    float m_stageAmbientTimer = 0.0f;
    float m_bossBannerTimer = 0.0f;
    float m_bossWarningTimer = 0.0f;
    float m_bossFocusX = 0.0f;
    float m_cameraTrauma = 0.0f;
    float m_cameraX = 0.0f;
    float m_cameraTargetX = 0.0f;
    float m_viewScale = 1.0f;
    float m_viewOffsetX = 0.0f;
    float m_viewOffsetY = 0.0f;
    float m_userViewScale = 0.96f;
    float m_playerBaseShake = 0.0f;
    float m_enemyBaseShake = 0.0f;
    float m_resultTime = 0.0f;
    bool m_paused = false;
    bool m_gameOver = false;
    bool m_victory = false;
    bool m_hitShakeEnabled = true;
    bool m_escapeMenuOpen = false;
    bool m_pauseBeforeEscape = false;
    std::wstring m_message;
    float m_messageTimer = 0.0f;
};
