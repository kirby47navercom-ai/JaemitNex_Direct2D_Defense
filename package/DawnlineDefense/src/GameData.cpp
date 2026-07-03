#include "GameData.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <cwchar>
#include <fstream>
#include <locale>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct EnemyBalance
{
    UnitStats stats;
    float rewardThreat = 0.0f;
    float hpThreat = 0.0f;
    float damageThreat = 0.0f;
    float speedThreat = 0.0f;
};

struct BalanceTables
{
    std::array<StageDefinition, kStageCount> stages;
    std::array<UnitStats, kRosterCount> players;
    std::array<EnemyBalance, kEnemyCount> enemies;
};

int ClampIndex(int index, int count)
{
    return std::max(0, std::min(count - 1, index));
}

D2D1_COLOR_F ColorFromHex(unsigned int rgb)
{
    return D2D1::ColorF(rgb);
}

UnitStats MakePlayer(std::wstring name, int cost, float cooldown, float hp, float damage, float range,
                     float attackDelay, float speed, float radius, bool ranged, unsigned int color, unsigned int accent)
{
    UnitStats stats;
    stats.name = std::move(name);
    stats.cost = cost;
    stats.cooldown = cooldown;
    stats.hp = hp;
    stats.damage = damage;
    stats.range = range;
    stats.attackDelay = attackDelay;
    stats.speed = speed;
    stats.radius = radius;
    stats.ranged = ranged;
    stats.color = ColorFromHex(color);
    stats.accent = ColorFromHex(accent);
    return stats;
}

EnemyBalance MakeEnemy(std::wstring name, int reward, float rewardThreat, float hp, float hpThreat,
                       float damage, float damageThreat, float range, float attackDelay,
                       float speed, float speedThreat, float radius, bool ranged,
                       unsigned int color, unsigned int accent)
{
    EnemyBalance balance;
    balance.stats.name = std::move(name);
    balance.stats.reward = reward;
    balance.stats.hp = hp;
    balance.stats.damage = damage;
    balance.stats.range = range;
    balance.stats.attackDelay = attackDelay;
    balance.stats.speed = speed;
    balance.stats.radius = radius;
    balance.stats.ranged = ranged;
    balance.stats.color = ColorFromHex(color);
    balance.stats.accent = ColorFromHex(accent);
    balance.rewardThreat = rewardThreat;
    balance.hpThreat = hpThreat;
    balance.damageThreat = damageThreat;
    balance.speedThreat = speedThreat;
    return balance;
}

std::array<StageDefinition, kStageCount> MakeDefaultStages()
{
    return {{
        {L"수성", L"가까운 첫 궤도", L"먼지형과 침형 적",
         2200.0f, 2500.0f, 190.0f, 2.36f, 0.96f, 42.0f,
         ColorFromHex(0x101B22), ColorFromHex(0x1C3037), ColorFromHex(0x13262D), ColorFromHex(0x8A9EA8)},
        {L"금성", L"짙은 대기권", L"산성사수 원거리 적",
         2250.0f, 2850.0f, 205.0f, 2.18f, 1.05f, 39.0f,
         ColorFromHex(0x191722), ColorFromHex(0x332936), ColorFromHex(0x241E2B), ColorFromHex(0xE0B16D)},
        {L"지구", L"푸른 방어선", L"포자병 균형 웨이브",
         2350.0f, 3200.0f, 220.0f, 2.05f, 1.14f, 36.0f,
         ColorFromHex(0x0E1D29), ColorFromHex(0x14303A), ColorFromHex(0x102731), ColorFromHex(0x56A7B7)},
        {L"화성", L"붉은 모래 전선", L"녹슨 장갑형 압박",
         2400.0f, 3550.0f, 230.0f, 1.98f, 1.24f, 33.0f,
         ColorFromHex(0x1D1718), ColorFromHex(0x3A2528), ColorFromHex(0x291D20), ColorFromHex(0xDD7666)},
        {L"목성", L"거대한 폭풍권", L"중력방패 중심",
         2600.0f, 4200.0f, 250.0f, 1.90f, 1.36f, 30.0f,
         ColorFromHex(0x1E1B16), ColorFromHex(0x3A3127), ColorFromHex(0x2B261F), ColorFromHex(0xD8A66A)},
        {L"토성", L"고리 위 방어전", L"고리사수 원거리전",
         2650.0f, 4650.0f, 260.0f, 1.78f, 1.48f, 28.0f,
         ColorFromHex(0x171B20), ColorFromHex(0x2D3037), ColorFromHex(0x222731), ColorFromHex(0xCDBB83)},
        {L"천왕성", L"기울어진 얼음 궤도", L"얼음러너 압박",
         2750.0f, 5200.0f, 275.0f, 1.68f, 1.62f, 25.0f,
         ColorFromHex(0x101D22), ColorFromHex(0x19353A), ColorFromHex(0x122B31), ColorFromHex(0x80E5D4)},
        {L"해왕성", L"먼 푸른 심해", L"해류사수 원거리 러시",
         2850.0f, 5850.0f, 290.0f, 1.56f, 1.78f, 23.0f,
         ColorFromHex(0x101928), ColorFromHex(0x182C48), ColorFromHex(0x13243C), ColorFromHex(0x75A7FF)},
        {L"명왕성", L"가장 먼 어둠", L"공허 중장갑과 빠른 보스",
         3000.0f, 6600.0f, 310.0f, 1.50f, 1.96f, 18.0f,
         ColorFromHex(0x15171F), ColorFromHex(0x292637), ColorFromHex(0x211F2E), ColorFromHex(0xC8B7FF)},
        {L"태양", L"마지막 항성 전선", L"플레어 돌격과 최종 보스",
         3300.0f, 7600.0f, 350.0f, 1.38f, 2.22f, 15.0f,
         ColorFromHex(0x241613), ColorFromHex(0x4A2A1F), ColorFromHex(0x351F18), ColorFromHex(0xFFB347)}
    }};
}

std::array<UnitStats, kRosterCount> MakeDefaultPlayers()
{
    std::array<UnitStats, kRosterCount> table = {};
    table[static_cast<int>(PlayerUnit::Paw)] = MakePlayer(L"기본냥", 75, 1.35f, 130.0f, 22.0f, 30.0f, 0.72f, 68.0f, 16.0f, false, 0xF4FBFF, 0x65B8FF);
    table[static_cast<int>(PlayerUnit::Box)] = MakePlayer(L"방패냥", 145, 4.3f, 460.0f, 14.0f, 28.0f, 1.08f, 38.0f, 21.0f, false, 0xF8F0D6, 0xDCA85B);
    table[static_cast<int>(PlayerUnit::Spark)] = MakePlayer(L"전기냥", 265, 6.4f, 96.0f, 68.0f, 158.0f, 1.78f, 47.0f, 15.0f, true, 0xE8D7FF, 0xBA7BFF);
    table[static_cast<int>(PlayerUnit::Dash)] = MakePlayer(L"질주냥", 115, 2.05f, 86.0f, 18.0f, 26.0f, 0.36f, 118.0f, 13.0f, false, 0xD9FFE6, 0x62DD88);
    table[static_cast<int>(PlayerUnit::Bell)] = MakePlayer(L"종냥", 220, 5.15f, 118.0f, 42.0f, 118.0f, 1.06f, 45.0f, 15.0f, true, 0xFFF8C7, 0xF2C94C);
    table[static_cast<int>(PlayerUnit::Titan)] = MakePlayer(L"거대냥", 390, 9.2f, 760.0f, 88.0f, 39.0f, 1.66f, 24.0f, 27.0f, false, 0xFFE0EF, 0xFF83B7);
    table[static_cast<int>(PlayerUnit::Frost)] = MakePlayer(L"얼음방패냥", 185, 4.85f, 330.0f, 29.0f, 48.0f, 1.02f, 34.0f, 20.0f, false, 0xDDFBFF, 0x74E8FF);
    table[static_cast<int>(PlayerUnit::Comet)] = MakePlayer(L"혜성냥", 155, 3.05f, 104.0f, 34.0f, 34.0f, 0.54f, 136.0f, 14.0f, false, 0xFFF0D8, 0xFF9F4A);
    table[static_cast<int>(PlayerUnit::Orbit)] = MakePlayer(L"궤도냥", 330, 7.2f, 112.0f, 84.0f, 190.0f, 2.05f, 35.0f, 16.0f, true, 0xDBE6FF, 0x88A8FF);
    table[static_cast<int>(PlayerUnit::Solar)] = MakePlayer(L"태양검냥", 480, 10.6f, 520.0f, 126.0f, 58.0f, 1.48f, 42.0f, 24.0f, false, 0xFFE7B5, 0xFFB347);
    table[static_cast<int>(PlayerUnit::Mint)] = MakePlayer(L"지원냥", 245, 5.7f, 170.0f, 36.0f, 126.0f, 0.98f, 43.0f, 16.0f, true, 0xD8FFF3, 0x61E6B0);
    table[static_cast<int>(PlayerUnit::Drill)] = MakePlayer(L"드릴냥", 315, 6.8f, 420.0f, 72.0f, 34.0f, 1.18f, 54.0f, 20.0f, false, 0xE8E0D2, 0xCDAA72);
    table[static_cast<int>(PlayerUnit::Prism)] = MakePlayer(L"프리즘냥", 430, 8.9f, 92.0f, 118.0f, 230.0f, 2.35f, 31.0f, 15.0f, true, 0xF2E8FF, 0xE19BFF);
    table[static_cast<int>(PlayerUnit::Nebula)] = MakePlayer(L"성운포냥", 620, 13.4f, 420.0f, 152.0f, 168.0f, 1.86f, 28.0f, 26.0f, true, 0xE5D9FF, 0x9D83FF);
    return table;
}

std::array<EnemyBalance, kEnemyCount> MakeDefaultEnemies()
{
    std::array<EnemyBalance, kEnemyCount> table = {};
    table[static_cast<int>(EnemyUnit::Dust)] = MakeEnemy(L"먼지졸병", 34, 3.0f, 88.0f, 24.0f, 18.0f, 2.5f, 29.0f, 0.95f, 46.0f, 1.7f, 15.0f, false, 0x3A5163, 0x8FA8B8);
    table[static_cast<int>(EnemyUnit::Brute)] = MakeEnemy(L"철갑병", 78, 5.0f, 310.0f, 78.0f, 31.0f, 4.0f, 34.0f, 1.32f, 31.0f, 0.0f, 22.0f, false, 0x684C75, 0xD5A8EA);
    table[static_cast<int>(EnemyUnit::Skitter)] = MakeEnemy(L"가시러너", 45, 4.0f, 66.0f, 20.0f, 16.0f, 2.0f, 27.0f, 0.62f, 82.0f, 2.0f, 12.0f, false, 0x756D3B, 0xFFE76A);
    table[static_cast<int>(EnemyUnit::Sulfur)] = MakeEnemy(L"산성사수", 50, 4.0f, 94.0f, 25.0f, 19.0f, 2.4f, 88.0f, 1.14f, 42.0f, 1.2f, 15.0f, true, 0xB8794F, 0xFFD27A);
    table[static_cast<int>(EnemyUnit::Moss)] = MakeEnemy(L"포자병", 58, 4.0f, 145.0f, 36.0f, 22.0f, 2.8f, 31.0f, 0.92f, 43.0f, 1.0f, 17.0f, false, 0x385C42, 0x7BDB88);
    table[static_cast<int>(EnemyUnit::Rust)] = MakeEnemy(L"녹슨망치", 86, 6.0f, 390.0f, 88.0f, 36.0f, 4.5f, 36.0f, 1.36f, 28.0f, 0.7f, 24.0f, false, 0x7D3A2D, 0xFF8B60);
    table[static_cast<int>(EnemyUnit::Storm)] = MakeEnemy(L"중력방패", 106, 7.0f, 520.0f, 108.0f, 32.0f, 4.0f, 42.0f, 1.18f, 25.0f, 0.6f, 27.0f, false, 0x8A6846, 0xF1D09A);
    table[static_cast<int>(EnemyUnit::Ring)] = MakeEnemy(L"고리사수", 72, 5.0f, 160.0f, 42.0f, 34.0f, 3.8f, 118.0f, 1.38f, 47.0f, 1.2f, 16.0f, true, 0x736C55, 0xE6D392);
    table[static_cast<int>(EnemyUnit::Frost)] = MakeEnemy(L"얼음러너", 62, 5.0f, 118.0f, 30.0f, 21.0f, 2.7f, 33.0f, 0.74f, 96.0f, 2.4f, 14.0f, false, 0x4E8F95, 0xB9FFF5);
    table[static_cast<int>(EnemyUnit::Tide)] = MakeEnemy(L"해류사수", 76, 6.0f, 132.0f, 35.0f, 28.0f, 3.5f, 132.0f, 1.08f, 70.0f, 1.8f, 17.0f, true, 0x2F5C97, 0x75A7FF);
    table[static_cast<int>(EnemyUnit::Void)] = MakeEnemy(L"공허장갑", 122, 8.0f, 650.0f, 128.0f, 48.0f, 5.4f, 46.0f, 1.28f, 24.0f, 0.5f, 28.0f, false, 0x332B4A, 0xC8B7FF);
    table[static_cast<int>(EnemyUnit::Flare)] = MakeEnemy(L"화염러너", 92, 7.0f, 190.0f, 48.0f, 38.0f, 4.6f, 38.0f, 0.68f, 112.0f, 2.6f, 16.0f, false, 0x9E3F2D, 0xFFB347);
    table[static_cast<int>(EnemyUnit::Spore)] = MakeEnemy(L"포자포병", 82, 6.0f, 210.0f, 54.0f, 31.0f, 3.6f, 112.0f, 1.24f, 36.0f, 1.0f, 18.0f, true, 0x625083, 0xF0A8FF);
    table[static_cast<int>(EnemyUnit::Quake)] = MakeEnemy(L"지진돌격", 145, 9.0f, 820.0f, 150.0f, 58.0f, 6.2f, 48.0f, 1.62f, 19.0f, 0.4f, 32.0f, false, 0x57463D, 0xD6B08C);
    table[static_cast<int>(EnemyUnit::Mirror)] = MakeEnemy(L"거울사수", 88, 7.0f, 165.0f, 44.0f, 35.0f, 4.1f, 152.0f, 1.02f, 62.0f, 1.7f, 15.0f, true, 0xDDEAFF, 0x9CEBFF);
    table[static_cast<int>(EnemyUnit::Comet)] = MakeEnemy(L"혜성추격", 96, 7.0f, 150.0f, 40.0f, 42.0f, 4.8f, 32.0f, 0.54f, 132.0f, 3.0f, 15.0f, false, 0xB7543B, 0xFFDB7A);
    table[static_cast<int>(EnemyUnit::Boss)] = MakeEnemy(L"태양문지기", 240, 12.0f, 1180.0f, 180.0f, 74.0f, 7.0f, 42.0f, 1.45f, 25.0f, 0.0f, 31.0f, false, 0x71323A, 0xFF9BA8);
    return table;
}

std::wstring Trim(std::wstring value)
{
    const wchar_t* spaces = L" \t\r\n";
    const size_t start = value.find_first_not_of(spaces);
    if (start == std::wstring::npos)
    {
        return L"";
    }
    const size_t end = value.find_last_not_of(spaces);
    return value.substr(start, end - start + 1);
}

std::wstring LowerAscii(std::wstring value)
{
    for (wchar_t& ch : value)
    {
        if (ch >= L'A' && ch <= L'Z')
        {
            ch = static_cast<wchar_t>(ch - L'A' + L'a');
        }
    }
    return value;
}

std::vector<std::wstring> SplitCsvLine(const std::wstring& line)
{
    std::vector<std::wstring> fields;
    std::wstring current;
    std::wistringstream stream(line);
    while (std::getline(stream, current, L','))
    {
        fields.push_back(Trim(current));
    }
    return fields;
}

int ToInt(const std::wstring& value, int fallback)
{
    try
    {
        return std::stoi(value);
    }
    catch (...)
    {
        return fallback;
    }
}

float ToFloat(const std::wstring& value, float fallback)
{
    try
    {
        return std::stof(value);
    }
    catch (...)
    {
        return fallback;
    }
}

bool ToBool(const std::wstring& value, bool fallback)
{
    const std::wstring lower = LowerAscii(value);
    if (lower == L"1" || lower == L"true" || lower == L"yes")
    {
        return true;
    }
    if (lower == L"0" || lower == L"false" || lower == L"no")
    {
        return false;
    }
    return fallback;
}

unsigned int ToHexColor(std::wstring value, unsigned int fallback)
{
    value = Trim(value);
    if (!value.empty() && value.front() == L'#')
    {
        value.erase(value.begin());
    }
    if (value.empty())
    {
        return fallback;
    }

    wchar_t* end = nullptr;
    const unsigned long parsed = std::wcstoul(value.c_str(), &end, 16);
    if (end == value.c_str())
    {
        return fallback;
    }
    return static_cast<unsigned int>(parsed & 0xFFFFFFul);
}

std::wstring ExecutableDir()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring fullPath = path;
    const size_t slash = fullPath.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
    {
        return L".\\";
    }
    return fullPath.substr(0, slash + 1);
}

std::wstring BalancePath(const wchar_t* fileName)
{
    return ExecutableDir() + L"data\\" + fileName;
}

std::wifstream OpenUtf8Csv(const std::wstring& path)
{
    std::wifstream file;
    try
    {
        file.imbue(std::locale(".UTF-8"));
    }
    catch (...)
    {
        // UTF-8 로캘을 만들 수 없는 환경에서는 기본 로캘로 읽고, 코드 기본값으로 안전하게 보정한다.
    }
    file.open(path);
    return file;
}

void LoadUnitBalance(BalanceTables& tables)
{
    std::wifstream file = OpenUtf8Csv(BalancePath(L"balance_units.csv"));
    if (!file)
    {
        return;
    }

    std::wstring line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.front() == 0xFEFF)
        {
            line.erase(line.begin());
        }
        const std::wstring trimmed = Trim(line);
        if (trimmed.empty() || trimmed.front() == L'#')
        {
            continue;
        }

        const std::vector<std::wstring> fields = SplitCsvLine(trimmed);
        if (fields.size() < 19 || LowerAscii(fields[0]) == L"type")
        {
            continue;
        }

        const std::wstring type = LowerAscii(fields[0]);
        const int index = ToInt(fields[1], -1);
        if (type == L"player" && index >= 0 && index < kRosterCount)
        {
            UnitStats stats = tables.players[index];
            stats.name = fields[2];
            stats.cost = ToInt(fields[3], stats.cost);
            stats.reward = ToInt(fields[4], stats.reward);
            stats.cooldown = ToFloat(fields[5], stats.cooldown);
            stats.hp = ToFloat(fields[6], stats.hp);
            stats.damage = ToFloat(fields[7], stats.damage);
            stats.range = ToFloat(fields[8], stats.range);
            stats.attackDelay = ToFloat(fields[9], stats.attackDelay);
            stats.speed = ToFloat(fields[10], stats.speed);
            stats.radius = ToFloat(fields[11], stats.radius);
            stats.ranged = ToBool(fields[12], stats.ranged);
            stats.color = ColorFromHex(ToHexColor(fields[13], 0xFFFFFF));
            stats.accent = ColorFromHex(ToHexColor(fields[14], 0xFFFFFF));
            tables.players[index] = stats;
        }
        else if (type == L"enemy" && index >= 0 && index < kEnemyCount)
        {
            EnemyBalance balance = tables.enemies[index];
            balance.stats.name = fields[2];
            balance.stats.cost = ToInt(fields[3], balance.stats.cost);
            balance.stats.reward = ToInt(fields[4], balance.stats.reward);
            balance.stats.cooldown = ToFloat(fields[5], balance.stats.cooldown);
            balance.stats.hp = ToFloat(fields[6], balance.stats.hp);
            balance.stats.damage = ToFloat(fields[7], balance.stats.damage);
            balance.stats.range = ToFloat(fields[8], balance.stats.range);
            balance.stats.attackDelay = ToFloat(fields[9], balance.stats.attackDelay);
            balance.stats.speed = ToFloat(fields[10], balance.stats.speed);
            balance.stats.radius = ToFloat(fields[11], balance.stats.radius);
            balance.stats.ranged = ToBool(fields[12], balance.stats.ranged);
            balance.stats.color = ColorFromHex(ToHexColor(fields[13], 0xFFFFFF));
            balance.stats.accent = ColorFromHex(ToHexColor(fields[14], 0xFFFFFF));
            balance.hpThreat = ToFloat(fields[15], balance.hpThreat);
            balance.damageThreat = ToFloat(fields[16], balance.damageThreat);
            balance.rewardThreat = ToFloat(fields[17], balance.rewardThreat);
            balance.speedThreat = ToFloat(fields[18], balance.speedThreat);
            tables.enemies[index] = balance;
        }
    }
}

void LoadStageBalance(BalanceTables& tables)
{
    std::wifstream file = OpenUtf8Csv(BalancePath(L"balance_stages.csv"));
    if (!file)
    {
        return;
    }

    std::wstring line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.front() == 0xFEFF)
        {
            line.erase(line.begin());
        }
        const std::wstring trimmed = Trim(line);
        if (trimmed.empty() || trimmed.front() == L'#')
        {
            continue;
        }

        const std::vector<std::wstring> fields = SplitCsvLine(trimmed);
        if (fields.size() < 14 || LowerAscii(fields[0]) == L"index")
        {
            continue;
        }

        const int index = ToInt(fields[0], -1);
        if (index < 0 || index >= kStageCount)
        {
            continue;
        }

        StageDefinition stage = tables.stages[index];
        stage.name = fields[1];
        stage.subtitle = fields[2];
        stage.gimmick = fields[3];
        stage.playerHp = ToFloat(fields[4], stage.playerHp);
        stage.enemyHp = ToFloat(fields[5], stage.enemyHp);
        stage.startEnergy = ToFloat(fields[6], stage.startEnergy);
        stage.enemyInterval = ToFloat(fields[7], stage.enemyInterval);
        stage.threatScale = ToFloat(fields[8], stage.threatScale);
        stage.bossFirstTime = ToFloat(fields[9], stage.bossFirstTime);
        stage.backColor = ColorFromHex(ToHexColor(fields[10], 0x0D1B23));
        stage.laneColor = ColorFromHex(ToHexColor(fields[11], 0x142935));
        stage.laneInnerColor = ColorFromHex(ToHexColor(fields[12], 0x10212D));
        stage.lineColor = ColorFromHex(ToHexColor(fields[13], 0x4A6272));
        tables.stages[index] = stage;
    }
}

BalanceTables MakeBalanceTables()
{
    BalanceTables tables{MakeDefaultStages(), MakeDefaultPlayers(), MakeDefaultEnemies()};
    LoadStageBalance(tables);
    LoadUnitBalance(tables);
    return tables;
}

const BalanceTables& Tables()
{
    static const BalanceTables tables = MakeBalanceTables();
    return tables;
}
}

StageDefinition GetStageDefinition(int index)
{
    return Tables().stages[ClampIndex(index, kStageCount)];
}

UnitStats GetPlayerStats(PlayerUnit unit)
{
    return Tables().players[ClampIndex(static_cast<int>(unit), kRosterCount)];
}

UnitStats GetEnemyStats(EnemyUnit unit, float threat)
{
    const EnemyBalance& balance = Tables().enemies[ClampIndex(static_cast<int>(unit), kEnemyCount)];
    UnitStats stats = balance.stats;
    stats.reward += static_cast<int>(threat * balance.rewardThreat);
    stats.hp += threat * balance.hpThreat;
    stats.damage += threat * balance.damageThreat;
    stats.speed += threat * balance.speedThreat;
    return stats;
}
