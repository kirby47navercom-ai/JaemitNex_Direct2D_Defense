#include "PawlineGameImpl.h"

// Mouse/keyboard commands and menu/shop state transitions.
void PawlineGameImpl::SetMessage(const std::wstring& message)
{
    m_message = message;
    m_messageTimer = 3.6f;
}

void PawlineGameImpl::OpenEscapeMenu()
{
    if (m_escapeMenuOpen)
    {
        return;
    }

    m_pauseBeforeEscape = m_paused;
    m_escapeMenuOpen = true;
    if (m_screen == GameScreen::Playing)
    {
        m_paused = true;
    }
}

void PawlineGameImpl::CloseEscapeMenu()
{
    if (!m_escapeMenuOpen)
    {
        return;
    }

    m_escapeMenuOpen = false;
    if (m_screen == GameScreen::Playing)
    {
        m_paused = m_pauseBeforeEscape;
    }
}

void PawlineGameImpl::AdjustEscapeMenuSpeed(float delta)
{
    m_defaultGameSpeed = std::max(0.5f, std::min(3.0f, m_defaultGameSpeed + delta));
    if (m_screen == GameScreen::Playing)
    {
        m_gameSpeed = std::max(0.5f, std::min(3.0f, m_gameSpeed + delta));
    }
}

void PawlineGameImpl::OnLeftClick(Vec2 pos)
{
    // Every screen owns its click handling. The shared rectangle helpers live in
    // PawlineGameImplLayout.cpp, so render and input stay pixel-aligned.
    AddUiPulse(pos, D2D1::ColorF(0x65B8FF, 0.72f));
    if (m_escapeMenuOpen)
    {
        OnEscapeMenuClick(pos);
        return;
    }

    if (m_screen == GameScreen::Title)
    {
        if (Contains(TitleStartButtonRect(), pos))
        {
            ResetToMenu();
        }
        else if (Contains(TitleOptionsButtonRect(), pos))
        {
            m_screen = GameScreen::Options;
        }
        else if (Contains(TitleQuitButtonRect(), pos))
        {
            PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
        }
        return;
    }
    if (m_screen == GameScreen::Options)
    {
        OnOptionsClick(pos);
        return;
    }
    if (m_screen == GameScreen::Menu)
    {
        OnMenuClick(pos);
        return;
    }
    if (m_screen == GameScreen::Briefing)
    {
        OnBriefingClick(pos);
        return;
    }
    if (m_screen == GameScreen::Shop)
    {
        OnShopClick(pos);
        return;
    }
    if (m_screen == GameScreen::Result)
    {
        OnResultClick(pos);
        return;
    }

    const D2D1_RECT_F cameraRail = D2D1::RectF(40.0f, 80.0f, 1240.0f, 106.0f);
    if (Contains(cameraRail, pos))
    {
        // 상단 미니맵 레일을 클릭하면 해당 월드 위치로 카메라를 이동한다.
        const float pct = Clamp01((pos.x - cameraRail.left) / (cameraRail.right - cameraRail.left));
        m_cameraTargetX = std::max(0.0f, std::min(kCameraMaxX, pct * kWorldWidth - kWidth * 0.5f));
        m_cameraX = Lerp(m_cameraX, m_cameraTargetX, 0.45f);
        SetMessage(L"Camera jumped.");
        return;
    }

    for (int i = 0; i < kLoadoutSize; ++i)
    {
        if (Contains(CardRect(i), pos))
        {
            TrySpawnPlayer(i);
            return;
        }
    }

    if (Contains(WalletButtonRect(), pos))
    {
        TryUpgradeWallet();
        return;
    }
    if (Contains(CannonButtonRect(), pos))
    {
        TryFireCannon();
        return;
    }
    if (Contains(SpeedDownButtonRect(), pos))
    {
        DecreaseGameSpeed();
        return;
    }
    if (Contains(SpeedUpButtonRect(), pos))
    {
        IncreaseGameSpeed();
        return;
    }
    if (Contains(PauseButtonRect(), pos))
    {
        m_paused = !m_paused;
        SetMessage(m_paused ? L"Paused." : L"Resumed.");
        return;
    }
    if (Contains(RestartButtonRect(), pos))
    {
        ResetGame();
        return;
    }
}

void PawlineGameImpl::OnMenuClick(Vec2 pos)
{
    for (int i = 0; i < kStageCount; ++i)
    {
        if (Contains(MenuStageRect(i), pos))
        {
            m_selectedStage = i;
            return;
        }
    }

    for (int i = 0; i < kLoadoutSize; ++i)
    {
        if (Contains(MenuLoadoutSlotRect(i), pos))
        {
            m_selectedLoadoutSlot = i;
            return;
        }
    }

    for (int i = 0; i < kRosterCount; ++i)
    {
        if (Contains(RosterCardRect(i), pos))
        {
            SetLoadoutUnit(static_cast<PlayerUnit>(i));
            return;
        }
    }

    if (Contains(StartGameButtonRect(), pos))
    {
        m_screen = GameScreen::Briefing;
        SetMessage(L"Check your deck before launch.");
    }
    if (Contains(MenuShopButtonRect(), pos))
    {
        m_screen = GameScreen::Shop;
    }
}

void PawlineGameImpl::OnBriefingClick(Vec2 pos)
{
    if (Contains(BriefingStartButtonRect(), pos))
    {
        ResetGame();
        return;
    }
    if (Contains(BriefingBackButtonRect(), pos))
    {
        ResetToMenu();
        return;
    }
    if (Contains(BriefingShopButtonRect(), pos))
    {
        m_screen = GameScreen::Shop;
        return;
    }

    for (int i = 0; i < kLoadoutSize; ++i)
    {
        const D2D1_RECT_F rect = D2D1::RectF(604.0f + static_cast<float>(i) * 112.0f, 260.0f, 700.0f + static_cast<float>(i) * 112.0f, 390.0f);
        if (Contains(rect, pos))
        {
            m_selectedLoadoutSlot = i;
            m_screen = GameScreen::Menu;
            SetMessage(L"Pick a replacement unit.");
            return;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        if (Contains(BriefingDifficultyRect(i), pos))
        {
            m_difficulty = static_cast<Difficulty>(i);
            SetMessage(std::wstring(L"Difficulty ") + DifficultyLabel());
            return;
        }
    }
}

void PawlineGameImpl::OnShopClick(Vec2 pos)
{
    if (Contains(ShopBackButtonRect(), pos))
    {
        ResetToMenu();
        return;
    }

    for (int i = 0; i < kRosterCount; ++i)
    {
        if (Contains(ShopUnitRect(i), pos))
        {
            m_shopSelectedUnit = i;
            TryBuyOrUpgradeUnit(static_cast<PlayerUnit>(i));
            return;
        }
    }
}

void PawlineGameImpl::OnOptionsClick(Vec2 pos)
{
    if (Contains(OptionsShakeButtonRect(), pos))
    {
        m_hitShakeEnabled = !m_hitShakeEnabled;
        return;
    }
    if (Contains(OptionsSpeedDownButtonRect(), pos))
    {
        m_defaultGameSpeed = std::max(0.5f, m_defaultGameSpeed - 0.5f);
        return;
    }
    if (Contains(OptionsSpeedUpButtonRect(), pos))
    {
        m_defaultGameSpeed = std::min(3.0f, m_defaultGameSpeed + 0.5f);
        return;
    }
    if (Contains(OptionsViewDownButtonRect(), pos))
    {
        m_userViewScale = std::max(0.82f, m_userViewScale - 0.05f);
        UpdateViewMetrics();
        return;
    }
    if (Contains(OptionsViewUpButtonRect(), pos))
    {
        m_userViewScale = std::min(1.0f, m_userViewScale + 0.05f);
        UpdateViewMetrics();
        return;
    }
    if (Contains(OptionsViewResetButtonRect(), pos))
    {
        m_userViewScale = 0.96f;
        UpdateViewMetrics();
        return;
    }
    if (Contains(OptionsBackButtonRect(), pos))
    {
        m_screen = GameScreen::Title;
    }
}

void PawlineGameImpl::OnEscapeMenuClick(Vec2 pos)
{
    if (Contains(EscapeResumeButtonRect(), pos))
    {
        CloseEscapeMenu();
        return;
    }
    if (Contains(EscapeShakeButtonRect(), pos))
    {
        m_hitShakeEnabled = !m_hitShakeEnabled;
        return;
    }
    if (Contains(EscapeSpeedDownButtonRect(), pos))
    {
        AdjustEscapeMenuSpeed(-0.5f);
        return;
    }
    if (Contains(EscapeSpeedUpButtonRect(), pos))
    {
        AdjustEscapeMenuSpeed(0.5f);
        return;
    }
    if (Contains(EscapeStageButtonRect(), pos))
    {
        m_escapeMenuOpen = false;
        ResetToMenu();
        return;
    }
    if (Contains(EscapeQuitButtonRect(), pos))
    {
        PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
    }
}

void PawlineGameImpl::OnResultClick(Vec2 pos)
{
    if (Contains(ResultRetryButtonRect(), pos))
    {
        ResetGame();
        return;
    }
    if (Contains(ResultNextButtonRect(), pos))
    {
        if (m_victory && m_selectedStage < kStageCount - 1)
        {
            ++m_selectedStage;
            ResetGame();
        }
        else
        {
            ResetToMenu();
        }
        return;
    }
    if (Contains(ResultMenuButtonRect(), pos))
    {
        ResetToMenu();
    }
}

void PawlineGameImpl::TryBuyOrUpgradeUnit(PlayerUnit unit)
{
    // LUMEN is persistent progression: locked units are purchased once, while
    // unlocked units spend the same currency on level upgrades.
    const int index = UnitIndex(unit);
    const UnitStats base = GetPlayerStats(unit);

    if (!m_unitUnlocked[index])
    {
        const int cost = UnitUnlockCost(unit);
        if (m_lumen < cost)
        {
            SetMessage(L"Need " + ToWideInt(cost) + L" LUMEN to buy " + base.name + L".");
            return;
        }

        m_lumen -= cost;
        m_unitUnlocked[index] = true;
        m_unitLevels[index] = 1;
        SetMessage(base.name + L" unlocked.");
        AddFloatText({640.0f, 702.0f}, base.name + L" unlocked", D2D1::ColorF(0xB8FF89), 1.2f);
        AddRing({640.0f, 402.0f}, 180.0f, 0.55f, base.accent, 3.0f);
        SaveProgress();
        return;
    }

    if (m_unitLevels[index] >= kMaxUnitLevel)
    {
        SetMessage(base.name + L" is already max level.");
        return;
    }

    const int cost = UnitUpgradeCost(unit);
    if (m_lumen < cost)
    {
        SetMessage(L"Need " + ToWideInt(cost) + L" LUMEN to upgrade " + base.name + L".");
        return;
    }

    m_lumen -= cost;
    ++m_unitLevels[index];
    SetMessage(base.name + L" upgraded to Lv." + ToWideInt(m_unitLevels[index]) + L".");
    AddFloatText({640.0f, 702.0f}, base.name + L" Lv." + ToWideInt(m_unitLevels[index]), base.accent, 1.2f);
    AddRing({640.0f, 402.0f}, 150.0f, 0.48f, base.accent, 3.0f);
    SaveProgress();
}

void PawlineGameImpl::SetLoadoutUnit(PlayerUnit unit)
{
    if (!IsUnitUnlocked(unit))
    {
        SetMessage(L"Locked unit. Open the shop to buy it.");
        return;
    }

    for (int i = 0; i < kLoadoutSize; ++i)
    {
        if (m_loadout[i] == unit)
        {
            std::swap(m_loadout[i], m_loadout[m_selectedLoadoutSlot]);
            return;
        }
    }

    m_loadout[m_selectedLoadoutSlot] = unit;
}

void PawlineGameImpl::OnKeyDown(WPARAM key)
{
    if (m_escapeMenuOpen)
    {
        if (key == VK_ESCAPE || key == VK_BACK || key == 'P')
        {
            CloseEscapeMenu();
        }
        else if (key == 'H')
        {
            m_hitShakeEnabled = !m_hitShakeEnabled;
        }
        else if (key == VK_LEFT || key == VK_OEM_MINUS || key == VK_OEM_4)
        {
            AdjustEscapeMenuSpeed(-0.5f);
        }
        else if (key == VK_RIGHT || key == VK_OEM_PLUS || key == VK_OEM_6)
        {
            AdjustEscapeMenuSpeed(0.5f);
        }
        else if (key == 'M')
        {
            m_escapeMenuOpen = false;
            ResetToMenu();
        }
        else if (key == 'Q')
        {
            PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
        }
        return;
    }

    if (m_screen == GameScreen::Shop)
    {
        if (key == VK_ESCAPE || key == VK_BACK || key == 'M')
        {
            ResetToMenu();
        }
        return;
    }

    if (m_screen == GameScreen::Briefing)
    {
        if (key == VK_RETURN || key == VK_SPACE)
        {
            ResetGame();
        }
        else if (key == VK_ESCAPE || key == VK_BACK || key == 'M')
        {
            ResetToMenu();
        }
        else if (key == 'S' || key == 'B')
        {
            m_screen = GameScreen::Shop;
        }
        else if (key == 'E')
        {
            m_difficulty = Difficulty::Easy;
            SetMessage(L"Difficulty EASY");
        }
        else if (key == 'N')
        {
            m_difficulty = Difficulty::Normal;
            SetMessage(L"Difficulty NORMAL");
        }
        else if (key == 'H')
        {
            m_difficulty = Difficulty::Hard;
            SetMessage(L"Difficulty HARD");
        }
        return;
    }

    if (key == VK_ESCAPE && m_screen != GameScreen::Title && m_screen != GameScreen::Options)
    {
        OpenEscapeMenu();
        return;
    }

    if (m_screen == GameScreen::Title)
    {
        if (key == VK_RETURN || key == VK_SPACE)
        {
            ResetToMenu();
        }
        else if (key == 'O')
        {
            m_screen = GameScreen::Options;
        }
        else if (key == VK_ESCAPE || key == 'Q')
        {
            PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
        }
        return;
    }

    if (m_screen == GameScreen::Options)
    {
        if (key == VK_ESCAPE || key == VK_BACK)
        {
            m_screen = GameScreen::Title;
        }
        else if (key == 'H')
        {
            m_hitShakeEnabled = !m_hitShakeEnabled;
        }
        else if (key == VK_LEFT || key == VK_OEM_MINUS || key == VK_OEM_4)
        {
            m_defaultGameSpeed = std::max(0.5f, m_defaultGameSpeed - 0.5f);
        }
        else if (key == VK_RIGHT || key == VK_OEM_PLUS || key == VK_OEM_6)
        {
            m_defaultGameSpeed = std::min(3.0f, m_defaultGameSpeed + 0.5f);
        }
        else if (key == VK_DOWN)
        {
            m_userViewScale = std::max(0.82f, m_userViewScale - 0.05f);
            UpdateViewMetrics();
        }
        else if (key == VK_UP)
        {
            m_userViewScale = std::min(1.0f, m_userViewScale + 0.05f);
            UpdateViewMetrics();
        }
        else if (key == 'A')
        {
            m_userViewScale = 0.96f;
            UpdateViewMetrics();
        }
        return;
    }

    if (m_screen == GameScreen::Menu)
    {
        if (key >= '1' && key <= '9')
        {
            m_selectedStage = static_cast<int>(key - '1');
        }
        else if (key == '0')
        {
            m_selectedStage = 9;
        }
        else if (key == VK_LEFT)
        {
            m_selectedStage = std::max(0, m_selectedStage - 1);
        }
        else if (key == VK_RIGHT)
        {
            m_selectedStage = std::min(kStageCount - 1, m_selectedStage + 1);
        }
        else if (key == VK_RETURN || key == VK_SPACE)
        {
            m_screen = GameScreen::Briefing;
            SetMessage(L"Check your deck before launch.");
        }
        else if (key == 'S' || key == 'B')
        {
            m_screen = GameScreen::Shop;
        }
        return;
    }

    if (m_screen == GameScreen::Result)
    {
        if (key == 'R')
        {
            ResetGame();
        }
        else if (key == 'M')
        {
            ResetToMenu();
        }
        else if ((key == VK_RETURN || key == VK_SPACE) && m_victory && m_selectedStage < kStageCount - 1)
        {
            ++m_selectedStage;
            ResetGame();
        }
        return;
    }

    if (key == VK_F1 && m_screen == GameScreen::Playing)
    {
        m_showcaseMode = !m_showcaseMode;
        m_showcaseTimer = 0.0f;
        SetMessage(m_showcaseMode ? L"Showcase camera on." : L"Showcase camera off.");
        return;
    }

    if (key >= '1' && key < '1' + kLoadoutSize)
    {
        TrySpawnPlayer(static_cast<int>(key - '1'));
        return;
    }

    switch (key)
    {
    case 'W':
        TryUpgradeWallet();
        break;
    case VK_SPACE:
        TryFireCannon();
        break;
    case VK_OEM_MINUS:
    case VK_OEM_4:
        DecreaseGameSpeed();
        break;
    case VK_OEM_PLUS:
    case VK_OEM_6:
        IncreaseGameSpeed();
        break;
    case 'P':
        m_paused = !m_paused;
        SetMessage(m_paused ? L"Paused." : L"Resumed.");
        break;
    case 'R':
        ResetGame();
        break;
    default:
        break;
    }
}
