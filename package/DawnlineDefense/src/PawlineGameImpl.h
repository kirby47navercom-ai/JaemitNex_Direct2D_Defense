#pragma once

#include <windows.h>
#include "framework/AudioManager.h"
#include "framework/DeltaTimer.h"
#include "GameData.h"

#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <fstream>
#include <iomanip>
#include <optional>
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
constexpr int kSaveSlotCount = 3;
// 같은 종류의 효과음이 짧은 시간에 몰려도 일부는 살아남도록 둔 가상 음성 슬롯 수다.
// 실제 동시 재생은 FMOD 빌드에서 가장 안정적이고, 기본 WinMM 빌드는 OS 믹서 한계가 있다.
constexpr size_t kSfxThrottleVoices = 10;
constexpr size_t kAttackSfxThrottleVoices = 8;

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
    StoryIntro,
    Options,
    Menu,
    Archive,
    Shop,
    Briefing,
    Playing,
    Result,
    Ending
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

enum class SfxKind
{
    Spawn,
    EnemySpawn,
    Hit,
    HeavyHit,
    Shoot,
    Upgrade,
    Clear,
    UnitAttack,
    ProjectileImpact,
    BaseHit,
    Death,
    Boss,
    Stage,
    Wallet,
    Ui,
    Count
};

constexpr size_t kSfxChannelCount = static_cast<size_t>(SfxKind::Count);

enum class UiSliderDragTarget
{
    None,
    OptionsSfx,
    OptionsUi,
    OptionsBgm,
    OptionsSpeed,
    OptionsView,
    EscapeSfx,
    EscapeUi,
    EscapeBgm
};

enum class ImageVfxKind
{
    Slash,
    EnemySlash,
    Heal,
    HealSoft,
    Fire,
    Ice,
    Thunder,
    Water,
    Dark,
    Acid,
    Earth,
    Smoke,
    Holy,
    Wind,
    WindHit,
    Wood,
    HitFlash,
    Smear,
    Thrust,
    Explosion,
    FireBreath,
    MagicMirror,
    EnergyImpact,
    Crystal,
    AirBurst,
    ThunderSplash,
    WaterBallImpact,
    SmokeDust
};

enum class Difficulty
{
    Easy,
    Normal,
    Hard
};

// 행성 기믹과 보스 패턴을 바로 터뜨리지 않고, 먼저 위험 범위를 보여주기 위한 종류 값이다.
enum class TelegraphKind
{
    MercuryHeat,
    VenusFog,
    EarthBloom,
    MarsMeteor,
    JupiterGravity,
    SaturnReinforce,
    UranusIce,
    NeptuneTide,
    PlutoVoid,
    SolarFlare,
    BossFlareLine,
    BossPulseCircle,
    BossReinforce
};

enum class TelegraphShape
{
    Circle,
    Line,
    FullLane
};


// 전장 위에서 움직이는 하나의 유닛.
// 아군/적이 같은 구조체를 쓰기 때문에 이동, 타겟 탐색, 공격, 피격 연출을 같은 코드로 처리한다.
struct Unit
{
    // 식별/소속/위치.
    int id = 0;
    Team team = Team::Player;
    int kind = 0;
    Vec2 pos;

    // 전투 능력치.
    float hp = 1.0f;
    float maxHp = 1.0f;
    float damage = 1.0f;
    float range = 24.0f;
    float attackDelay = 1.0f;
    float attackTimer = 0.0f;
    float speed = 40.0f;
    float radius = 16.0f;

    // 렌더링과 애니메이션 피드백.
    float hitFlash = 0.0f;
    float shakeTimer = 0.0f;
    float shakePhase = 0.0f;
    float attackAnim = 0.0f;
    float attackAnimMax = 0.0f;
    float attackDir = 1.0f;
    float stateTime = 0.0f;
    float walkCycle = 0.0f;

    // 스턴/넉백 상태. 체력 문턱을 넘으면 이 값들이 켜져 잠깐 뒤로 밀린다.
    float stunTimer = 0.0f;
    float knockbackTimer = 0.0f;
    float knockbackVelocity = 0.0f;
    float nextKnockbackPct = 0.50f;

    // 보상과 역할 플래그.
    int reward = 0;
    bool ranged = false;
    bool elite = false;
    bool boss = false;
    bool alive = true;
    UnitAnimState animState = UnitAnimState::Idle;
};

enum class ProjectileVisual
{
    Bolt,
    BellWave,
    OrbitStar,
    PrismShard,
    NebulaOrb,
    MintPulse,
    FrostShard,
    AcidGlob,
    TideWave,
    MirrorShard,
    VoidOrb,
    SolarSpark,
    SporeSeed
};

// 원거리 공격은 유닛과 분리해서 실제로 날아가고, 유닛 종류별로
// 다른 탄 모양과 착탄 이펙트를 낼 수 있게 관리한다.
struct Projectile
{
    Vec2 pos;
    Vec2 lastPos;
    int targetId = -1;
    int sourceId = -1;
    // 발사자가 사라져도 착탄 상성을 계산할 수 있게 발사 당시 종류를 보관한다.
    int sourceKind = 0;
    Team team = Team::Player;
    bool targetBase = false;
    float damage = 1.0f;
    float speed = 380.0f;
    float radius = 4.0f;
    float life = 2.2f;
    float age = 0.0f;
    float spin = 0.0f;
    float wobble = 0.0f;
    ProjectileVisual visual = ProjectileVisual::Bolt;
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

// 전투에서 잠깐 생겼다가 사라지는 빛 줄기.
// UpdateParticles에서 델타타임으로 수명을 줄이고, Render에서는 블룸 레이어처럼 겹쳐 그린다.
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
    // 이름은 SparkLine이지만 화면에는 선보다 작은 빛 파편으로 보이도록 렌더링한다.
    Vec2 start;
    Vec2 end;
    float life = 0.22f;
    float maxLife = 0.22f;
    float width = 1.8f;
    D2D1_COLOR_F color = D2D1::ColorF(0xFFFFFF);
};

// 외부 PNG 이펙트 시트를 짧게 재생하는 객체.
// 선으로 긋는 임시 연출보다 완성된 빛 이미지가 전투 순간을 맡도록 한다.
struct ImageVfx
{
    ImageVfxKind kind = ImageVfxKind::Slash;
    Vec2 pos;
    float size = 80.0f;
    float life = 0.25f;
    float maxLife = 0.25f;
    float dir = 1.0f;
    float frameOffset = 0.0f;
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

// 위험 예고 하나의 상태. life가 0이 되면 실제 피해, 회복, 지원군 효과가 실행된다.
struct Telegraph
{
    TelegraphKind kind = TelegraphKind::MarsMeteor;
    TelegraphShape shape = TelegraphShape::Circle;
    Vec2 start;
    Vec2 end;
    float radius = 80.0f;
    float width = 70.0f;
    float life = 1.0f;
    float maxLife = 1.0f;
    float damage = 0.0f;
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

    void RegisterPrivateFonts();

    void UnregisterPrivateFonts();

    std::wstring ExecutableDir() const;

    std::wstring AssetPath(const std::wstring& relativePath) const;

    bool StartMusicNow(const std::wstring& absolutePath, bool loop, float fadeLevel);

    void PlayMusicTrack(const std::wstring& relativePath, float fadeSeconds = 0.55f, bool loop = true);

    void StartBackgroundMusic();

    std::wstring StageMusicPath(int stageIndex) const;

    void StartStageMusic();

    std::wstring ResultMusicPath(bool victory) const;

    void StartResultMusic(bool victory);

    void PlayMusicStinger(const std::wstring& relativePath, float volumeScale = 1.0f);

    void StartDangerMusicLayer();

    void StopDangerMusicLayer();

    float MusicDangerTarget() const;

    void UpdateMusicSystem(float dt);

    void SyncMusicVolume();

    float VolumeForSfxKind(SfxKind kind) const;

    void PlaySfx(SfxKind kind, float minGapSeconds = 0.05f);

    void PlaySfxFile(const std::wstring& relativeFileName, SfxKind throttleKind, float minGapSeconds = 0.05f);

    void PlaySfxAt(SfxKind kind, float worldX, float minGapSeconds = 0.05f, float volumeScale = 1.0f);

    void PlaySfxFileAt(const std::wstring& relativeFileName, SfxKind throttleKind, float worldX, float minGapSeconds = 0.05f, float volumeScale = 1.0f);

    std::wstring AttackSfxPath(const Unit& attacker) const;

    float AttackSfxVolumeScale(const Unit& attacker) const;

    void PlayAttackSfxAt(const Unit& attacker, float minGapSeconds = 0.018f);

    void AdjustSfxVolume(float delta);

    void AdjustUiVolume(float delta);

    void AdjustBgmVolume(float delta);

    void ResetAudioVolumes();

    HRESULT LoadBitmapFromFile(const std::wstring& path, ID2D1Bitmap** bitmap) const;

    void LoadBitmapAssets();

    void DiscardBitmapAssets();

    const StageDefinition CurrentStage() const;

    int UnitIndex(PlayerUnit unit) const;

    bool IsUnitUnlocked(PlayerUnit unit) const;

    int UnitLevel(PlayerUnit unit) const;

    bool IsUnitEvolved(PlayerUnit unit) const;

    UnitStats PlayerStats(PlayerUnit unit) const;

    std::wstring UnitDisplayName(PlayerUnit unit) const;

    int UnitUnlockCost(PlayerUnit unit) const;

    int UnitUpgradeCost(PlayerUnit unit) const;

    int StageClearReward(bool firstClear) const;

    float DifficultyThreatMultiplier() const;

    float DifficultyRewardMultiplier() const;

    std::wstring DifficultyLabel() const;

    bool IsStageUnlocked(int index) const;

    int HighestUnlockedStage() const;

    bool HasAnyProgressFile() const;

    void BeginStoryCrawl(bool autoContinueToMenu, GameScreen returnScreen = GameScreen::Title, bool returnToEscapeMenu = false);

    void FinishStoryCrawl();

    void BeginEndingScene();

    void FinishEndingScene();

    void SelectStage(int index);

    void GrantStageReward();

    std::wstring ProgressPath() const;

    std::wstring ProgressPath(int slot) const;

    std::wstring LegacyProgressPath() const;

    std::wstring SaveSlotLabel() const;

    void LoadProgress();

    void LoadProgress(int slot);

    void SaveProgress();

    void SaveProgressToSlot(int slot) const;

    void SelectSaveSlot(int slot);

    void ResetProgressMemory();

    void DeleteSelectedSaveSlot();

    void ResetProgressData();

    void ResetToTitle();

    void ResetToMenu();

    void ResetGame();

    void StartDemoRun();

    void Update(float dt);

    void UpdateDemoMode(float dt);

    void TriggerDebugClear();

    void TriggerDebugUnlockAll();

    void UpdateCamera(float dt);

    void UpdateViewMetrics();

    void SetViewTransform(float worldCameraX = 0.0f, bool includeCameraShake = false);

    Vec2 ClientToVirtual(Vec2 pos) const;

    void ResetCombatFeedbackState();

    void UpdateCombatFeedbackTimers(float dt);

    float CombatTimeScale() const;

    float PostFxFeedbackIntensity() const;

    void UpdateEnemyDirector(float dt);

    void UpdateDirectorPressure(float dt);

    float DirectorSpawnMultiplier() const;

    float BossTriggerHpRatio() const;

    EnemyUnit PickStageEnemy(int value, int phase) const;

    EnemyUnit StageBossType() const;

    void PromoteBossUnit(Unit& boss);

    float BossDamageTakenScale(const Unit& boss) const;

    std::wstring StageEnemySummary() const;

    std::wstring CounterPlanSummary() const;

    float StageThreatRating() const;

    float LoadoutPowerRating() const;

    std::wstring BalanceAdvice() const;

    float GimmickInterval() const;

    std::optional<std::reference_wrapper<Unit>> FindBossUnit();

    std::optional<std::reference_wrapper<const Unit>> FindBossUnit() const;

    void UpdateBossPatterns(float dt);

    void TriggerBossPattern(Unit& boss);

    float EffectiveUnitRange(const Unit& unit) const;

    float StageMoveSpeedModifier(const Unit& unit) const;

    void UpdateStageGimmicks(float dt);

    void TriggerStageGimmick();

    void AddTelegraph(TelegraphKind kind, TelegraphShape shape, Vec2 start, Vec2 end, float radius, float width, float windup, float damage, D2D1_COLOR_F color);

    void UpdateTelegraphs(float dt);

    void ExecuteTelegraph(const Telegraph& telegraph);

    void ApplyAreaDamage(Vec2 center, float radius, float damage, D2D1_COLOR_F color);

    void ApplyLineDamage(Vec2 start, Vec2 end, float width, float damage, D2D1_COLOR_F color);

    void SpawnStageReinforcement(EnemyUnit type, float forwardOffset, bool elite = false);

    float ThreatLevel() const;

    float MaxEnergy() const;

    float EnergyRegen() const;

    int WalletUpgradeCost() const;

    float WalletUnitBoost() const;

    int UnitEnergyCost(PlayerUnit unit) const;

    float UnitCooldown(PlayerUnit unit) const;

    float WalletPulseInterval() const;

    bool HasLoadoutUnit(PlayerUnit unit) const;

    float SynergyHpMultiplier(PlayerUnit unit) const;

    float SynergyDamageMultiplier(PlayerUnit unit) const;

    float SynergyRangeMultiplier(PlayerUnit unit) const;

    float SynergySpeedMultiplier(PlayerUnit unit) const;

    std::wstring SynergySummary() const;

    std::wstring GrowthRecommendation() const;

    void UpdateWalletPulse(float dt);

    void TriggerWalletPulse(bool upgradeBurst);

    void SpawnPlayer(PlayerUnit type);

    void SpawnEnemy(EnemyUnit type, bool elite = false);

    float RandomLaneY();

    void UpdateUnits(float dt);

    void SetUnitAnimState(Unit& unit, UnitAnimState state);

    UnitAnimState ResolveAttackAnimState(const Unit& unit) const;

    void TriggerBossEntrance(Unit& boss, D2D1_COLOR_F color);

    void TriggerBossPhaseChange(Unit& boss, int phase);

    float UnitKnockbackStep(const Unit& unit) const;

    void CheckUnitKnockback(Unit& target, Team sourceTeam);

    void TriggerUnitKnockback(Unit& unit, Team sourceTeam, float strength, float stunDuration, bool force);

    void TriggerHitStop(float holdDuration, float slowScale, float slowDuration);

    int FindTargetIndex(const Unit& unit) const;

    bool IsBlocked(const Unit& unit) const;

    bool IsEnemyBaseInRange(const Unit& unit) const;

    void AttackUnit(Unit& attacker, Unit& target);

    void AttackBase(Unit& attacker);

    float AttackMatchupMultiplier(Team attackerTeam, int attackerKind, const Unit& target) const;

    void AddCounterFloatText(const Unit& target, float multiplier);

    void FireProjectile(Unit& attacker, const Unit& target);

    void FireProjectileAtBase(Unit& attacker);

    Vec2 ProjectileMuzzle(const Unit& attacker, Vec2 targetPos) const;

    void ConfigureProjectileVisual(Projectile& projectile, const Unit& attacker);

    void AddProjectileImpact(const Projectile& projectile);

    void AddMeleeClashVfx(const Unit& attacker, Vec2 targetPos, D2D1_COLOR_F color);

    void BeginAttack(Unit& attacker, Vec2 targetPos);

    void AddAttackVfx(const Unit& attacker, Vec2 targetPos, D2D1_COLOR_F color);

    void UpdateProjectiles(float dt);

    std::optional<std::reference_wrapper<Unit>> FindUnitById(int id);

    void DamageUnit(Unit& target, float damage, Team sourceTeam);

    void ApplyImpactReaction(Unit& target, Team sourceTeam, float damage, D2D1_COLOR_F color);

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

    void AddImageVfx(ImageVfxKind kind, Vec2 pos, float size, float life, D2D1_COLOR_F color, float dir = 1.0f);

    void AddBurst(Vec2 pos, D2D1_COLOR_F color, int count);

    void AddDustPuff(Vec2 pos, D2D1_COLOR_F color, int count);

    void AddDeathBurst(const Unit& unit);

    void AddHitEffects(Vec2 pos, D2D1_COLOR_F color);

    void AddFloatText(Vec2 pos, const std::wstring& text, D2D1_COLOR_F color, float life);

    void AddUiPulse(Vec2 pos, D2D1_COLOR_F color, float maxRadius = 52.0f, float life = 0.34f);

    void SetMessage(const std::wstring& message);

    void OpenEscapeMenu();

    void CloseEscapeMenu();

    void AdjustEscapeMenuSpeed(float delta);

    void OnLeftClick(Vec2 pos);

    UiSliderDragTarget SliderDragTargetAt(Vec2 pos) const;

    void BeginSliderDrag(Vec2 pos);

    void UpdateSliderDrag(Vec2 pos, bool playFeedback);

    void EndSliderDrag();

    void ApplySliderDragValue(UiSliderDragTarget target, Vec2 pos, bool playFeedback);

    bool IsInteractivePoint(Vec2 pos) const;

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

    D2D1_RECT_F TitleDemoButtonRect() const;

    D2D1_RECT_F TitleStoryButtonRect() const;

    D2D1_RECT_F TitleOptionsButtonRect() const;

    D2D1_RECT_F TitleQuitButtonRect() const;

    D2D1_RECT_F StorySkipButtonRect() const;

    D2D1_RECT_F OptionsShakeButtonRect() const;

    D2D1_RECT_F OptionsFlashButtonRect() const;

    D2D1_RECT_F OptionsSfxDownButtonRect() const;

    D2D1_RECT_F OptionsSfxUpButtonRect() const;

    D2D1_RECT_F OptionsSfxSliderRect() const;

    D2D1_RECT_F OptionsUiSliderRect() const;

    D2D1_RECT_F OptionsBgmDownButtonRect() const;

    D2D1_RECT_F OptionsBgmUpButtonRect() const;

    D2D1_RECT_F OptionsBgmSliderRect() const;

    D2D1_RECT_F OptionsAudioResetButtonRect() const;

    D2D1_RECT_F OptionsSpeedDownButtonRect() const;

    D2D1_RECT_F OptionsSpeedUpButtonRect() const;

    D2D1_RECT_F OptionsSpeedSliderRect() const;

    D2D1_RECT_F OptionsViewDownButtonRect() const;

    D2D1_RECT_F OptionsViewUpButtonRect() const;

    D2D1_RECT_F OptionsViewSliderRect() const;

    D2D1_RECT_F OptionsViewResetButtonRect() const;

    D2D1_RECT_F OptionsSaveSlotButtonRect(int index) const;

    D2D1_RECT_F OptionsSaveProgressButtonRect() const;

    D2D1_RECT_F OptionsLoadProgressButtonRect() const;

    D2D1_RECT_F OptionsDeleteProgressButtonRect() const;

    D2D1_RECT_F OptionsResetProgressButtonRect() const;

    D2D1_RECT_F OptionsBackButtonRect() const;

    D2D1_RECT_F MenuStageRect(int index) const;

    D2D1_RECT_F MenuLoadoutSlotRect(int index) const;

    D2D1_RECT_F RosterCardRect(int index) const;

    D2D1_RECT_F StartGameButtonRect() const;

    D2D1_RECT_F MenuShopButtonRect() const;

    D2D1_RECT_F MenuArchiveButtonRect() const;

    D2D1_RECT_F ArchiveBackButtonRect() const;

    D2D1_RECT_F ArchiveTabRect(int index) const;

    D2D1_RECT_F BriefingStartButtonRect() const;

    D2D1_RECT_F BriefingBackButtonRect() const;

    D2D1_RECT_F BriefingShopButtonRect() const;

    D2D1_RECT_F BriefingDifficultyRect(int index) const;

    D2D1_RECT_F BriefingLoadoutSlotRect(int index) const;

    D2D1_RECT_F ShopBackButtonRect() const;

    D2D1_RECT_F ShopUnitRect(int index) const;

    D2D1_RECT_F SpeedDownButtonRect() const;

    D2D1_RECT_F SpeedUpButtonRect() const;

    D2D1_RECT_F ResultRetryButtonRect() const;

    D2D1_RECT_F ResultNextButtonRect() const;

    D2D1_RECT_F ResultMenuButtonRect() const;

    D2D1_RECT_F WalletButtonRect() const;

    D2D1_RECT_F CannonButtonRect() const;

    D2D1_RECT_F MessageToastRect() const;

    D2D1_RECT_F PauseButtonRect() const;

    D2D1_RECT_F RestartButtonRect() const;

    D2D1_RECT_F EscapeResumeButtonRect() const;

    D2D1_RECT_F EscapeShakeButtonRect() const;

    D2D1_RECT_F EscapeSpeedDownButtonRect() const;

    D2D1_RECT_F EscapeSpeedUpButtonRect() const;

    D2D1_RECT_F EscapeSaveButtonRect() const;

    D2D1_RECT_F EscapeLoadButtonRect() const;

    D2D1_RECT_F EscapeStoryButtonRect() const;

    D2D1_RECT_F EscapeSfxSliderRect() const;

    D2D1_RECT_F EscapeUiSliderRect() const;

    D2D1_RECT_F EscapeBgmSliderRect() const;

    D2D1_RECT_F EscapeAudioResetButtonRect() const;

    D2D1_RECT_F EscapeStageButtonRect() const;

    D2D1_RECT_F EscapeQuitButtonRect() const;

    void Render();

    void SetColor(D2D1_COLOR_F color);

    void FillRect(D2D1_RECT_F rect, D2D1_COLOR_F color);

    void StrokeRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float width = 1.0f);

    D2D1_RECT_F FullViewportRect() const;

    void FillViewport(D2D1_COLOR_F color);

    void FillRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color);

    void StrokeRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color, float width = 1.0f);

    void FillEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color);

    void StrokeEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color, float width = 1.0f);

    void DrawLine(Vec2 a, Vec2 b, D2D1_COLOR_F color, float width = 1.0f);

    void DrawBitmap(ID2D1Bitmap* bitmap, D2D1_RECT_F destination, float opacity = 1.0f, const D2D1_RECT_F* source = nullptr);

    void DrawWeaponBitmap(ID2D1Bitmap* bitmap, Vec2 center, float width, float height, float angleDegrees, float opacity, bool flipX);

    void DrawString(const std::wstring& text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);

    void DrawString(const std::wstring& text, D2D1_RECT_F rect, const Microsoft::WRL::ComPtr<IDWriteTextFormat>& format, D2D1_COLOR_F color);

    void DrawOutlinedString(const std::wstring& text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color, float outlineAlpha = 0.82f);

    void DrawOutlinedString(const std::wstring& text, D2D1_RECT_F rect, const Microsoft::WRL::ComPtr<IDWriteTextFormat>& format, D2D1_COLOR_F color, float outlineAlpha = 0.82f);

    void DrawCartoonPanel(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F accent, bool hover = false);

    void DrawBriefingPanel(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F accent, bool hover = false);

    void DrawBriefingButton(D2D1_RECT_F rect, const std::wstring& label, bool enabled, D2D1_COLOR_F fill, bool active = false);

    float PixelTextWidth(const std::wstring& text, float cell) const;

    void DrawPixelText(const std::wstring& text, Vec2 pos, float cell, D2D1_COLOR_F color, float alpha = 1.0f, bool shadow = true);

    void DrawPixelTextCentered(const std::wstring& text, D2D1_RECT_F rect, float cell, D2D1_COLOR_F color, float alpha = 1.0f);

    Vec2 WorldToScreen(Vec2 pos) const;

    D2D1_RECT_F WorldRect(float left, float top, float right, float bottom) const;

    void DrawVfxAtlasTile(int tileX, int tileY, Vec2 center, float size, float opacity);

    void DrawUiPanelAsset(D2D1_RECT_F rect, int tileIndex, float opacity);

    void DrawPlayerIcon(PlayerUnit type, Vec2 center, float scale, bool enabled);

    bool IsUnitInLoadout(PlayerUnit unit) const;

    void DrawBitmapCover(ID2D1Bitmap* bitmap, D2D1_RECT_F area, float opacity, float time, float cameraX, float motionScale);

    void DrawDeepSpaceBackdrop(D2D1_RECT_F area, int stageIndex, float time, float cameraX, bool showRoute);

    void DrawSpaceDepthGrid(D2D1_RECT_F area, int stageIndex, float time, float cameraX);

    void DrawTitle();

    void DrawOptions();

    void DrawMenu();

    void DrawArchive();

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

    Vec2 StageLightDirection() const;

    float UnitShadowLift(const Unit& unit) const;

    void DrawUnits();

    Vec2 UnitRenderPos(const Unit& unit) const;

    float AttackProgress(const Unit& unit) const;

    float AttackIntensity(const Unit& unit) const;

    float AttackWindup(const Unit& unit) const;

    float AttackStrike(const Unit& unit) const;

    float AttackRecoil(const Unit& unit) const;

    float AttackLungeDistance(const Unit& unit) const;

    void DrawUnitActionLines(const Unit& unit, Vec2 pos, D2D1_COLOR_F accent);

    void DrawImageVfxFrame(ImageVfxKind kind, int frame, Vec2 center, float size, float opacity);

    void DrawImageVfxSprites();

    bool DrawUnitCharacterSprite(const Unit& unit, Vec2 pos, D2D1_COLOR_F accent);

    void DrawUnitIdentityMark(const Unit& unit, Vec2 pos, D2D1_COLOR_F accent);

    void DrawPlayerWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil);

    void DrawEnemyWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil);

    void DrawPlayerUnit(const Unit& unit);

    void DrawEnemyUnit(const Unit& unit);

    void DrawUnitHp(const Unit& unit);

    void DrawUnitStunEffect(const Unit& unit);

    void DrawProjectiles();

    void DrawProjectileShape(const Projectile& projectile, Vec2 drawPos, Vec2 dir, Vec2 normal, float pulse);

    void DrawBeams();

    void DrawSparkLines();

    void DrawRings();

    void DrawParticles();

    void DrawShaderPostProcess();

    void DrawScreenFlash();

    void DrawFloatTexts();

    void DrawUiPulses();

    void DrawTelegraphs();

    void DrawStageGimmickOverlay();

    void DrawBossPresentation();

    void DrawTutorialTips();

    void DrawSynergyPanel(D2D1_RECT_F rect);

    void DrawBalancePanel(D2D1_RECT_F rect);

    void DrawShopUnitDetail();

    void DrawShowcaseBadge();

    std::wstring DemoStepText() const;

    void DrawDebugBadge();

    void DrawHeader();

    void DrawBattleLogo();

    void DrawCameraHud();

    void DrawTopStat(float x, const std::wstring& label, const std::wstring& value, D2D1_COLOR_F color);

    void DrawCommandBar();

    void DrawFullscreenFrameExtensions();

    void DrawCombatHelpPanel();

    void DrawUnitCard(int index);

    void DrawWalletButton();

    void DrawCannonButton();

    void DrawButton(D2D1_RECT_F rect, const std::wstring& label, bool enabled, D2D1_COLOR_F fill);

    void DrawStoryCrawl();

    void DrawEndingScene();

    void DrawMessage();

    void DrawOverlay();

    void DrawSceneTransition();

    void DrawEscapeMenuClean();

    void DrawResultScreen();

    void DrawFinalClearScene(D2D1_RECT_F panel);

private:
    // Win32 창 핸들은 운영체제가 소유하고, 게임은 값을 보관해서 메시지 처리에 사용한다.
    HWND m_hwnd = nullptr;
    // Direct2D/DirectWrite COM 객체는 ComPtr이 참조 카운트를 자동으로 관리한다.
    Microsoft::WRL::ComPtr<ID2D1Factory> m_factory;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush;
    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> m_roundStroke;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_writeFactory;
    Microsoft::WRL::ComPtr<IWICImagingFactory> m_wicFactory;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_titleFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_headerFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_bodyFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_smallFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_buttonFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_centerFormat;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_deepSpaceBitmap;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_vfxAtlas;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_slashEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_enemySlashEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_healEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_healSoftEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_fireEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_iceEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_thunderEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_waterEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_darkEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_acidEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_earthEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_smokeEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_holyEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_windEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_windHitEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_woodEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_hitFlashEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_smearEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_thrustEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_explosionEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_fireBreathEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_magicMirrorEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_energyImpactEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_crystalEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_airBurstEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_thunderSplashEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_waterBallImpactEffectSheet;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_smokeDustEffectSheet;
    // 아군/적 유닛을 종류별로 다르게 보여주는 통합 스프라이트 아틀라스다.
    // 10열 프레임, 유닛별 7행 모션(idle/walk/run/hurt/death/windup/attack) 구조를 사용한다.
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_playerUnitAtlas;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_enemyUnitAtlas;
    std::array<Microsoft::WRL::ComPtr<ID2D1Bitmap>, kRosterCount> m_playerWeaponBitmaps;
    std::array<Microsoft::WRL::ComPtr<ID2D1Bitmap>, kEnemyCount> m_enemyWeaponBitmaps;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> m_uiAtlas;
    bool m_comInitialized = false;
    bool m_bitmapAssetsLoaded = false;
    bool m_privateFontLoaded = false;
    std::vector<std::wstring> m_privateFontPaths;

    // 프레임 시간과 랜덤 연출 생성기.
    DeltaTimer m_timer;
    std::mt19937 m_rng{std::random_device{}()};
    // 효과음이 한 프레임에 과하게 겹치지 않도록 종류별 마지막 재생 시간을 기록한다.
    std::array<std::array<float, kSfxThrottleVoices>, kSfxChannelCount> m_sfxLastTimes = [] {
        std::array<std::array<float, kSfxThrottleVoices>, kSfxChannelCount> values = {};
        for (auto& lanes : values)
        {
            lanes.fill(-10.0f);
        }
        return values;
    }();
    // 공격 효과음은 유닛 타입별로 따로 막아야 여러 종류가 동시에 싸울 때 소리가 먹히지 않는다.
    std::array<std::array<float, kAttackSfxThrottleVoices>, kRosterCount> m_playerAttackSfxLastTimes = [] {
        std::array<std::array<float, kAttackSfxThrottleVoices>, kRosterCount> values = {};
        for (auto& lanes : values)
        {
            lanes.fill(-10.0f);
        }
        return values;
    }();
    std::array<std::array<float, kAttackSfxThrottleVoices>, kEnemyCount> m_enemyAttackSfxLastTimes = [] {
        std::array<std::array<float, kAttackSfxThrottleVoices>, kEnemyCount> values = {};
        for (auto& lanes : values)
        {
            lanes.fill(-10.0f);
        }
        return values;
    }();
    framework::AudioManager m_audio;
    // 효과음과 배경음악은 옵션에서 따로 조절한다.
    std::wstring m_currentMusicPath;
    std::wstring m_pendingMusicPath;
    std::wstring m_currentMusicLayerPath;
    float m_musicFadeLevel = 1.0f;
    float m_musicFadeStartLevel = 1.0f;
    float m_musicFadeTimer = 0.0f;
    float m_musicFadeDuration = 0.55f;
    float m_musicLayerLevel = 0.0f;
    bool m_musicFadingOut = false;
    bool m_pendingMusicLoop = true;
    float m_sfxVolume = 0.86f;
    float m_uiVolume = 0.82f;
    float m_bgmVolume = 0.32f;
    bool m_soundEnabled = true;

    // 전투 중 살아 움직이는 객체와 짧게 사라지는 VFX 컨테이너.
    std::vector<Unit> m_units;
    std::vector<Projectile> m_projectiles;
    std::vector<Particle> m_particles;
    std::vector<RingEffect> m_rings;
    std::vector<BeamEffect> m_beams;
    std::vector<SparkLine> m_sparkLines;
    std::vector<ImageVfx> m_imageVfx;
    std::vector<FloatText> m_floatTexts;
    std::vector<UiPulse> m_uiPulses;
    std::vector<Telegraph> m_telegraphs;

    // 플레이어 성장/편성 저장 대상. SaveProgress/LoadProgress가 이 값을 파일에 기록한다.
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

    // 현재 화면과 메뉴 선택 상태.
    Vec2 m_mouse = {};
    GameScreen m_screen = GameScreen::Title;
    GameScreen m_observedScreen = GameScreen::Title;
    int m_selectedStage = 0;
    int m_selectedLoadoutSlot = 0;
    int m_shopSelectedUnit = 0;
    int m_archiveTab = 0;
    int m_nextUnitId = 1;
    int m_walletLevel = 1;
    int m_score = 0;
    int m_resultScore = 0;
    int m_lumen = 0;
    int m_lastReward = 0;
    int m_saveSlot = 0;
    UiSliderDragTarget m_activeSliderDrag = UiSliderDragTarget::None;

    // 전투 자원, 기지 체력, 시간 배율 상태.
    float m_energy = 0.0f;
    float m_playerBaseHp = 1.0f;
    float m_enemyBaseHp = 1.0f;
    float m_playerBaseMaxHp = 1.0f;
    float m_enemyBaseMaxHp = 1.0f;
    float m_stageTime = 0.0f;
    float m_gameSpeed = 1.0f;
    float m_defaultGameSpeed = 1.0f;
    float m_enemyTimer = 0.0f;
    float m_directorPressure = 0.0f;
    float m_nextBossTime = 38.0f;

    // 캐논/화면 플래시/히트스톱처럼 전투 손맛을 만드는 연출 타이머.
    float m_cannonCharge = 0.0f;
    float m_cannonFlash = 0.0f;
    float m_screenFlash = 0.0f;
    float m_hitStopTimer = 0.0f;
    float m_hitStopMax = 0.0f;
    float m_slowMoTimer = 0.0f;
    float m_slowMoMax = 0.0f;
    float m_slowMoScale = 1.0f;
    float m_postFxPulse = 0.0f;

    // UI, 스테이지 기믹, 보스 페이즈 타이머.
    float m_uiTime = 0.0f;
    float m_walletPulseTimer = 0.0f;
    float m_stageGimmickTimer = 0.0f;
    float m_stageGimmickPulse = 0.0f;
    float m_stageAmbientTimer = 0.0f;
    float m_bossPatternTimer = 7.0f;
    float m_bossBannerTimer = 0.0f;
    float m_bossWarningTimer = 0.0f;
    float m_bossPhaseBannerTimer = 0.0f;
    float m_bossPhaseBannerMax = 0.0f;
    float m_bossFocusX = 0.0f;

    // 카메라와 화면 안전 여백. 사용자가 화면 잘림을 보정할 때 m_userViewScale을 조절한다.
    float m_cameraTrauma = 0.0f;
    float m_cameraX = 0.0f;
    float m_cameraTargetX = 0.0f;
    float m_viewScale = 1.0f;
    float m_viewOffsetX = 0.0f;
    float m_viewOffsetY = 0.0f;
    float m_userViewScale = 1.0f;
    float m_playerBaseShake = 0.0f;
    float m_enemyBaseShake = 0.0f;
    float m_resultTime = 0.0f;
    float m_showcaseTimer = 0.0f;
    float m_demoSpawnTimer = 0.0f;
    float m_demoWalletTimer = 0.0f;
    float m_resetConfirmTimer = 0.0f;
    float m_deleteConfirmTimer = 0.0f;
    float m_autoSaveNoticeTimer = 0.0f;
    float m_storyTimer = 0.0f;
    float m_endingTimer = 0.0f;
    // 화면이 바뀌면 자동으로 켜지는 짧은 페이드 연출 타이머다.
    float m_sceneTransitionTimer = 0.0f;
    float m_sceneTransitionMax = 0.46f;

    // 게임 전체 플래그. 일시정지, 결과, 접근성, 디버그 상태를 구분한다.
    Difficulty m_difficulty = Difficulty::Normal;
    bool m_paused = false;
    bool m_gameOver = false;
    bool m_victory = false;
    bool m_hitShakeEnabled = true;
    bool m_reduceFlashes = false;
    bool m_escapeMenuOpen = false;
    bool m_pauseBeforeEscape = false;
    bool m_bossSpawned = false;
    bool m_bossPhaseTwoTriggered = false;
    bool m_bossPhaseThreeTriggered = false;
    bool m_showcaseMode = false;
    bool m_debugMode = false;
    bool m_storyAutoContinueToMenu = false;
    bool m_storyReturnToEscapeMenu = false;
    bool m_introViewedThisSession = false;
    bool m_endingUnlocked = false;
    GameScreen m_storyReturnScreen = GameScreen::Title;
    int m_bossPhaseBannerLevel = 0;
    std::wstring m_message;
    std::wstring m_autoSaveNotice;
    float m_messageTimer = 0.0f;
};
