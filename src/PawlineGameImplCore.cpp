#include "PawlineGameImpl.h"

// Lifecycle, persistence, and high-level frame update.
PawlineGameImpl::~PawlineGameImpl()
{
    DiscardDeviceResources();
    SafeRelease(&m_roundStroke);
    SafeRelease(&m_centerFormat);
    SafeRelease(&m_buttonFormat);
    SafeRelease(&m_smallFormat);
    SafeRelease(&m_bodyFormat);
    SafeRelease(&m_headerFormat);
    SafeRelease(&m_titleFormat);
    SafeRelease(&m_writeFactory);
    SafeRelease(&m_factory);
}

HRESULT PawlineGameImpl::Initialize()
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_factory);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_writeFactory));
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CreateTextFormats();
    if (FAILED(hr))
    {
        return hr;
    }

    const D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties(
        D2D1_CAP_STYLE_ROUND,
        D2D1_CAP_STYLE_ROUND,
        D2D1_CAP_STYLE_ROUND,
        D2D1_LINE_JOIN_ROUND);
    hr = m_factory->CreateStrokeStyle(strokeProps, nullptr, 0, &m_roundStroke);
    if (FAILED(hr))
    {
        return hr;
    }

    LoadProgress();
    ResetToTitle();

    const wchar_t className[] = L"PawlineDefenseWindow";
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = PawlineGameImpl::WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.lpszClassName = className;
    RegisterClassExW(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT workArea = {};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    const LONG workWidth = std::max<LONG>(960, workArea.right - workArea.left - 72);
    const LONG workHeight = std::max<LONG>(600, workArea.bottom - workArea.top - 72);
    const LONG clientWidth = std::min<LONG>(static_cast<LONG>(kWidth), workWidth);
    const LONG clientHeight = std::min<LONG>(static_cast<LONG>(kHeight), workHeight);
    RECT rect = {0, 0, clientWidth, clientHeight};
    AdjustWindowRect(&rect, style, FALSE);

    m_hwnd = CreateWindowExW(
        0,
        className,
        L"Pawline Defense - Direct2D",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this);

    if (!m_hwnd)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    UpdateWindow(m_hwnd);

    m_timer.Reset();
    timeBeginPeriod(1);
    return S_OK;
}

int PawlineGameImpl::Run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }

        // Clamp delta time so a dragged window or breakpoint does not make one
        // frame simulate a huge jump in movement, cooldowns, and projectiles.
        float dt = std::min(m_timer.Tick(), 0.033f);

        Update(dt);
        Render();
        Sleep(1);
    }
    timeEndPeriod(1);
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK PawlineGameImpl::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* app = reinterpret_cast<PawlineGameImpl*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        app->m_hwnd = hwnd;
    }

    auto* app = reinterpret_cast<PawlineGameImpl*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (app)
    {
        return app->HandleMessage(message, wParam, lParam);
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT PawlineGameImpl::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_DISPLAYCHANGE:
        InvalidateRect(m_hwnd, nullptr, FALSE);
        return 0;
    case WM_SIZE:
        if (m_renderTarget)
        {
            m_renderTarget->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
            UpdateViewMetrics();
        }
        return 0;
        case WM_MOUSEMOVE:
        {
            const Vec2 nextMouse = ClientToVirtual({static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam))});
            const bool draggingBattle = (wParam & MK_LBUTTON) != 0 &&
                                        (m_screen == GameScreen::Playing || m_screen == GameScreen::Result) &&
                                        !m_escapeMenuOpen &&
                                        m_mouse.y >= kBattleTop && m_mouse.y <= kBattleBottom &&
                                        nextMouse.y >= kBattleTop && nextMouse.y <= kBattleBottom;
            if (draggingBattle)
            {
                m_cameraTargetX = std::max(0.0f, std::min(kCameraMaxX, m_cameraTargetX + (m_mouse.x - nextMouse.x)));
                m_cameraX = std::max(0.0f, std::min(kCameraMaxX, m_cameraX + (m_mouse.x - nextMouse.x)));
            }
            m_mouse = nextMouse;
            return 0;
        }
        case WM_LBUTTONDOWN:
            SetFocus(m_hwnd);
            OnLeftClick(ClientToVirtual({static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam))}));
            return 0;
        case WM_MOUSEWHEEL:
            if (m_screen == GameScreen::Playing || m_screen == GameScreen::Result)
            {
                const float steps = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
                m_cameraTargetX = std::max(0.0f, std::min(kCameraMaxX, m_cameraTargetX - steps * 260.0f));
                return 0;
            }
            break;
        case WM_KEYDOWN:
            OnKeyDown(wParam);
            return 0;
    }
    return DefWindowProcW(m_hwnd, message, wParam, lParam);
}

HRESULT PawlineGameImpl::CreateTextFormats()
{
    HRESULT hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"en-us", &m_titleFormat);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"en-us", &m_headerFormat);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 15.0f, L"en-us", &m_bodyFormat);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 12.5f, L"en-us", &m_smallFormat);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 15.0f, L"en-us", &m_buttonFormat);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"en-us", &m_centerFormat);
    if (FAILED(hr))
    {
        return hr;
    }

    m_titleFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    m_buttonFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_buttonFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    m_centerFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_centerFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    return S_OK;
}

HRESULT PawlineGameImpl::CreateDeviceResources()
{
    if (m_renderTarget)
    {
        return S_OK;
    }

    RECT rc = {};
    GetClientRect(m_hwnd, &rc);
    HRESULT hr = m_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            m_hwnd,
            D2D1::SizeU(static_cast<UINT32>(rc.right - rc.left), static_cast<UINT32>(rc.bottom - rc.top))),
        &m_renderTarget);
    if (FAILED(hr))
    {
        return hr;
    }

    return m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF), &m_brush);
}

void PawlineGameImpl::DiscardDeviceResources()
{
    SafeRelease(&m_brush);
    SafeRelease(&m_renderTarget);
}

const StageDefinition PawlineGameImpl::CurrentStage() const
{
    return GetStageDefinition(m_selectedStage);
}

int PawlineGameImpl::UnitIndex(PlayerUnit unit) const
{
    return std::max(0, std::min(kRosterCount - 1, static_cast<int>(unit)));
}

bool PawlineGameImpl::IsUnitUnlocked(PlayerUnit unit) const
{
    return m_unitUnlocked[UnitIndex(unit)];
}

int PawlineGameImpl::UnitLevel(PlayerUnit unit) const
{
    return m_unitLevels[UnitIndex(unit)];
}

UnitStats PawlineGameImpl::PlayerStats(PlayerUnit unit) const
{
    UnitStats stats = GetPlayerStats(unit);
    const int level = UnitLevel(unit);
    const float hpScale = 1.0f + static_cast<float>(level - 1) * 0.22f;
    const float damageScale = 1.0f + static_cast<float>(level - 1) * 0.18f;
    const float cooldownScale = std::max(0.74f, 1.0f - static_cast<float>(level - 1) * 0.045f);
    stats.hp *= hpScale;
    stats.damage *= damageScale;
    stats.cooldown *= cooldownScale;
    stats.cost += (level - 1) * 10;
    return stats;
}

int PawlineGameImpl::UnitUnlockCost(PlayerUnit unit) const
{
    const int index = UnitIndex(unit);
    if (index < 5)
    {
        return 0;
    }
    return 180 + index * 60 + (index >= 10 ? 90 : 0);
}

int PawlineGameImpl::UnitUpgradeCost(PlayerUnit unit) const
{
    const int level = UnitLevel(unit);
    if (level >= kMaxUnitLevel)
    {
        return 0;
    }
    const int index = UnitIndex(unit);
    return 130 + level * 105 + index * 28;
}

int PawlineGameImpl::StageClearReward(bool firstClear) const
{
    const int stageBonus = 240 + m_selectedStage * 90;
    const int scoreBonus = std::min(260, m_score / 45);
    const int timeBonus = std::max(0, 150 - static_cast<int>(m_stageTime * 1.2f));
    const int firstBonus = firstClear ? 280 + m_selectedStage * 80 : 0;
    return stageBonus + scoreBonus + timeBonus + firstBonus;
}

void PawlineGameImpl::GrantStageReward()
{
    const bool firstClear = !m_stageCleared[m_selectedStage];
    m_stageCleared[m_selectedStage] = true;
    m_lastReward = StageClearReward(firstClear);
    m_lumen += m_lastReward;
    AddFloatText({640.0f, 154.0f}, L"+" + ToWideInt(m_lastReward) + L" LUMEN", D2D1::ColorF(0xF6FF83), 1.5f);
    AddRing({640.0f, kLaneY}, 260.0f, 0.65f, D2D1::ColorF(0xF6FF83, 0.48f), 4.0f);
    SaveProgress();
}

std::wstring PawlineGameImpl::ProgressPath() const
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring fullPath = path;
    const size_t slash = fullPath.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
    {
        return L"pawline_progress.txt";
    }
    return fullPath.substr(0, slash + 1) + L"pawline_progress.txt";
}

void PawlineGameImpl::LoadProgress()
{
    std::wifstream file(ProgressPath());
    if (!file)
    {
        return;
    }

    file >> m_lumen;
    for (int i = 0; i < kRosterCount; ++i)
    {
        int unlocked = 0;
        file >> unlocked;
        if (file)
        {
            m_unitUnlocked[i] = unlocked != 0;
        }
    }
    for (int i = 0; i < kRosterCount; ++i)
    {
        int level = 1;
        file >> level;
        if (file)
        {
            m_unitLevels[i] = std::max(1, std::min(kMaxUnitLevel, level));
        }
    }
    for (int i = 0; i < kStageCount; ++i)
    {
        int cleared = 0;
        file >> cleared;
        if (file)
        {
            m_stageCleared[i] = cleared != 0;
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        m_unitUnlocked[i] = true;
    }
}

void PawlineGameImpl::SaveProgress() const
{
    std::wofstream file(ProgressPath());
    if (!file)
    {
        return;
    }

    file << m_lumen << L"\n";
    for (bool unlocked : m_unitUnlocked)
    {
        file << (unlocked ? 1 : 0) << L" ";
    }
    file << L"\n";
    for (int level : m_unitLevels)
    {
        file << level << L" ";
    }
    file << L"\n";
    for (bool cleared : m_stageCleared)
    {
        file << (cleared ? 1 : 0) << L" ";
    }
    file << L"\n";
}

void PawlineGameImpl::ResetToTitle()
{
    m_screen = GameScreen::Title;
    m_units.clear();
    m_projectiles.clear();
    m_particles.clear();
    m_rings.clear();
    m_beams.clear();
    m_sparkLines.clear();
    m_floatTexts.clear();
    m_uiPulses.clear();
    m_screenFlash = 0.0f;
    m_paused = false;
    m_escapeMenuOpen = false;
    m_pauseBeforeEscape = false;
    m_gameOver = false;
    m_victory = false;
    m_message.clear();
    m_messageTimer = 0.0f;
}

void PawlineGameImpl::ResetToMenu()
{
    m_screen = GameScreen::Menu;
    m_units.clear();
    m_projectiles.clear();
    m_particles.clear();
    m_rings.clear();
    m_beams.clear();
    m_sparkLines.clear();
    m_floatTexts.clear();
    m_uiPulses.clear();
    m_screenFlash = 0.0f;
    m_paused = false;
    m_escapeMenuOpen = false;
    m_pauseBeforeEscape = false;
    m_gameOver = false;
    m_victory = false;
    m_message.clear();
    m_messageTimer = 0.0f;
}

void PawlineGameImpl::ResetGame()
{
    const StageDefinition stage = CurrentStage();
    m_screen = GameScreen::Playing;
    m_units.clear();
    m_projectiles.clear();
    m_particles.clear();
    m_rings.clear();
    m_beams.clear();
    m_sparkLines.clear();
    m_floatTexts.clear();
    m_uiPulses.clear();

    m_cardCooldowns.fill(0.0f);
    m_walletLevel = 1;
    m_energy = stage.startEnergy;
    m_playerBaseHp = stage.playerHp;
    m_enemyBaseHp = stage.enemyHp;
    m_playerBaseMaxHp = m_playerBaseHp;
    m_enemyBaseMaxHp = m_enemyBaseHp;
    m_stageTime = 0.0f;
    m_gameSpeed = m_defaultGameSpeed;
    m_enemyTimer = stage.enemyInterval;
    m_nextBossTime = stage.bossFirstTime;
    m_cannonCharge = 35.0f;
    m_cannonFlash = 0.0f;
    m_screenFlash = 0.0f;
    m_walletPulseTimer = WalletPulseInterval();
    m_cameraX = 0.0f;
    m_cameraTargetX = 0.0f;
    m_playerBaseShake = 0.0f;
    m_enemyBaseShake = 0.0f;
    m_score = 0;
    m_lastReward = 0;
    m_nextUnitId = 1;
    m_paused = false;
    m_escapeMenuOpen = false;
    m_pauseBeforeEscape = false;
    m_gameOver = false;
    m_victory = false;
    SetMessage(stage.name + L": summon units and break the enemy base.");
}

void PawlineGameImpl::Update(float dt)
{
    // UI time always moves, but combat time below is multiplied by m_gameSpeed.
    // This keeps menus/VFX alive even when the battle itself is paused.
    m_uiTime += dt;
    if (m_messageTimer > 0.0f)
    {
        m_messageTimer -= dt;
    }
    if (m_playerBaseShake > 0.0f)
    {
        m_playerBaseShake -= dt;
    }
    if (m_enemyBaseShake > 0.0f)
    {
        m_enemyBaseShake -= dt;
    }
    if (m_cannonFlash > 0.0f)
    {
        m_cannonFlash -= dt;
    }
    if (m_screenFlash > 0.0f)
    {
        m_screenFlash -= dt;
    }

    if (m_screen == GameScreen::Title || m_screen == GameScreen::Options || m_screen == GameScreen::Menu || m_screen == GameScreen::Shop || m_screen == GameScreen::Result)
    {
        if (m_screen == GameScreen::Result)
        {
            UpdateCamera(dt);
        }
        UpdateParticles(dt);
        CleanupEntities();
        return;
    }

    if (m_paused || m_gameOver || m_victory)
    {
        UpdateParticles(dt);
        CleanupEntities();
        return;
    }

    // All gameplay systems consume gameDt, so speed control stays centralized.
    const float gameDt = dt * m_gameSpeed;
    m_stageTime += gameDt;
    UpdateCamera(gameDt);
    m_energy = std::min(MaxEnergy(), m_energy + EnergyRegen() * gameDt);
    m_cannonCharge = std::min(100.0f, m_cannonCharge + (6.8f + static_cast<float>(m_walletLevel) * 1.1f) * gameDt);
    UpdateWalletPulse(gameDt);

    for (float& cooldown : m_cardCooldowns)
    {
        cooldown = std::max(0.0f, cooldown - gameDt);
    }

    UpdateEnemyDirector(gameDt);
    UpdateUnits(gameDt);
    UpdateProjectiles(gameDt);
    UpdateParticles(gameDt);
    CleanupEntities();

    if (m_enemyBaseHp <= 0.0f)
    {
        m_victory = true;
        m_screen = GameScreen::Result;
        m_enemyBaseHp = 0.0f;
        m_resultScore = m_score + static_cast<int>(std::max(0.0f, m_playerBaseHp)) + m_walletLevel * 300;
        m_resultTime = m_stageTime;
        GrantStageReward();
        SetMessage(L"Enemy base destroyed. Lumen earned.");
    }
    if (m_playerBaseHp <= 0.0f)
    {
        m_gameOver = true;
        m_screen = GameScreen::Result;
        m_playerBaseHp = 0.0f;
        m_resultScore = m_score;
        m_resultTime = m_stageTime;
        SetMessage(L"Home base fell. Press R to rebuild.");
    }
}

void PawlineGameImpl::UpdateViewMetrics()
{
    if (!m_renderTarget)
    {
        m_viewScale = 1.0f;
        m_viewOffsetX = 0.0f;
        m_viewOffsetY = 0.0f;
        return;
    }

    const D2D1_SIZE_F size = m_renderTarget->GetSize();
    const float fitScale = std::max(0.25f, std::min(size.width / kWidth, size.height / kHeight));
    m_viewScale = fitScale * m_userViewScale;
    m_viewOffsetX = (size.width - kWidth * m_viewScale) * 0.5f;
    m_viewOffsetY = (size.height - kHeight * m_viewScale) * 0.5f;
}

void PawlineGameImpl::SetViewTransform(float worldCameraX)
{
    const D2D1_MATRIX_3X2_F matrix =
        D2D1::Matrix3x2F::Translation(-worldCameraX, 0.0f) *
        D2D1::Matrix3x2F::Scale(m_viewScale, m_viewScale) *
        D2D1::Matrix3x2F::Translation(m_viewOffsetX, m_viewOffsetY);
    m_renderTarget->SetTransform(matrix);
}

Vec2 PawlineGameImpl::ClientToVirtual(Vec2 pos) const
{
    const float safeScale = std::max(0.001f, m_viewScale);
    return {
        (pos.x - m_viewOffsetX) / safeScale,
        (pos.y - m_viewOffsetY) / safeScale};
}

void PawlineGameImpl::UpdateCamera(float dt)
{
    if (m_screen != GameScreen::Playing && m_screen != GameScreen::Result)
    {
        m_cameraTargetX = 0.0f;
    }
    else
    {
        const bool inBattleView = m_mouse.y >= kBattleTop && m_mouse.y <= kBattleBottom;
        const float edge = 118.0f;
        if (inBattleView && !m_escapeMenuOpen)
        {
            if (m_mouse.x < edge)
            {
                const float strength = Clamp01((edge - m_mouse.x) / edge);
                m_cameraTargetX -= (560.0f + strength * 780.0f) * strength * dt;
            }
            else if (m_mouse.x > kWidth - edge)
            {
                const float strength = Clamp01((m_mouse.x - (kWidth - edge)) / edge);
                m_cameraTargetX += (560.0f + strength * 780.0f) * strength * dt;
            }
        }

        if (m_screen == GameScreen::Playing && m_units.empty() && m_stageTime < 2.0f)
        {
            m_cameraTargetX = 0.0f;
        }
    }

    m_cameraTargetX = std::max(0.0f, std::min(kCameraMaxX, m_cameraTargetX));
    const float follow = 1.0f - std::pow(0.001f, std::min(dt, 0.05f));
    m_cameraX = Lerp(m_cameraX, m_cameraTargetX, follow);
}
