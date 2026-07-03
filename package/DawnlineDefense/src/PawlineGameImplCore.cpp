#include "PawlineGameImpl.h"

// Lifecycle, persistence, and high-level frame update.
PawlineGameImpl::~PawlineGameImpl()
{
    DiscardDeviceResources();
    UnregisterPrivateFonts();
    if (m_comInitialized)
    {
        CoUninitialize();
    }
}

HRESULT PawlineGameImpl::Initialize()
{
    const HRESULT coHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(coHr))
    {
        m_comInitialized = true;
    }
    else if (coHr != RPC_E_CHANGED_MODE)
    {
        return coHr;
    }

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_factory.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_writeFactory.GetAddressOf()));
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_wicFactory.GetAddressOf()));
    if (FAILED(hr))
    {
        return hr;
    }

    RegisterPrivateFonts();

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
    hr = m_factory->CreateStrokeStyle(strokeProps, nullptr, 0, m_roundStroke.GetAddressOf());
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
    const wchar_t* uiFont = m_privateFontLoaded ? L"Galmuri11" : L"Segoe UI";
    HRESULT hr = m_writeFactory->CreateTextFormat(
        uiFont, nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"en-us", m_titleFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        uiFont, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"en-us", m_headerFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        uiFont, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 15.0f, L"en-us", m_bodyFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        uiFont, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 12.5f, L"en-us", m_smallFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        uiFont, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 15.0f, L"en-us", m_buttonFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        uiFont, nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"en-us", m_centerFormat.GetAddressOf());
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
        m_renderTarget.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF), m_brush.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        LoadBitmapAssets();
    }
    return hr;
}

void PawlineGameImpl::DiscardDeviceResources()
{
    DiscardBitmapAssets();
    m_brush.Reset();
    m_renderTarget.Reset();
}

void PawlineGameImpl::RegisterPrivateFonts()
{
    m_privateFontPath = AssetPath(L"assets\\fonts\\Galmuri11.ttf");
    if (GetFileAttributesW(m_privateFontPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        return;
    }
    m_privateFontLoaded = AddFontResourceExW(m_privateFontPath.c_str(), FR_PRIVATE, nullptr) > 0;
}

void PawlineGameImpl::UnregisterPrivateFonts()
{
    if (!m_privateFontLoaded || m_privateFontPath.empty())
    {
        return;
    }
    RemoveFontResourceExW(m_privateFontPath.c_str(), FR_PRIVATE, nullptr);
    m_privateFontLoaded = false;
}

std::wstring PawlineGameImpl::ExecutableDir() const
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

std::wstring PawlineGameImpl::AssetPath(const std::wstring& relativePath) const
{
    const std::wstring base = ExecutableDir();
    const std::wstring direct = base + relativePath;
    if (GetFileAttributesW(direct.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        return direct;
    }
    return base + L"..\\..\\" + relativePath;
}

HRESULT PawlineGameImpl::LoadBitmapFromFile(const std::wstring& path, ID2D1Bitmap** bitmap) const
{
    if (!m_wicFactory || !m_renderTarget || !bitmap)
    {
        return E_FAIL;
    }

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = m_wicFactory->CreateDecoderFromFilename(
        path.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        decoder.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = m_wicFactory->CreateFormatConverter(converter.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr))
    {
        return hr;
    }

    return m_renderTarget->CreateBitmapFromWicBitmap(converter.Get(), nullptr, bitmap);
}

void PawlineGameImpl::LoadBitmapAssets()
{
    if (m_bitmapAssetsLoaded || !m_renderTarget)
    {
        return;
    }

    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\combat_vfx_atlas.png"), m_vfxAtlas.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\slash_effect_sheet.png"), m_slashEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\slash_gameboy_effect_sheet.png"), m_enemySlashEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\heal_effect_sheet.png"), m_healEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\heal_gameboy_effect_sheet.png"), m_healSoftEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\ui\\pawline_ui_atlas.png"), m_uiAtlas.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\cutins\\solar_gatekeeper_cutin.png"), m_bossCutin.ReleaseAndGetAddressOf());

    for (int i = 0; i < kStageCount; ++i)
    {
        std::wstringstream path;
        path << L"assets\\backgrounds\\stage_" << std::setw(2) << std::setfill(L'0') << i << L"_space.png";
        LoadBitmapFromFile(AssetPath(path.str()), m_backgroundBitmaps[i].ReleaseAndGetAddressOf());
    }

    m_bitmapAssetsLoaded = true;
}

void PawlineGameImpl::DiscardBitmapAssets()
{
    m_vfxAtlas.Reset();
    m_slashEffectSheet.Reset();
    m_enemySlashEffectSheet.Reset();
    m_healEffectSheet.Reset();
    m_healSoftEffectSheet.Reset();
    m_uiAtlas.Reset();
    m_bossCutin.Reset();
    for (auto& bitmap : m_backgroundBitmaps)
    {
        bitmap.Reset();
    }
    m_bitmapAssetsLoaded = false;
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

bool PawlineGameImpl::IsUnitEvolved(PlayerUnit unit) const
{
    return IsUnitUnlocked(unit) && UnitLevel(unit) >= kMaxUnitLevel;
}

std::wstring PawlineGameImpl::UnitDisplayName(PlayerUnit unit) const
{
    const UnitStats base = GetPlayerStats(unit);
    return IsUnitEvolved(unit) ? L"진화 " + base.name : base.name;
}

UnitStats PawlineGameImpl::PlayerStats(PlayerUnit unit) const
{
    UnitStats stats = GetPlayerStats(unit);
    const int level = UnitLevel(unit);
    const float hpScale = 1.0f + static_cast<float>(level - 1) * 0.22f;
    const float damageScale = 1.0f + static_cast<float>(level - 1) * 0.18f;
    const float cooldownScale = std::max(0.74f, 1.0f - static_cast<float>(level - 1) * 0.045f);
    stats.hp *= hpScale * SynergyHpMultiplier(unit);
    stats.damage *= damageScale * SynergyDamageMultiplier(unit);
    stats.range *= SynergyRangeMultiplier(unit);
    stats.speed *= SynergySpeedMultiplier(unit);
    stats.cooldown *= cooldownScale;
    stats.cost += (level - 1) * 10;
    if (IsUnitEvolved(unit))
    {
        // 레벨 5 유닛은 이름과 전투 성능이 한 번 더 변해, 성장의 보상이 눈에 보이게 한다.
        stats.name = UnitDisplayName(unit);
        stats.hp *= 1.10f;
        stats.damage *= 1.12f;
        stats.range *= stats.ranged ? 1.06f : 1.0f;
        stats.speed *= 1.04f;
        stats.cooldown *= 0.94f;
    }
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
    return static_cast<int>(std::round(static_cast<float>(stageBonus + scoreBonus + timeBonus + firstBonus) * DifficultyRewardMultiplier()));
}

float PawlineGameImpl::DifficultyThreatMultiplier() const
{
    switch (m_difficulty)
    {
    case Difficulty::Easy:
        return 0.84f;
    case Difficulty::Hard:
        return 1.22f;
    default:
        return 1.0f;
    }
}

float PawlineGameImpl::DifficultyRewardMultiplier() const
{
    switch (m_difficulty)
    {
    case Difficulty::Easy:
        return 0.88f;
    case Difficulty::Hard:
        return 1.28f;
    default:
        return 1.0f;
    }
}

std::wstring PawlineGameImpl::DifficultyLabel() const
{
    switch (m_difficulty)
    {
    case Difficulty::Easy:
        return L"EASY";
    case Difficulty::Hard:
        return L"HARD";
    default:
        return L"NORMAL";
    }
}

bool PawlineGameImpl::IsStageUnlocked(int index) const
{
    if (index <= 0 || m_debugMode)
    {
        return true;
    }
    if (index >= kStageCount)
    {
        return false;
    }
    return m_stageCleared[index] || m_stageCleared[index - 1];
}

int PawlineGameImpl::HighestUnlockedStage() const
{
    int highest = 0;
    for (int i = 1; i < kStageCount; ++i)
    {
        if (IsStageUnlocked(i))
        {
            highest = i;
        }
    }
    return highest;
}

void PawlineGameImpl::SelectStage(int index)
{
    const int clamped = std::max(0, std::min(kStageCount - 1, index));
    if (!IsStageUnlocked(clamped))
    {
        SetMessage(L"이전 스테이지를 먼저 클리어해야 해.");
        AddUiPulse({m_mouse.x, m_mouse.y}, D2D1::ColorF(0xFF9BA8));
        return;
    }
    m_selectedStage = clamped;
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
    return ProgressPath(m_saveSlot);
}

std::wstring PawlineGameImpl::ProgressPath(int slot) const
{
    // 실행 파일과 같은 폴더에 저장해서 ZIP 제출본을 다른 PC로 옮겨도 진행 파일 위치를 찾기 쉽다.
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring fullPath = path;
    const size_t slash = fullPath.find_last_of(L"\\/");
    const int safeSlot = std::clamp(slot, 0, kSaveSlotCount - 1);
    const std::wstring fileName = L"pawline_progress_slot" + ToWideInt(safeSlot + 1) + L".txt";
    if (slash == std::wstring::npos)
    {
        return fileName;
    }
    return fullPath.substr(0, slash + 1) + fileName;
}

std::wstring PawlineGameImpl::LegacyProgressPath() const
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

std::wstring PawlineGameImpl::SaveSlotLabel() const
{
    return L"슬롯 " + ToWideInt(m_saveSlot + 1);
}

void PawlineGameImpl::LoadProgress()
{
    LoadProgress(m_saveSlot);
}

void PawlineGameImpl::LoadProgress(int slot)
{
    m_saveSlot = std::clamp(slot, 0, kSaveSlotCount - 1);
    // 진행 데이터, 편성, 옵션을 불러온다. 파일이 없으면 기본값 그대로 시작한다.
    std::wifstream file(ProgressPath());
    if (!file && m_saveSlot == 0)
    {
        file.clear();
        file.open(LegacyProgressPath());
    }
    if (!file)
    {
        SetMessage(SaveSlotLabel() + L"에 저장 데이터가 아직 없어.");
        return;
    }

    // 저장 파일은 v2부터 태그를 가진다. 태그가 없으면 예전 숫자 시작 형식으로 읽는다.
    std::wstring header;
    file >> header;
    const bool version2 = header == L"PAWLINE_PROGRESS_V2";
    if (version2)
    {
        file >> m_lumen;
    }
    else
    {
        std::wistringstream legacy(header);
        legacy >> m_lumen;
    }

    for (bool& unlockedState : m_unitUnlocked)
    {
        int unlocked = 0;
        file >> unlocked;
        if (file)
        {
            unlockedState = unlocked != 0;
        }
    }
    for (int& levelState : m_unitLevels)
    {
        int level = 1;
        file >> level;
        if (file)
        {
            levelState = std::clamp(level, 1, kMaxUnitLevel);
        }
    }
    for (bool& clearedState : m_stageCleared)
    {
        int cleared = 0;
        file >> cleared;
        if (file)
        {
            clearedState = cleared != 0;
        }
    }

    if (version2)
    {
        for (PlayerUnit& unit : m_loadout)
        {
            int unitIndex = 0;
            file >> unitIndex;
            if (file)
            {
                unit = static_cast<PlayerUnit>(std::clamp(unitIndex, 0, kRosterCount - 1));
            }
        }

        file >> m_selectedStage >> m_selectedLoadoutSlot;
        m_selectedStage = std::clamp(m_selectedStage, 0, kStageCount - 1);
        m_selectedLoadoutSlot = std::clamp(m_selectedLoadoutSlot, 0, kLoadoutSize - 1);

        int shake = 1;
        int reduceFlash = 0;
        file >> m_defaultGameSpeed >> m_userViewScale >> shake >> reduceFlash;
        if (file)
        {
            m_defaultGameSpeed = std::clamp(m_defaultGameSpeed, 0.5f, 3.0f);
            m_userViewScale = std::clamp(m_userViewScale, 0.82f, 1.0f);
            m_hitShakeEnabled = shake != 0;
            m_reduceFlashes = reduceFlash != 0;
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        m_unitUnlocked[i] = true;
    }
    if (!IsStageUnlocked(m_selectedStage))
    {
        m_selectedStage = HighestUnlockedStage();
    }
}

void PawlineGameImpl::SaveProgress()
{
    SaveProgressToSlot(m_saveSlot);
    m_autoSaveNotice = L"자동 저장됨  " + SaveSlotLabel();
    m_autoSaveNoticeTimer = 1.85f;
}

void PawlineGameImpl::SaveProgressToSlot(int slot) const
{
    // v2 저장 파일은 성장 상태뿐 아니라 편성/옵션까지 같이 기록한다.
    std::wofstream file(ProgressPath(slot));
    if (!file)
    {
        return;
    }

    file << L"PAWLINE_PROGRESS_V2\n";
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
    for (PlayerUnit unit : m_loadout)
    {
        file << static_cast<int>(unit) << L" ";
    }
    file << L"\n";
    file << m_selectedStage << L" " << m_selectedLoadoutSlot << L"\n";
    file << m_defaultGameSpeed << L" " << m_userViewScale << L" "
         << (m_hitShakeEnabled ? 1 : 0) << L" " << (m_reduceFlashes ? 1 : 0) << L"\n";
}

void PawlineGameImpl::SelectSaveSlot(int slot)
{
    const int safeSlot = std::clamp(slot, 0, kSaveSlotCount - 1);
    if (m_saveSlot == safeSlot)
    {
        SetMessage(SaveSlotLabel() + L" 선택 중.");
        return;
    }

    m_saveSlot = safeSlot;
    std::wifstream check(ProgressPath(m_saveSlot));
    if (!check && m_saveSlot == 0)
    {
        check.clear();
        check.open(LegacyProgressPath());
    }
    const bool hasSave = static_cast<bool>(check);
    LoadProgress(m_saveSlot);
    UpdateViewMetrics();
    SetMessage(hasSave ? SaveSlotLabel() + L" 불러오기 완료." : SaveSlotLabel() + L"은 아직 비어 있어.");
}

void PawlineGameImpl::ResetProgressData()
{
    // 제출 전 테스트 중에도 안전하게 처음 상태로 되돌릴 수 있는 진행 초기화다.
    m_lumen = 0;
    m_unitUnlocked = {
        true, true, true, true, true,
        false, false, false, false, false,
        false, false, false, false};
    m_unitLevels = {
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1};
    m_stageCleared = {
        false, false, false, false, false,
        false, false, false, false, false};
    m_loadout = {PlayerUnit::Paw, PlayerUnit::Box, PlayerUnit::Spark, PlayerUnit::Dash, PlayerUnit::Bell};
    m_selectedStage = 0;
    m_selectedLoadoutSlot = 0;
    m_shopSelectedUnit = 0;
    m_resetConfirmTimer = 0.0f;
    SaveProgress();
    SetMessage(L"진행 데이터를 처음 상태로 돌렸어.");
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
    m_imageVfx.clear();
    m_floatTexts.clear();
    m_uiPulses.clear();
    m_telegraphs.clear();
    m_screenFlash = 0.0f;
    ResetCombatFeedbackState();
    m_directorPressure = 0.0f;
    m_paused = false;
    m_escapeMenuOpen = false;
    m_pauseBeforeEscape = false;
    m_gameOver = false;
    m_victory = false;
    m_showcaseMode = false;
    m_demoSpawnTimer = 0.0f;
    m_demoWalletTimer = 0.0f;
    m_resetConfirmTimer = 0.0f;
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
    m_imageVfx.clear();
    m_floatTexts.clear();
    m_uiPulses.clear();
    m_telegraphs.clear();
    m_screenFlash = 0.0f;
    ResetCombatFeedbackState();
    m_directorPressure = 0.0f;
    m_paused = false;
    m_escapeMenuOpen = false;
    m_pauseBeforeEscape = false;
    m_gameOver = false;
    m_victory = false;
    m_showcaseMode = false;
    m_demoSpawnTimer = 0.0f;
    m_demoWalletTimer = 0.0f;
    m_resetConfirmTimer = 0.0f;
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
    m_imageVfx.clear();
    m_floatTexts.clear();
    m_uiPulses.clear();
    m_telegraphs.clear();

    m_cardCooldowns.fill(0.0f);
    m_walletLevel = 1;
    m_energy = stage.startEnergy;
    const float playerScale = m_difficulty == Difficulty::Easy ? 1.16f : (m_difficulty == Difficulty::Hard ? 0.92f : 1.0f);
    const float enemyScale = m_difficulty == Difficulty::Easy ? 0.88f : (m_difficulty == Difficulty::Hard ? 1.18f : 1.0f);
    m_playerBaseHp = stage.playerHp * playerScale;
    m_enemyBaseHp = stage.enemyHp * enemyScale;
    m_playerBaseMaxHp = m_playerBaseHp;
    m_enemyBaseMaxHp = m_enemyBaseHp;
    m_stageTime = 0.0f;
    m_gameSpeed = m_defaultGameSpeed;
    m_enemyTimer = stage.enemyInterval * (m_difficulty == Difficulty::Easy ? 1.15f : (m_difficulty == Difficulty::Hard ? 0.86f : 1.0f));
    m_directorPressure = 0.0f;
    m_nextBossTime = stage.bossFirstTime;
    m_cannonCharge = 35.0f;
    m_cannonFlash = 0.0f;
    m_screenFlash = 0.0f;
    ResetCombatFeedbackState();
    m_walletPulseTimer = WalletPulseInterval();
    m_stageGimmickTimer = 7.0f + static_cast<float>(m_selectedStage % 3) * 1.8f;
    m_stageGimmickPulse = 0.0f;
    m_stageAmbientTimer = 0.0f;
    m_bossPatternTimer = std::max(5.0f, 8.4f - static_cast<float>(m_selectedStage) * 0.26f);
    m_bossFocusX = 0.0f;
    m_cameraTrauma = 0.0f;
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
    m_demoSpawnTimer = 0.70f;
    m_demoWalletTimer = 2.20f;
    SetMessage(stage.name + L": 유닛을 소환해서 적 기지를 밀어내자.");
}

void PawlineGameImpl::StartDemoRun()
{
    // 제출 영상용 자동 시연. 플레이어가 직접 조작하지 않아도 전투, 월렛, 캐논, 카메라가 보인다.
    m_selectedStage = HighestUnlockedStage();
    m_difficulty = Difficulty::Normal;
    ResetGame();
    m_showcaseMode = true;
    m_showcaseTimer = 0.0f;
    m_demoSpawnTimer = 0.35f;
    m_demoWalletTimer = 1.0f;
    SetMessage(L"시연 모드 시작. F1로 켜고 끌 수 있어.");
}

void PawlineGameImpl::Update(float dt)
{
    // UI time always moves, but combat time below is multiplied by m_gameSpeed.
    // This keeps menus/VFX alive even when the battle itself is paused.
    m_uiTime += dt;
    if (m_screen != m_observedScreen)
    {
        // 모든 씬 변경을 한곳에서 감지해 페이드 전환을 켠다.
        m_observedScreen = m_screen;
        m_sceneTransitionTimer = m_sceneTransitionMax;
    }
    if (m_sceneTransitionTimer > 0.0f)
    {
        m_sceneTransitionTimer = std::max(0.0f, m_sceneTransitionTimer - dt);
    }
    if (m_messageTimer > 0.0f)
    {
        const float fadeSpeed = Contains(MessageToastRect(), m_mouse) ? 5.6f : 1.0f;
        m_messageTimer = std::max(0.0f, m_messageTimer - dt * fadeSpeed);
    }
    if (m_autoSaveNoticeTimer > 0.0f)
    {
        m_autoSaveNoticeTimer = std::max(0.0f, m_autoSaveNoticeTimer - dt);
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
    if (m_resetConfirmTimer > 0.0f)
    {
        m_resetConfirmTimer = std::max(0.0f, m_resetConfirmTimer - dt);
    }
    if (m_stageGimmickPulse > 0.0f)
    {
        m_stageGimmickPulse -= dt;
    }
    if (m_bossBannerTimer > 0.0f)
    {
        m_bossBannerTimer -= dt;
    }
    if (m_bossWarningTimer > 0.0f)
    {
        m_bossWarningTimer -= dt;
    }
    if (m_cameraTrauma > 0.0f)
    {
        m_cameraTrauma = std::max(0.0f, m_cameraTrauma - dt * 1.35f);
    }
    UpdateCombatFeedbackTimers(dt);
    if (m_showcaseMode)
    {
        m_showcaseTimer += dt;
    }

    if (m_screen == GameScreen::Title || m_screen == GameScreen::Options || m_screen == GameScreen::Menu || m_screen == GameScreen::Codex || m_screen == GameScreen::Shop || m_screen == GameScreen::Briefing || m_screen == GameScreen::Result)
    {
        if (m_screen == GameScreen::Result)
        {
            UpdateCamera(dt);
        }
        UpdateTelegraphs(dt);
        UpdateParticles(dt);
        CleanupEntities();
        return;
    }

    if (m_paused || m_gameOver || m_victory)
    {
        UpdateTelegraphs(dt);
        UpdateParticles(dt);
        CleanupEntities();
        return;
    }

    // 모든 전투 시스템은 gameDt만 먹는다. 속도 조절과 히트스톱을 한곳에서 합성한다.
    const float gameDt = dt * m_gameSpeed * CombatTimeScale();
    m_stageTime += gameDt;
    UpdateCamera(gameDt);
    m_energy = std::min(MaxEnergy(), m_energy + EnergyRegen() * gameDt);
    const float cannonSynergy = HasLoadoutUnit(PlayerUnit::Solar) && HasLoadoutUnit(PlayerUnit::Bell) ? 1.12f : 1.0f;
    m_cannonCharge = std::min(100.0f, m_cannonCharge + (6.8f + static_cast<float>(m_walletLevel) * 1.1f) * cannonSynergy * gameDt);
    UpdateWalletPulse(gameDt);
    UpdateDemoMode(gameDt);
    UpdateStageGimmicks(gameDt);
    UpdateBossPatterns(gameDt);

    for (float& cooldown : m_cardCooldowns)
    {
        cooldown = std::max(0.0f, cooldown - gameDt);
    }

    UpdateEnemyDirector(gameDt);
    UpdateUnits(gameDt);
    UpdateProjectiles(gameDt);
    UpdateTelegraphs(gameDt);
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
        SetMessage(L"적 기지 파괴. LUMEN 획득.");
    }
    if (m_playerBaseHp <= 0.0f)
    {
        m_gameOver = true;
        m_screen = GameScreen::Result;
        m_playerBaseHp = 0.0f;
        m_resultScore = m_score;
        m_resultTime = m_stageTime;
        SetMessage(L"아군 기지가 무너졌어. R로 다시 도전.");
    }
}

void PawlineGameImpl::UpdateDemoMode(float dt)
{
    if (!m_showcaseMode || m_screen != GameScreen::Playing || m_gameOver || m_victory || m_paused || m_escapeMenuOpen)
    {
        return;
    }

    m_demoWalletTimer -= dt;
    if (m_demoWalletTimer <= 0.0f)
    {
        // 데모 모드에서는 경제 시스템이 보이도록 월렛 강화와 캐논 사용을 자동으로 시도한다.
        if (WalletUpgradeCost() > 0 && m_energy >= static_cast<float>(WalletUpgradeCost()))
        {
            TryUpgradeWallet();
        }
        if (m_cannonCharge >= 100.0f)
        {
            TryFireCannon();
        }
        m_demoWalletTimer = 4.8f;
    }

    m_demoSpawnTimer -= dt;
    if (m_demoSpawnTimer > 0.0f)
    {
        return;
    }

    const int start = static_cast<int>(std::floor(m_showcaseTimer * 0.67f)) % kLoadoutSize;
    for (int offset = 0; offset < kLoadoutSize; ++offset)
    {
        const int index = (start + offset) % kLoadoutSize;
        const PlayerUnit unit = m_loadout[index];
        if (m_cardCooldowns[index] <= 0.0f && IsUnitUnlocked(unit) && m_energy >= static_cast<float>(UnitEnergyCost(unit)))
        {
            TrySpawnPlayer(index);
            break;
        }
    }
    m_demoSpawnTimer = 1.15f;
}

void PawlineGameImpl::TriggerDebugClear()
{
    if (m_screen != GameScreen::Playing)
    {
        return;
    }

    // 테스트 모드용 즉시 클리어. 밸런스 확인과 제출 영상 체크 때만 쓰는 숨은 도구다.
    m_enemyBaseHp = 0.0f;
    AddBurst({kEnemyBaseX - 64.0f, kLaneY}, D2D1::ColorF(0xF6FF83), 54);
    AddCameraTrauma(0.70f);
    SetMessage(L"테스트 클리어 실행.");
}

void PawlineGameImpl::TriggerDebugUnlockAll()
{
    // 빌드 확인용으로 모든 유닛과 스테이지를 여는 기능이다. 진행 저장에도 반영된다.
    for (int i = 0; i < kRosterCount; ++i)
    {
        m_unitUnlocked[i] = true;
        m_unitLevels[i] = std::max(m_unitLevels[i], 3);
    }
    for (int i = 0; i < kStageCount; ++i)
    {
        m_stageCleared[i] = true;
    }
    SaveProgress();
    SetMessage(L"테스트 모드: 전체 잠금 해제.");
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

void PawlineGameImpl::SetViewTransform(float worldCameraX, bool includeCameraShake)
{
    Vec2 shake = {};
    if (includeCameraShake && m_cameraTrauma > 0.0f)
    {
        const float power = m_cameraTrauma * m_cameraTrauma;
        shake.x = std::sin((m_uiTime + m_stageTime) * 74.0f) * power * 9.0f;
        shake.y = std::cos((m_uiTime + m_stageTime) * 91.0f) * power * 5.0f;
    }

    const D2D1_MATRIX_3X2_F matrix =
        D2D1::Matrix3x2F::Translation(-worldCameraX + shake.x, shake.y) *
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
        if (m_showcaseMode && m_bossBannerTimer <= 0.0f)
        {
            const float wave = (std::sin(m_showcaseTimer * 0.34f) + 1.0f) * 0.5f;
            m_cameraTargetX = Lerp(0.0f, kCameraMaxX, wave);
            if (auto boss = FindBossUnit())
            {
                if (std::fmod(m_showcaseTimer, 18.0f) > 12.0f)
                {
                    m_cameraTargetX = std::max(0.0f, std::min(kCameraMaxX, boss->get().pos.x - kWidth * 0.58f));
                }
            }
        }
        if (m_bossBannerTimer > 0.0f)
        {
            m_cameraTargetX = m_bossFocusX;
        }
    }

    m_cameraTargetX = std::max(0.0f, std::min(kCameraMaxX, m_cameraTargetX));
    const float follow = 1.0f - std::pow(0.001f, std::min(dt, 0.05f));
    m_cameraX = Lerp(m_cameraX, m_cameraTargetX, follow);
}
