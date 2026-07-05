#include "PawlineGameImpl.h"

#include <mmsystem.h>

// Lifecycle, persistence, and high-level frame update.
PawlineGameImpl::~PawlineGameImpl()
{
    DiscardDeviceResources();
    UnregisterPrivateFonts();
    m_audio.Shutdown();
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
    m_units.reserve(160);
    m_projectiles.reserve(192);
    m_particles.reserve(640);
    m_rings.reserve(96);
    m_beams.reserve(48);
    m_sparkLines.reserve(160);
    m_imageVfx.reserve(160);
    m_floatTexts.reserve(96);
    m_uiPulses.reserve(48);
    m_telegraphs.reserve(48);
    m_drawOrder.reserve(160);
    m_audio.Initialize();
    StartBackgroundMusic();

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
        L"Space Defanse - Direct2D",
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
            if ((wParam & MK_LBUTTON) != 0 && m_activeSliderDrag != UiSliderDragTarget::None)
            {
                m_mouse = nextMouse;
                UpdateSliderDrag(nextMouse, false);
                return 0;
            }
            if ((wParam & MK_LBUTTON) == 0 && m_activeSliderDrag != UiSliderDragTarget::None)
            {
                EndSliderDrag();
            }

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
        {
            SetFocus(m_hwnd);
            SetCapture(m_hwnd);
            const Vec2 click = ClientToVirtual({static_cast<float>(GET_X_LPARAM(lParam)), static_cast<float>(GET_Y_LPARAM(lParam))});
            BeginSliderDrag(click);
            OnLeftClick(click);
            return 0;
        }
    case WM_LBUTTONUP:
        if (GetCapture() == m_hwnd)
        {
            ReleaseCapture();
        }
        EndSliderDrag();
        return 0;
    case WM_CAPTURECHANGED:
        if (reinterpret_cast<HWND>(lParam) != m_hwnd)
        {
            EndSliderDrag();
        }
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
    const bool hasGyeonggiFont = std::any_of(m_privateFontPaths.begin(), m_privateFontPaths.end(), [](const std::wstring& path) {
        return path.find(L"Gyeonggi") != std::wstring::npos;
    });
    const wchar_t* titleFont = hasGyeonggiFont ? L"GyeonggiTitleOTF" : L"Segoe UI";
    const wchar_t* bodyFont = hasGyeonggiFont ? L"GyeonggiBatangOTF" : L"Segoe UI";
    HRESULT hr = m_writeFactory->CreateTextFormat(
        titleFont, nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 34.0f, L"ko-kr", m_titleFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        titleFont, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"ko-kr", m_headerFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        bodyFont, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 16.5f, L"ko-kr", m_bodyFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        bodyFont, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 13.7f, L"ko-kr", m_smallFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        titleFont, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"ko-kr", m_buttonFormat.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_writeFactory->CreateTextFormat(
        titleFont, nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 19.5f, L"ko-kr", m_centerFormat.GetAddressOf());
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
    // 픽셀 글자는 직접 그리지만, 일반 UI 문장에는 패키지에 포함한 경기천년체를 사용한다.
    const std::array<std::wstring, 6> fontFiles = {
        L"assets\\fonts\\Galmuri11.ttf",
        L"assets\\fonts\\GyeonggiTitle_Bold.otf",
        L"assets\\fonts\\GyeonggiTitle_Medium.otf",
        L"assets\\fonts\\GyeonggiTitle_Light.otf",
        L"assets\\fonts\\GyeonggiBatang_Regular.otf",
        L"assets\\fonts\\GyeonggiBatang_Bold.otf",
    };

    m_privateFontPaths.clear();
    for (const std::wstring& relativePath : fontFiles)
    {
        const std::wstring fontPath = AssetPath(relativePath);
        if (GetFileAttributesW(fontPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            continue;
        }
        if (AddFontResourceExW(fontPath.c_str(), FR_PRIVATE, nullptr) > 0)
        {
            m_privateFontPaths.push_back(fontPath);
        }
    }
    m_privateFontLoaded = !m_privateFontPaths.empty();
}

void PawlineGameImpl::UnregisterPrivateFonts()
{
    if (!m_privateFontLoaded)
    {
        return;
    }
    for (const std::wstring& fontPath : m_privateFontPaths)
    {
        RemoveFontResourceExW(fontPath.c_str(), FR_PRIVATE, nullptr);
    }
    m_privateFontPaths.clear();
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

bool PawlineGameImpl::StartMusicNow(const std::wstring& absolutePath, bool loop, float fadeLevel)
{
    // 실제 스트림을 교체하는 낮은 단계 함수다. 호출자는 페이드 상태만 정리해 주면 된다.
    m_musicFadeLevel = std::clamp(fadeLevel, 0.0f, 1.0f);
    if (m_audio.PlayMusic(absolutePath, m_bgmVolume * m_musicFadeLevel, loop))
    {
        m_currentMusicPath = absolutePath;
        return true;
    }

    m_currentMusicPath.clear();
    return false;
}

void PawlineGameImpl::PlayMusicTrack(const std::wstring& relativePath, float fadeSeconds, bool loop)
{
    const std::wstring absolutePath = AssetPath(relativePath);
    if (absolutePath == m_currentMusicPath && m_pendingMusicPath.empty())
    {
        SyncMusicVolume();
        return;
    }

    const float safeFade = std::clamp(fadeSeconds, 0.05f, 2.5f);
    if (m_currentMusicPath.empty() || safeFade <= 0.06f)
    {
        if (!StartMusicNow(absolutePath, loop, 1.0f))
        {
            StartMusicNow(AssetPath(L"assets\\music\\outer_space_loop.mp3"), true, 1.0f);
        }
        m_pendingMusicPath.clear();
        m_musicFadingOut = false;
        return;
    }

    // 이미 다른 곡을 기다리는 중이면 마지막 요청만 남긴다.
    m_pendingMusicPath = absolutePath;
    m_pendingMusicLoop = loop;
    m_musicFadeStartLevel = m_musicFadeLevel;
    m_musicFadeDuration = safeFade;
    m_musicFadeTimer = 0.0f;
    m_musicFadingOut = true;
}

void PawlineGameImpl::StartBackgroundMusic()
{
    // 메뉴와 타이틀에서는 몽환적인 우주 루프를 계속 재생한다.
    StopDangerMusicLayer();
    PlayMusicTrack(L"assets\\music\\outer_space_loop.mp3", 0.65f, true);
}

std::wstring PawlineGameImpl::StageMusicPath(int stageIndex) const
{
    // 수성부터 태양까지 각 스테이지의 분위기를 나누는 전투용 루프 목록이다.
    constexpr std::array<const wchar_t*, kStageCount> kStageMusic = {
        L"assets\\music\\stage_mercury.wav",
        L"assets\\music\\stage_venus.wav",
        L"assets\\music\\stage_earth.wav",
        L"assets\\music\\stage_mars.wav",
        L"assets\\music\\stage_jupiter.wav",
        L"assets\\music\\stage_saturn.wav",
        L"assets\\music\\stage_uranus.wav",
        L"assets\\music\\stage_neptune.wav",
        L"assets\\music\\stage_pluto.wav",
        L"assets\\music\\stage_sun.wav",
    };

    const int index = std::clamp(stageIndex, 0, kStageCount - 1);
    return kStageMusic[static_cast<size_t>(index)];
}

void PawlineGameImpl::StartStageMusic()
{
    PlayMusicTrack(StageMusicPath(m_selectedStage), 0.70f, true);
    StartDangerMusicLayer();
}

std::wstring PawlineGameImpl::ResultMusicPath(bool victory) const
{
    return victory ? L"assets\\music\\result_victory.wav" : L"assets\\music\\result_defeat.wav";
}

void PawlineGameImpl::StartResultMusic(bool victory)
{
    StopDangerMusicLayer();
    PlayMusicTrack(ResultMusicPath(victory), 0.62f, true);
}

void PawlineGameImpl::PlayMusicStinger(const std::wstring& relativePath, float volumeScale)
{
    // 보스 등장처럼 음악적 의미가 있는 짧은 소리는 BGM 볼륨을 따라가게 한다.
    const float volume = std::clamp(m_bgmVolume * volumeScale, 0.0f, 1.0f);
    if (volume <= 0.001f)
    {
        return;
    }

    m_audio.SetVolume(volume);
    m_audio.PlayEffect(AssetPath(relativePath));
}

void PawlineGameImpl::StartDangerMusicLayer()
{
    const std::wstring layerPath = AssetPath(L"assets\\music\\layer_danger.wav");
    if (layerPath == m_currentMusicLayerPath)
    {
        return;
    }

    if (m_audio.PlayMusicLayer(layerPath, 0.0f, true))
    {
        m_currentMusicLayerPath = layerPath;
        m_musicLayerLevel = 0.0f;
    }
}

void PawlineGameImpl::StopDangerMusicLayer()
{
    m_currentMusicLayerPath.clear();
    m_musicLayerLevel = 0.0f;
    m_audio.SetMusicLayerVolume(0.0f);
    m_audio.StopMusicLayer();
}

float PawlineGameImpl::MusicDangerTarget() const
{
    if (m_screen != GameScreen::Playing || m_paused || m_gameOver || m_victory)
    {
        return 0.0f;
    }

    const float basePressure = m_playerBaseMaxHp > 1.0f ? 1.0f - Clamp01(m_playerBaseHp / m_playerBaseMaxHp) : 0.0f;
    float enemyProximity = 0.0f;
    float bossPressure = 0.0f;
    for (const Unit& unit : m_units)
    {
        if (!unit.alive || unit.team != Team::Enemy)
        {
            continue;
        }
        enemyProximity = std::max(enemyProximity, Clamp01((520.0f - (unit.pos.x - kPlayerBaseX)) / 520.0f));
        if (unit.boss)
        {
            bossPressure = std::max(bossPressure, 0.70f);
        }
    }

    const float director = Clamp01(m_directorPressure * 0.18f);
    return Clamp01(basePressure * 0.56f + enemyProximity * 0.44f + bossPressure + director * 0.22f);
}

void PawlineGameImpl::UpdateMusicSystem(float dt)
{
    if (m_musicFadingOut)
    {
        m_musicFadeTimer += dt;
        m_musicFadeLevel = Lerp(m_musicFadeStartLevel, 0.0f, Clamp01(m_musicFadeTimer / m_musicFadeDuration));
        m_audio.SetMusicVolume(m_bgmVolume * m_musicFadeLevel);

        if (m_musicFadeTimer >= m_musicFadeDuration)
        {
            const std::wstring nextPath = m_pendingMusicPath;
            const bool nextLoop = m_pendingMusicLoop;
            m_pendingMusicPath.clear();
            m_musicFadingOut = false;
            m_musicFadeTimer = 0.0f;

            if (!StartMusicNow(nextPath, nextLoop, 0.0f))
            {
                StartMusicNow(AssetPath(L"assets\\music\\outer_space_loop.mp3"), true, 0.0f);
            }
        }
    }
    else if (m_musicFadeLevel < 0.999f)
    {
        m_musicFadeTimer += dt;
        m_musicFadeLevel = Clamp01(m_musicFadeTimer / m_musicFadeDuration);
        m_audio.SetMusicVolume(m_bgmVolume * m_musicFadeLevel);
    }
    else
    {
        m_musicFadeLevel = 1.0f;
    }

    const float targetLayer = MusicDangerTarget() * 0.58f;
    const float follow = 1.0f - std::exp(-dt * (targetLayer > m_musicLayerLevel ? 2.8f : 4.2f));
    m_musicLayerLevel = Lerp(m_musicLayerLevel, targetLayer, follow);
    m_audio.SetMusicLayerVolume(m_bgmVolume * m_musicLayerLevel);
}

void PawlineGameImpl::SyncMusicVolume()
{
    // 옵션에서 BGM 볼륨을 바꾸면 이미 재생 중인 채널에도 즉시 반영한다.
    m_audio.SetMusicVolume(m_bgmVolume * m_musicFadeLevel);
    m_audio.SetMusicLayerVolume(m_bgmVolume * m_musicLayerLevel);
}

float PawlineGameImpl::VolumeForSfxKind(SfxKind kind) const
{
    // 메뉴 클릭음은 UI 볼륨, 전투에서 나는 나머지 소리는 효과음 볼륨을 따른다.
    return kind == SfxKind::Ui ? m_uiVolume : m_sfxVolume;
}

void PawlineGameImpl::PlaySfx(SfxKind kind, float minGapSeconds)
{
    std::wstring fileName;
    switch (kind)
    {
    case SfxKind::Spawn:
        fileName = L"events\\player_spawn.wav";
        break;
    case SfxKind::EnemySpawn:
        fileName = L"events\\enemy_spawn.wav";
        break;
    case SfxKind::Hit:
        fileName = L"events\\hit_light.wav";
        break;
    case SfxKind::HeavyHit:
        fileName = L"events\\hit_heavy.wav";
        break;
    case SfxKind::Shoot:
        fileName = L"events\\cannon_fire.wav";
        break;
    case SfxKind::Upgrade:
        fileName = L"events\\ui_confirm.wav";
        break;
    case SfxKind::Clear:
        fileName = L"events\\stage_clear.wav";
        break;
    case SfxKind::Ui:
        fileName = L"events\\ui_click.wav";
        break;
    case SfxKind::ProjectileImpact:
        fileName = L"events\\projectile_impact.wav";
        break;
    case SfxKind::BaseHit:
        fileName = L"events\\base_hit.wav";
        break;
    case SfxKind::Death:
        fileName = L"events\\unit_death.wav";
        break;
    case SfxKind::Boss:
        fileName = L"events\\boss_arrive.wav";
        break;
    case SfxKind::Stage:
        fileName = L"events\\scene_curtain.wav";
        break;
    case SfxKind::Wallet:
        fileName = L"events\\wallet_upgrade.wav";
        break;
    case SfxKind::UnitAttack:
    case SfxKind::Count:
        return;
    }

    PlaySfxFile(fileName, kind, minGapSeconds);
}

void PawlineGameImpl::PlaySfxAt(SfxKind kind, float worldX, float minGapSeconds, float volumeScale)
{
    std::wstring fileName;
    switch (kind)
    {
    case SfxKind::Spawn:
        fileName = L"events\\player_spawn.wav";
        break;
    case SfxKind::EnemySpawn:
        fileName = L"events\\enemy_spawn.wav";
        break;
    case SfxKind::Hit:
        fileName = L"events\\hit_light.wav";
        break;
    case SfxKind::HeavyHit:
        fileName = L"events\\hit_heavy.wav";
        break;
    case SfxKind::Shoot:
        fileName = L"events\\cannon_fire.wav";
        break;
    case SfxKind::Upgrade:
        fileName = L"events\\ui_confirm.wav";
        break;
    case SfxKind::Clear:
        fileName = L"events\\stage_clear.wav";
        break;
    case SfxKind::ProjectileImpact:
        fileName = L"events\\projectile_impact.wav";
        break;
    case SfxKind::BaseHit:
        fileName = L"events\\base_hit.wav";
        break;
    case SfxKind::Death:
        fileName = L"events\\unit_death.wav";
        break;
    case SfxKind::Boss:
        fileName = L"events\\boss_arrive.wav";
        break;
    case SfxKind::Stage:
        fileName = L"events\\scene_curtain.wav";
        break;
    case SfxKind::Wallet:
        fileName = L"events\\wallet_upgrade.wav";
        break;
    case SfxKind::Ui:
        fileName = L"events\\ui_click.wav";
        break;
    case SfxKind::UnitAttack:
    case SfxKind::Count:
        return;
    }

    PlaySfxFileAt(fileName, kind, worldX, minGapSeconds, volumeScale);
}

void PawlineGameImpl::PlaySfxFile(const std::wstring& relativeFileName, SfxKind throttleKind, float minGapSeconds)
{
    const float volume = VolumeForSfxKind(throttleKind);
    if (volume <= 0.001f)
    {
        return;
    }

    const int index = std::clamp(static_cast<int>(throttleKind), 0, static_cast<int>(m_sfxLastTimes.size()) - 1);
    auto& lanes = m_sfxLastTimes[static_cast<size_t>(index)];
    auto oldestLane = std::min_element(lanes.begin(), lanes.end());
    if (oldestLane == lanes.end() || m_uiTime - *oldestLane < minGapSeconds)
    {
        return;
    }
    *oldestLane = m_uiTime;

    m_audio.SetVolume(volume);
    m_audio.PlayEffect(AssetPath(L"assets\\sfx\\" + relativeFileName));
}

void PawlineGameImpl::PlaySfxFileAt(const std::wstring& relativeFileName, SfxKind throttleKind, float worldX, float minGapSeconds, float volumeScale)
{
    const float volume = VolumeForSfxKind(throttleKind);
    if (volume <= 0.001f)
    {
        return;
    }

    const int index = std::clamp(static_cast<int>(throttleKind), 0, static_cast<int>(m_sfxLastTimes.size()) - 1);
    auto& lanes = m_sfxLastTimes[static_cast<size_t>(index)];
    auto oldestLane = std::min_element(lanes.begin(), lanes.end());
    if (oldestLane == lanes.end() || m_uiTime - *oldestLane < minGapSeconds)
    {
        return;
    }
    *oldestLane = m_uiTime;

    m_audio.SetVolume(volume);
    m_audio.PlayEffectAt(AssetPath(L"assets\\sfx\\" + relativeFileName), worldX, volumeScale);
}

std::wstring PawlineGameImpl::AttackSfxPath(const Unit& attacker) const
{
    if (attacker.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(attacker.kind))
        {
        case PlayerUnit::Paw:
            return L"kenney\\player_paw.wav";
        case PlayerUnit::Box:
            return L"kenney\\player_box.wav";
        case PlayerUnit::Spark:
            return L"kenney\\player_spark.wav";
        case PlayerUnit::Dash:
            return L"kenney\\player_dash.wav";
        case PlayerUnit::Bell:
            return L"kenney\\player_bell.wav";
        case PlayerUnit::Titan:
            return L"kenney\\player_titan.wav";
        case PlayerUnit::Frost:
            return L"kenney\\player_frost.wav";
        case PlayerUnit::Comet:
            return L"kenney\\player_comet_short.wav";
        case PlayerUnit::Orbit:
            return L"kenney\\player_orbit.wav";
        case PlayerUnit::Solar:
            return L"kenney\\player_solar.wav";
        case PlayerUnit::Mint:
            return L"kenney\\player_mint.wav";
        case PlayerUnit::Drill:
            return L"kenney\\player_drill.wav";
        case PlayerUnit::Prism:
            return L"kenney\\player_prism.wav";
        case PlayerUnit::Nebula:
            return L"kenney\\player_nebula.wav";
        }
    }

    switch (static_cast<EnemyUnit>(attacker.kind))
    {
    case EnemyUnit::Dust:
        return L"kenney\\enemy_dust.wav";
    case EnemyUnit::Brute:
        return L"kenney\\enemy_brute.wav";
    case EnemyUnit::Skitter:
        return L"kenney\\enemy_skitter.wav";
    case EnemyUnit::Sulfur:
        return L"kenney\\enemy_sulfur.wav";
    case EnemyUnit::Moss:
        return L"kenney\\enemy_moss.wav";
    case EnemyUnit::Rust:
        return L"kenney\\enemy_rust.wav";
    case EnemyUnit::Storm:
        return L"kenney\\enemy_storm.wav";
    case EnemyUnit::Ring:
        return L"kenney\\enemy_ring.wav";
    case EnemyUnit::Frost:
        return L"kenney\\enemy_frost.wav";
    case EnemyUnit::Tide:
        return L"kenney\\enemy_tide.wav";
    case EnemyUnit::Void:
        return L"kenney\\enemy_void.wav";
    case EnemyUnit::Flare:
        return L"kenney\\enemy_flare.wav";
    case EnemyUnit::Spore:
        return L"kenney\\enemy_spore_short.wav";
    case EnemyUnit::Quake:
        return L"kenney\\enemy_quake.wav";
    case EnemyUnit::Mirror:
        return L"kenney\\enemy_mirror.wav";
    case EnemyUnit::Comet:
        return L"kenney\\enemy_comet_short.wav";
    case EnemyUnit::Boss:
        return L"kenney\\enemy_boss.wav";
    }

    return L"shoot.wav";
}

float PawlineGameImpl::AttackSfxVolumeScale(const Unit& attacker) const
{
    // 파일별 음량 보정 이후에도 캐릭터 콘셉트에 맞게 무기음의 존재감을 한 번 더 맞춘다.
    if (attacker.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(attacker.kind))
        {
        case PlayerUnit::Paw:
            return 1.10f;
        case PlayerUnit::Box:
            return 0.76f;
        case PlayerUnit::Spark:
            return 0.74f;
        case PlayerUnit::Dash:
            return 0.96f;
        case PlayerUnit::Bell:
            return 0.68f;
        case PlayerUnit::Titan:
            return 0.92f;
        case PlayerUnit::Frost:
            return 0.70f;
        case PlayerUnit::Comet:
            return 0.86f;
        case PlayerUnit::Orbit:
            return 1.02f;
        case PlayerUnit::Solar:
            return 0.88f;
        case PlayerUnit::Mint:
            return 0.82f;
        case PlayerUnit::Drill:
            return 1.04f;
        case PlayerUnit::Prism:
            return 1.06f;
        case PlayerUnit::Nebula:
            return 0.92f;
        }
    }

    switch (static_cast<EnemyUnit>(attacker.kind))
    {
    case EnemyUnit::Dust:
        return 0.94f;
    case EnemyUnit::Brute:
        return 0.96f;
    case EnemyUnit::Skitter:
        return 0.68f;
    case EnemyUnit::Sulfur:
        return 0.58f;
    case EnemyUnit::Moss:
        return 0.98f;
    case EnemyUnit::Rust:
        return 0.98f;
    case EnemyUnit::Storm:
        return 0.72f;
    case EnemyUnit::Ring:
        return 0.92f;
    case EnemyUnit::Frost:
        return 0.68f;
    case EnemyUnit::Tide:
        return 0.92f;
    case EnemyUnit::Void:
        return 0.90f;
    case EnemyUnit::Flare:
        return 0.78f;
    case EnemyUnit::Spore:
        return 0.86f;
    case EnemyUnit::Quake:
        return 0.82f;
    case EnemyUnit::Mirror:
        return 0.92f;
    case EnemyUnit::Comet:
        return 0.84f;
    case EnemyUnit::Boss:
        return 0.88f;
    }

    return 1.0f;
}

void PawlineGameImpl::PlayAttackSfxAt(const Unit& attacker, float minGapSeconds)
{
    if (m_sfxVolume <= 0.001f)
    {
        return;
    }

    // 공격음은 SfxKind 하나로 묶지 않고 유닛 타입별로 기록한다.
    // 그래야 서로 다른 캐릭터가 동시에 싸워도 각자의 무기 소리가 살아난다.
    std::array<float, kAttackSfxThrottleVoices>* lanes = nullptr;
    if (attacker.team == Team::Player)
    {
        const int index = std::clamp(attacker.kind, 0, static_cast<int>(kRosterCount) - 1);
        lanes = &m_playerAttackSfxLastTimes[static_cast<size_t>(index)];
    }
    else
    {
        const int index = std::clamp(attacker.kind, 0, static_cast<int>(kEnemyCount) - 1);
        lanes = &m_enemyAttackSfxLastTimes[static_cast<size_t>(index)];
    }

    if (!lanes)
    {
        return;
    }

    auto oldestLane = std::min_element(lanes->begin(), lanes->end());
    if (oldestLane == lanes->end() || m_uiTime - *oldestLane < minGapSeconds)
    {
        return;
    }
    *oldestLane = m_uiTime;

    float volumeScale = attacker.ranged ? 0.92f : 0.88f;
    if (attacker.team == Team::Enemy)
    {
        volumeScale += 0.02f;
    }
    if (attacker.elite)
    {
        volumeScale += 0.06f;
    }
    if (attacker.boss)
    {
        volumeScale = 1.04f;
    }
    volumeScale *= AttackSfxVolumeScale(attacker);

    m_audio.SetVolume(m_sfxVolume);
    m_audio.PlayEffectAt(AssetPath(L"assets\\sfx\\" + AttackSfxPath(attacker)), attacker.pos.x, volumeScale);
}

void PawlineGameImpl::AdjustSfxVolume(float delta)
{
    m_sfxVolume = std::clamp(m_sfxVolume + delta, 0.0f, 1.0f);
    m_soundEnabled = m_sfxVolume > 0.001f || m_uiVolume > 0.001f;
    m_audio.SetVolume(m_sfxVolume);
    PlaySfx(SfxKind::Ui, 0.02f);
    SaveProgress();
}

void PawlineGameImpl::AdjustUiVolume(float delta)
{
    m_uiVolume = std::clamp(m_uiVolume + delta, 0.0f, 1.0f);
    m_soundEnabled = m_sfxVolume > 0.001f || m_uiVolume > 0.001f;
    PlaySfx(SfxKind::Ui, 0.02f);
    SaveProgress();
}

void PawlineGameImpl::AdjustBgmVolume(float delta)
{
    m_bgmVolume = std::clamp(m_bgmVolume + delta, 0.0f, 1.0f);
    SyncMusicVolume();
    PlaySfx(SfxKind::Ui, 0.02f);
    SaveProgress();
}

void PawlineGameImpl::ResetAudioVolumes()
{
    // 제출 시 기본값은 효과음이 또렷하고, BGM은 뒤에서 깔리는 정도로 맞춘다.
    m_sfxVolume = 0.86f;
    m_uiVolume = 0.82f;
    m_bgmVolume = 0.32f;
    m_soundEnabled = true;
    m_audio.SetVolume(m_sfxVolume);
    SyncMusicVolume();
    PlaySfx(SfxKind::Ui, 0.02f);
    SaveProgress();
    SetMessage(L"오디오 볼륨을 기본값으로 돌렸어.");
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

    // VFX 시트는 게임플레이 코드가 ImageVfxKind만 요청하면 렌더러가 고르게 쓰는 공용 리소스다.
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\combat_vfx_atlas.png"), m_vfxAtlas.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\slash_effect_sheet.png"), m_slashEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\slash_gameboy_effect_sheet.png"), m_enemySlashEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\heal_effect_sheet.png"), m_healEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\heal_gameboy_effect_sheet.png"), m_healSoftEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\fire_explosion_sheet.png"), m_fireEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\ice_burst_sheet.png"), m_iceEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\thunder_strike_sheet.png"), m_thunderEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\water_impact_sheet.png"), m_waterEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\dark_impact_sheet.png"), m_darkEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\acid_impact_sheet.png"), m_acidEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\earth_impact_sheet.png"), m_earthEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\smoke_puff_sheet.png"), m_smokeEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\holy_flash_sheet.png"), m_holyEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\wind_breath_sheet.png"), m_windEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\wind_hit_sheet.png"), m_windHitEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\wood_hit_sheet.png"), m_woodEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\hit_flash_sheet.png"), m_hitFlashEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\smear_horizontal_sheet.png"), m_smearEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\thrust_sheet.png"), m_thrustEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\explosion_large_sheet.png"), m_explosionEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\fire_breath_sheet.png"), m_fireBreathEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\magic_mirror_sheet.png"), m_magicMirrorEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\energy_impact_sheet.png"), m_energyImpactEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\crystal_sheet.png"), m_crystalEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\air_burst_sheet.png"), m_airBurstEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\thunder_splash_sheet.png"), m_thunderSplashEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\water_ball_impact_sheet.png"), m_waterBallImpactEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\vfx\\smoke_dust_sheet.png"), m_smokeDustEffectSheet.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\backgrounds\\deep_space_hudf.jpg"), m_deepSpaceBitmap.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\sprites\\kenney_toon_units\\player_unit_atlas.png"), m_playerUnitAtlas.ReleaseAndGetAddressOf());
    LoadBitmapFromFile(AssetPath(L"assets\\sprites\\kenney_toon_units\\enemy_unit_atlas.png"), m_enemyUnitAtlas.ReleaseAndGetAddressOf());
    const std::array<const wchar_t*, kRosterCount> playerWeapons = {
        L"player_00_blue_saber.png", L"player_01_guard_shield.png", L"player_02_spark_staff.png", L"player_03_dash_dagger.png", L"player_04_bell_scepter.png",
        L"player_05_titan_hammer.png", L"player_06_frost_lance.png", L"player_07_comet_lance.png", L"player_08_orbit_focus.png", L"player_09_solar_blade.png",
        L"player_10_mint_staff.png", L"player_11_drill_bit.png", L"player_12_prism_focus.png", L"player_13_nebula_cannon.png"};
    for (int i = 0; i < kRosterCount; ++i)
    {
        LoadBitmapFromFile(AssetPath(L"assets\\weapons\\external_roles\\" + std::wstring(playerWeapons[static_cast<size_t>(i)])),
                           m_playerWeaponBitmaps[static_cast<size_t>(i)].ReleaseAndGetAddressOf());
    }

    const std::array<const wchar_t*, kEnemyCount> enemyWeapons = {
        L"enemy_00_dust_claw.png", L"enemy_01_brute_axe.png", L"enemy_02_skitter_dagger.png", L"enemy_03_sulfur_spitter.png", L"enemy_04_moss_club.png",
        L"enemy_05_rust_hammer.png", L"enemy_06_storm_shield.png", L"enemy_07_ring_focus.png", L"enemy_08_frost_dagger.png", L"enemy_09_tide_cannon.png",
        L"enemy_10_void_axe.png", L"enemy_11_flare_lance.png", L"enemy_12_spore_staff.png", L"enemy_13_quake_hammer.png", L"enemy_14_mirror_focus.png",
        L"enemy_15_comet_lance.png", L"enemy_16_boss_solar_axe.png"};
    for (int i = 0; i < kEnemyCount; ++i)
    {
        LoadBitmapFromFile(AssetPath(L"assets\\weapons\\external_roles\\" + std::wstring(enemyWeapons[static_cast<size_t>(i)])),
                           m_enemyWeaponBitmaps[static_cast<size_t>(i)].ReleaseAndGetAddressOf());
    }

    LoadBitmapFromFile(AssetPath(L"assets\\ui\\pawline_ui_atlas.png"), m_uiAtlas.ReleaseAndGetAddressOf());

    m_bitmapAssetsLoaded = true;
}

void PawlineGameImpl::DiscardBitmapAssets()
{
    m_vfxAtlas.Reset();
    m_slashEffectSheet.Reset();
    m_enemySlashEffectSheet.Reset();
    m_healEffectSheet.Reset();
    m_healSoftEffectSheet.Reset();
    m_fireEffectSheet.Reset();
    m_iceEffectSheet.Reset();
    m_thunderEffectSheet.Reset();
    m_waterEffectSheet.Reset();
    m_darkEffectSheet.Reset();
    m_acidEffectSheet.Reset();
    m_earthEffectSheet.Reset();
    m_smokeEffectSheet.Reset();
    m_holyEffectSheet.Reset();
    m_windEffectSheet.Reset();
    m_windHitEffectSheet.Reset();
    m_woodEffectSheet.Reset();
    m_hitFlashEffectSheet.Reset();
    m_smearEffectSheet.Reset();
    m_thrustEffectSheet.Reset();
    m_explosionEffectSheet.Reset();
    m_fireBreathEffectSheet.Reset();
    m_magicMirrorEffectSheet.Reset();
    m_energyImpactEffectSheet.Reset();
    m_crystalEffectSheet.Reset();
    m_airBurstEffectSheet.Reset();
    m_thunderSplashEffectSheet.Reset();
    m_waterBallImpactEffectSheet.Reset();
    m_smokeDustEffectSheet.Reset();
    m_deepSpaceBitmap.Reset();
    m_playerUnitAtlas.Reset();
    m_enemyUnitAtlas.Reset();
    for (auto& bitmap : m_playerWeaponBitmaps)
    {
        bitmap.Reset();
    }
    for (auto& bitmap : m_enemyWeaponBitmaps)
    {
        bitmap.Reset();
    }
    m_uiAtlas.Reset();
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
    stats.name = UnitDisplayName(unit);
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
    SaveProgress();
}

void PawlineGameImpl::GrantStageReward()
{
    const bool firstClear = !m_stageCleared[m_selectedStage];
    m_stageCleared[m_selectedStage] = true;
    m_lastReward = StageClearReward(firstClear);
    m_lumen += m_lastReward;
    AddFloatText({640.0f, 154.0f}, L"+" + ToWideInt(m_lastReward) + L" LUMEN", D2D1::ColorF(0xF6FF83), 1.5f);
    AddRing({640.0f, kLaneY}, 260.0f, 0.65f, D2D1::ColorF(0xF6FF83, 0.48f), 4.0f);
    PlaySfx(SfxKind::Clear, 0.80f);
    SaveProgress();
}

bool PawlineGameImpl::HasAnyProgressFile() const
{
    const auto hasGameplayProgress = [](const std::wstring& path) {
        std::wifstream file(path);
        if (!file)
        {
            return false;
        }

        std::wstring header;
        file >> header;
        int lumen = 0;
        if (header == L"PAWLINE_PROGRESS_V2")
        {
            file >> lumen;
        }
        else
        {
            std::wistringstream legacy(header);
            legacy >> lumen;
        }
        if (lumen > 0)
        {
            return true;
        }

        for (int i = 0; i < kRosterCount; ++i)
        {
            int unlocked = 0;
            file >> unlocked;
            if (file && i >= 5 && unlocked != 0)
            {
                return true;
            }
        }
        for (int i = 0; i < kRosterCount; ++i)
        {
            int level = 1;
            file >> level;
            if (file && level > 1)
            {
                return true;
            }
        }
        for (int i = 0; i < kStageCount; ++i)
        {
            int cleared = 0;
            file >> cleared;
            if (file && cleared != 0)
            {
                return true;
            }
        }
        return false;
    };

    // 첫 실행 연출은 "저장 파일이 전혀 없는 상태"에서만 자동 재생한다.
    // 실행 파일 옆의 현재 슬롯 저장 파일과 예전 단일 저장 파일을 모두 확인한다.
    if (hasGameplayProgress(LegacyProgressPath()))
    {
        return true;
    }

    for (int slot = 0; slot < kSaveSlotCount; ++slot)
    {
        if (hasGameplayProgress(ProgressPath(slot)))
        {
            return true;
        }
    }
    return false;
}

void PawlineGameImpl::BeginStoryCrawl(bool autoContinueToMenu, GameScreen returnScreen, bool returnToEscapeMenu)
{
    // 프롤로그 크롤은 타이틀과 ESC 메뉴에서 다시 볼 수 있다.
    // ESC에서 열면 끝난 뒤 원래 전투/메뉴 상태로 돌아가도록 복귀 지점을 저장한다.
    m_screen = GameScreen::StoryIntro;
    m_storyTimer = 0.0f;
    m_storyAutoContinueToMenu = autoContinueToMenu;
    m_storyReturnScreen = autoContinueToMenu ? GameScreen::Menu : returnScreen;
    m_storyReturnToEscapeMenu = returnToEscapeMenu;
    m_introViewedThisSession = true;
    m_escapeMenuOpen = false;
    m_paused = false;
    m_sceneTransitionTimer = m_sceneTransitionMax;
    SetMessage(autoContinueToMenu ? L"첫 출격 전 기록을 재생해." : L"개전 기록을 다시 읽어.");
}

void PawlineGameImpl::FinishStoryCrawl()
{
    m_storyTimer = 0.0f;
    m_sceneTransitionTimer = m_sceneTransitionMax;
    if (m_storyAutoContinueToMenu)
    {
        m_storyAutoContinueToMenu = false;
        ResetToMenu();
        return;
    }

    m_screen = m_storyReturnScreen;
    if (m_storyReturnToEscapeMenu)
    {
        m_escapeMenuOpen = true;
        if (m_screen == GameScreen::Playing)
        {
            m_paused = true;
        }
    }
    else if (m_screen == GameScreen::Playing)
    {
        m_paused = m_pauseBeforeEscape;
    }
    m_storyReturnScreen = GameScreen::Title;
    m_storyReturnToEscapeMenu = false;
}

void PawlineGameImpl::BeginEndingScene()
{
    // 태양을 처음 클리어했을 때 짧은 엔딩을 먼저 보여주고, 이후 결과 화면으로 돌아간다.
    m_screen = GameScreen::Ending;
    m_endingTimer = 0.0f;
    m_endingUnlocked = true;
    m_escapeMenuOpen = false;
    m_paused = false;
    m_sceneTransitionTimer = m_sceneTransitionMax;
    PlaySfx(SfxKind::Clear, 0.50f);
}

void PawlineGameImpl::FinishEndingScene()
{
    m_endingTimer = 0.0f;
    m_screen = GameScreen::Result;
    m_sceneTransitionTimer = m_sceneTransitionMax;
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

            float savedSfxVolume = m_sfxVolume;
            if (file >> savedSfxVolume)
            {
                m_sfxVolume = std::clamp(savedSfxVolume, 0.0f, 1.0f);
                m_audio.SetVolume(m_sfxVolume);
                float savedBgmVolume = m_bgmVolume;
                if (file >> savedBgmVolume)
                {
                    m_bgmVolume = std::clamp(savedBgmVolume, 0.0f, 1.0f);
                    float savedUiVolume = m_uiVolume;
                    if (file >> savedUiVolume)
                    {
                        m_uiVolume = std::clamp(savedUiVolume, 0.0f, 1.0f);
                        int savedDifficulty = static_cast<int>(m_difficulty);
                        if (file >> savedDifficulty)
                        {
                            m_difficulty = static_cast<Difficulty>(std::clamp(savedDifficulty, 0, 2));
                        }
                        else
                        {
                            file.clear();
                        }
                    }
                    else
                    {
                        file.clear();
                    }
                }
                else
                {
                    file.clear();
                }
                m_soundEnabled = m_sfxVolume > 0.001f || m_uiVolume > 0.001f;
                SyncMusicVolume();
            }
            else
            {
                file.clear();
            }
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
         << (m_hitShakeEnabled ? 1 : 0) << L" " << (m_reduceFlashes ? 1 : 0) << L" "
         << m_sfxVolume << L" " << m_bgmVolume << L" " << m_uiVolume << L" "
         << static_cast<int>(m_difficulty) << L"\n";
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
    m_resetConfirmTimer = 0.0f;
    m_deleteConfirmTimer = 0.0f;
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

void PawlineGameImpl::ResetProgressMemory()
{
    // 저장 파일을 쓰지 않고 메모리 안의 진행 상태만 기본값으로 되돌린다.
    // 슬롯 삭제와 전체 초기화가 같은 기본 상태를 공유하도록 분리해 둔다.
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
    m_deleteConfirmTimer = 0.0f;
    m_introViewedThisSession = false;
    m_endingUnlocked = false;
}

void PawlineGameImpl::DeleteSelectedSaveSlot()
{
    // 현재 선택된 슬롯 파일만 삭제한다. 다른 슬롯은 건드리지 않는다.
    const std::wstring path = ProgressPath(m_saveSlot);
    const bool deleted = DeleteFileW(path.c_str()) != 0;
    if (m_saveSlot == 0)
    {
        DeleteFileW(LegacyProgressPath().c_str());
    }

    ResetProgressMemory();
    UpdateViewMetrics();
    m_autoSaveNotice = L"슬롯 삭제됨  " + SaveSlotLabel();
    m_autoSaveNoticeTimer = 1.85f;
    SetMessage(deleted ? SaveSlotLabel() + L" 저장 데이터를 삭제했어." : SaveSlotLabel() + L"에 삭제할 저장 파일이 없어.");
}

void PawlineGameImpl::ResetProgressData()
{
    // 제출 전 테스트 중에도 안전하게 처음 상태로 되돌릴 수 있는 전체 진행 초기화다.
    ResetProgressMemory();
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
    m_deleteConfirmTimer = 0.0f;
    m_message.clear();
    m_messageTimer = 0.0f;
}

void PawlineGameImpl::ResetToMenu()
{
    m_screen = GameScreen::Menu;
    StartBackgroundMusic();
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
    m_deleteConfirmTimer = 0.0f;
    m_message.clear();
    m_messageTimer = 0.0f;
}

void PawlineGameImpl::ResetGame()
{
    const StageDefinition stage = CurrentStage();
    StartStageMusic();
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
    m_audio.SetListener(m_cameraX + kWidth * 0.5f, kWidth * 1.10f);
    m_audio.Update();
    UpdateMusicSystem(dt);
    if (m_screen != m_observedScreen)
    {
        // 모든 씬 변경을 한곳에서 감지해 페이드 전환을 켠다.
        m_observedScreen = m_screen;
        m_sceneTransitionTimer = m_sceneTransitionMax;
        PlaySfx(SfxKind::Stage, 0.10f);
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
    if (m_deleteConfirmTimer > 0.0f)
    {
        m_deleteConfirmTimer = std::max(0.0f, m_deleteConfirmTimer - dt);
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

    if (m_screen == GameScreen::StoryIntro)
    {
        m_storyTimer += dt;
        if (m_storyAutoContinueToMenu && m_storyTimer > 22.0f)
        {
            FinishStoryCrawl();
        }
    }
    if (m_screen == GameScreen::Ending)
    {
        m_endingTimer += dt;
        if (m_endingTimer > 19.0f)
        {
            FinishEndingScene();
        }
    }

    if (m_screen == GameScreen::Title || m_screen == GameScreen::StoryIntro || m_screen == GameScreen::Ending || m_screen == GameScreen::Options || m_screen == GameScreen::Menu || m_screen == GameScreen::Archive || m_screen == GameScreen::Shop || m_screen == GameScreen::Briefing || m_screen == GameScreen::Result)
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
        const bool finalClear = m_selectedStage == kStageCount - 1;
        m_victory = true;
        m_screen = GameScreen::Result;
        StartResultMusic(true);
        m_enemyBaseHp = 0.0f;
        m_resultScore = m_score + static_cast<int>(std::max(0.0f, m_playerBaseHp)) + m_walletLevel * 300;
        m_resultTime = m_stageTime;
        GrantStageReward();
        if (finalClear && !m_endingUnlocked)
        {
            BeginEndingScene();
        }
        SetMessage(L"적 기지 파괴. LUMEN 획득.");
    }
    if (m_playerBaseHp <= 0.0f)
    {
        m_gameOver = true;
        m_screen = GameScreen::Result;
        StartResultMusic(false);
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
