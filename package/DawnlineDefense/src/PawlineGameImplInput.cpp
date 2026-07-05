#include "PawlineGameImpl.h"

// Mouse/keyboard commands and menu/shop state transitions.
void PawlineGameImpl::SetMessage(const std::wstring& message)
{
    m_message = message;
    // 화면을 오래 가리지 않도록 짧은 토스트로 보여준다. 마우스가 닿으면 Update에서 더 빠르게 사라진다.
    m_messageTimer = 1.55f;
}

void PawlineGameImpl::OpenEscapeMenu()
{
    if (m_escapeMenuOpen)
    {
        return;
    }

    m_pauseBeforeEscape = m_paused;
    m_escapeMenuOpen = true;
    PlaySfxFile(L"events\\ui_open.wav", SfxKind::Ui, 0.03f);
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
    PlaySfxFile(L"events\\ui_close.wav", SfxKind::Ui, 0.03f);
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

bool PawlineGameImpl::IsInteractivePoint(Vec2 pos) const
{
    if (m_escapeMenuOpen)
    {
        return Contains(EscapeResumeButtonRect(), pos) ||
               Contains(EscapeShakeButtonRect(), pos) ||
               Contains(EscapeSpeedDownButtonRect(), pos) ||
               Contains(EscapeSpeedUpButtonRect(), pos) ||
               Contains(EscapeStageButtonRect(), pos) ||
               Contains(EscapeQuitButtonRect(), pos);
    }

    switch (m_screen)
    {
    case GameScreen::Title:
        return Contains(TitleStartButtonRect(), pos) ||
               Contains(TitleStoryButtonRect(), pos) ||
               Contains(TitleDemoButtonRect(), pos) ||
               Contains(TitleOptionsButtonRect(), pos) ||
               Contains(TitleQuitButtonRect(), pos);
    case GameScreen::StoryIntro:
        return Contains(StorySkipButtonRect(), pos);
    case GameScreen::Ending:
        return true;
    case GameScreen::Options:
        for (int i = 0; i < kSaveSlotCount; ++i)
        {
            if (Contains(OptionsSaveSlotButtonRect(i), pos))
            {
                return true;
            }
        }
        return Contains(OptionsShakeButtonRect(), pos) ||
               Contains(OptionsFlashButtonRect(), pos) ||
               Contains(OptionsSfxSliderRect(), pos) ||
               Contains(OptionsBgmSliderRect(), pos) ||
               Contains(OptionsAudioResetButtonRect(), pos) ||
               Contains(OptionsSpeedSliderRect(), pos) ||
               Contains(OptionsViewSliderRect(), pos) ||
               Contains(OptionsViewResetButtonRect(), pos) ||
               Contains(OptionsSaveProgressButtonRect(), pos) ||
               Contains(OptionsLoadProgressButtonRect(), pos) ||
               Contains(OptionsDeleteProgressButtonRect(), pos) ||
               Contains(OptionsResetProgressButtonRect(), pos) ||
               Contains(OptionsBackButtonRect(), pos);
    case GameScreen::Menu:
        for (int i = 0; i < kStageCount; ++i)
        {
            if (Contains(MenuStageRect(i), pos))
            {
                return true;
            }
        }
        for (int i = 0; i < kLoadoutSize; ++i)
        {
            if (Contains(MenuLoadoutSlotRect(i), pos))
            {
                return true;
            }
        }
        for (int i = 0; i < kRosterCount; ++i)
        {
            if (Contains(RosterCardRect(i), pos))
            {
                return true;
            }
        }
        return Contains(StartGameButtonRect(), pos) ||
               Contains(MenuShopButtonRect(), pos) ||
               Contains(MenuArchiveButtonRect(), pos);
    case GameScreen::Archive:
        if (Contains(ArchiveBackButtonRect(), pos))
        {
            return true;
        }
        for (int i = 0; i < 3; ++i)
        {
            if (Contains(ArchiveTabRect(i), pos))
            {
                return true;
            }
        }
        return false;
    case GameScreen::Briefing:
        for (int i = 0; i < kLoadoutSize; ++i)
        {
            if (Contains(BriefingLoadoutSlotRect(i), pos))
            {
                return true;
            }
        }
        for (int i = 0; i < 3; ++i)
        {
            if (Contains(BriefingDifficultyRect(i), pos))
            {
                return true;
            }
        }
        return Contains(BriefingStartButtonRect(), pos) ||
               Contains(BriefingBackButtonRect(), pos) ||
               Contains(BriefingShopButtonRect(), pos);
    case GameScreen::Shop:
        if (Contains(ShopBackButtonRect(), pos))
        {
            return true;
        }
        for (int i = 0; i < kRosterCount; ++i)
        {
            if (Contains(ShopUnitRect(i), pos))
            {
                return true;
            }
        }
        return false;
    case GameScreen::Result:
        return Contains(ResultRetryButtonRect(), pos) ||
               Contains(ResultNextButtonRect(), pos) ||
               Contains(ResultMenuButtonRect(), pos);
    case GameScreen::Playing:
    {
        const D2D1_RECT_F cameraRail = D2D1::RectF(40.0f, 94.0f, 1240.0f, 116.0f);
        if (Contains(cameraRail, pos))
        {
            return true;
        }
        for (int i = 0; i < kLoadoutSize; ++i)
        {
            if (Contains(CardRect(i), pos))
            {
                return true;
            }
        }
        return Contains(WalletButtonRect(), pos) ||
               Contains(CannonButtonRect(), pos) ||
               Contains(SpeedDownButtonRect(), pos) ||
               Contains(SpeedUpButtonRect(), pos) ||
               Contains(PauseButtonRect(), pos) ||
               Contains(RestartButtonRect(), pos);
    }
    }
    return false;
}

void PawlineGameImpl::OnLeftClick(Vec2 pos)
{
    // Every screen owns its click handling. The shared rectangle helpers live in
    // PawlineGameImplLayout.cpp, so render and input stay pixel-aligned.
    if (IsInteractivePoint(pos))
    {
        auto centerOf = [](D2D1_RECT_F rect) {
            return Vec2{(rect.left + rect.right) * 0.5f, (rect.top + rect.bottom) * 0.5f};
        };

        // 큰 선택 펄스는 유닛 슬롯에만 사용한다. 일반 버튼은 작은 클릭 반응만 남겨
        // 유닛을 고른 것처럼 오해되는 상황을 줄인다.
        bool unitSlotPulse = false;
        if (m_screen == GameScreen::Playing)
        {
            for (int i = 0; i < kLoadoutSize; ++i)
            {
                const D2D1_RECT_F rect = CardRect(i);
                if (Contains(rect, pos))
                {
                    AddUiPulse(centerOf(rect), PlayerStats(m_loadout[i]).accent, 54.0f, 0.36f);
                    unitSlotPulse = true;
                    break;
                }
            }
        }
        else if (m_screen == GameScreen::Menu)
        {
            for (int i = 0; i < kLoadoutSize; ++i)
            {
                const D2D1_RECT_F rect = MenuLoadoutSlotRect(i);
                if (Contains(rect, pos))
                {
                    AddUiPulse(centerOf(rect), PlayerStats(m_loadout[i]).accent, 48.0f, 0.34f);
                    unitSlotPulse = true;
                    break;
                }
            }
            if (!unitSlotPulse)
            {
                for (int i = 0; i < kRosterCount; ++i)
                {
                    const D2D1_RECT_F rect = RosterCardRect(i);
                    if (Contains(rect, pos))
                    {
                        const PlayerUnit unit = static_cast<PlayerUnit>(i);
                        AddUiPulse(centerOf(rect), PlayerStats(unit).accent, 42.0f, 0.30f);
                        unitSlotPulse = true;
                        break;
                    }
                }
            }
        }
        else if (m_screen == GameScreen::Briefing)
        {
            for (int i = 0; i < kLoadoutSize; ++i)
            {
                const D2D1_RECT_F rect = BriefingLoadoutSlotRect(i);
                if (Contains(rect, pos))
                {
                    AddUiPulse(centerOf(rect), PlayerStats(m_loadout[i]).accent, 48.0f, 0.34f);
                    unitSlotPulse = true;
                    break;
                }
            }
        }

        if (!unitSlotPulse)
        {
            AddUiPulse(pos, D2D1::ColorF(0x65B8FF, 0.46f), 26.0f, 0.22f);
        }
        PlaySfx(SfxKind::Ui, 0.025f);
    }
    if (m_escapeMenuOpen)
    {
        OnEscapeMenuClick(pos);
        return;
    }

    if (m_screen == GameScreen::Title)
    {
        if (Contains(TitleStartButtonRect(), pos))
        {
            if (!m_introViewedThisSession && !HasAnyProgressFile())
            {
                BeginStoryCrawl(true);
            }
            else
            {
                ResetToMenu();
            }
        }
        else if (Contains(TitleStoryButtonRect(), pos))
        {
            BeginStoryCrawl(false);
        }
        else if (Contains(TitleDemoButtonRect(), pos))
        {
            StartDemoRun();
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
    if (m_screen == GameScreen::StoryIntro)
    {
        FinishStoryCrawl();
        return;
    }
    if (m_screen == GameScreen::Ending)
    {
        FinishEndingScene();
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
    if (m_screen == GameScreen::Archive)
    {
        if (Contains(ArchiveBackButtonRect(), pos))
        {
            ResetToMenu();
            return;
        }
        for (int i = 0; i < 3; ++i)
        {
            if (Contains(ArchiveTabRect(i), pos))
            {
                m_archiveTab = i;
                return;
            }
        }
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

    const D2D1_RECT_F cameraRail = D2D1::RectF(40.0f, 94.0f, 1240.0f, 116.0f);
    if (Contains(cameraRail, pos))
    {
        // 상단 미니맵 레일을 클릭하면 해당 월드 위치로 카메라를 이동한다.
        const float pct = Clamp01((pos.x - cameraRail.left) / (cameraRail.right - cameraRail.left));
        m_cameraTargetX = std::max(0.0f, std::min(kCameraMaxX, pct * kWorldWidth - kWidth * 0.5f));
        m_cameraX = Lerp(m_cameraX, m_cameraTargetX, 0.45f);
        SetMessage(L"카메라를 이동했어.");
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
        SetMessage(m_paused ? L"일시정지." : L"전투 재개.");
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
            SelectStage(i);
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
        if (!IsStageUnlocked(m_selectedStage))
        {
            SetMessage(L"잠긴 스테이지야. 이전 행성을 먼저 클리어해줘.");
            return;
        }
        m_screen = GameScreen::Briefing;
        SetMessage(L"출격 전에 편성을 확인해줘.");
        return;
    }
    if (Contains(MenuShopButtonRect(), pos))
    {
        m_screen = GameScreen::Shop;
        return;
    }
    if (Contains(MenuArchiveButtonRect(), pos))
    {
        m_screen = GameScreen::Archive;
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
        if (Contains(BriefingLoadoutSlotRect(i), pos))
        {
            m_selectedLoadoutSlot = i;
            m_screen = GameScreen::Menu;
            SetMessage(L"교체할 유닛을 골라줘.");
            return;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        if (Contains(BriefingDifficultyRect(i), pos))
        {
            m_difficulty = static_cast<Difficulty>(i);
            SetMessage(std::wstring(L"난이도 ") + DifficultyLabel());
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
    const auto sliderValue = [](D2D1_RECT_F rect, Vec2 point) {
        const float width = std::max(1.0f, rect.right - rect.left);
        return Clamp01((point.x - rect.left) / width);
    };

    for (int i = 0; i < kSaveSlotCount; ++i)
    {
        if (Contains(OptionsSaveSlotButtonRect(i), pos))
        {
            SelectSaveSlot(i);
            return;
        }
    }
    if (Contains(OptionsShakeButtonRect(), pos))
    {
        m_hitShakeEnabled = !m_hitShakeEnabled;
        return;
    }
    if (Contains(OptionsFlashButtonRect(), pos))
    {
        m_reduceFlashes = !m_reduceFlashes;
        SetMessage(m_reduceFlashes ? L"눈부심 줄이기 켜짐." : L"눈부심 줄이기 꺼짐.");
        return;
    }
    if (Contains(OptionsSfxSliderRect(), pos))
    {
        m_sfxVolume = sliderValue(OptionsSfxSliderRect(), pos);
        m_soundEnabled = m_sfxVolume > 0.001f;
        m_audio.SetVolume(m_sfxVolume);
        PlaySfx(SfxKind::Ui, 0.02f);
        return;
    }
    if (Contains(OptionsBgmSliderRect(), pos))
    {
        m_bgmVolume = sliderValue(OptionsBgmSliderRect(), pos);
        SyncMusicVolume();
        PlaySfx(SfxKind::Ui, 0.02f);
        return;
    }
    if (Contains(OptionsAudioResetButtonRect(), pos))
    {
        ResetAudioVolumes();
        return;
    }
    if (Contains(OptionsSpeedSliderRect(), pos))
    {
        const float value = sliderValue(OptionsSpeedSliderRect(), pos);
        m_defaultGameSpeed = 0.5f + std::round(value * 5.0f) * 0.5f;
        return;
    }
    if (Contains(OptionsViewSliderRect(), pos))
    {
        const float value = sliderValue(OptionsViewSliderRect(), pos);
        m_userViewScale = std::clamp(0.82f + value * 0.18f, 0.82f, 1.0f);
        UpdateViewMetrics();
        return;
    }
    if (Contains(OptionsViewResetButtonRect(), pos))
    {
        m_userViewScale = 1.0f;
        UpdateViewMetrics();
        return;
    }
    if (Contains(OptionsSaveProgressButtonRect(), pos))
    {
        SaveProgress();
        SetMessage(SaveSlotLabel() + L"에 저장했어.");
        return;
    }
    if (Contains(OptionsLoadProgressButtonRect(), pos))
    {
        LoadProgress();
        UpdateViewMetrics();
        SetMessage(SaveSlotLabel() + L"에서 불러왔어.");
        return;
    }
    if (Contains(OptionsDeleteProgressButtonRect(), pos))
    {
        if (m_deleteConfirmTimer > 0.0f)
        {
            DeleteSelectedSaveSlot();
        }
        else
        {
            m_deleteConfirmTimer = 5.0f;
            SetMessage(SaveSlotLabel() + L"을 한 번 더 누르면 삭제돼.");
        }
        return;
    }
    if (Contains(OptionsResetProgressButtonRect(), pos))
    {
        if (m_resetConfirmTimer > 0.0f)
        {
            ResetProgressData();
        }
        else
        {
            m_resetConfirmTimer = 5.0f;
            SetMessage(L"한 번 더 누르면 진행 데이터가 초기화돼.");
        }
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
            SetMessage(base.name + L" 구매에 LUMEN " + ToWideInt(cost) + L" 필요.");
            return;
        }

        m_lumen -= cost;
        m_unitUnlocked[index] = true;
        m_unitLevels[index] = 1;
        SetMessage(base.name + L" 해금 완료.");
        AddFloatText({640.0f, 702.0f}, base.name + L" 해금", D2D1::ColorF(0xB8FF89), 1.2f);
        AddRing({640.0f, 402.0f}, 180.0f, 0.55f, base.accent, 3.0f);
        SaveProgress();
        return;
    }

    if (m_unitLevels[index] >= kMaxUnitLevel)
    {
        SetMessage(base.name + L"은 이미 최대 진화 상태야.");
        return;
    }

    const int cost = UnitUpgradeCost(unit);
    if (m_lumen < cost)
    {
        SetMessage(base.name + L" 강화에 LUMEN " + ToWideInt(cost) + L" 필요.");
        return;
    }

    m_lumen -= cost;
    ++m_unitLevels[index];
    const bool evolved = m_unitLevels[index] >= kMaxUnitLevel;
    SetMessage(evolved ? base.name + L" 진화 완료." : base.name + L" Lv." + ToWideInt(m_unitLevels[index]) + L" 강화 완료.");
    AddFloatText({640.0f, 702.0f}, evolved ? L"진화 " + base.name : base.name + L" Lv." + ToWideInt(m_unitLevels[index]), evolved ? D2D1::ColorF(0xF6FF83) : base.accent, 1.2f);
    AddRing({640.0f, 402.0f}, evolved ? 210.0f : 150.0f, evolved ? 0.72f : 0.48f, evolved ? D2D1::ColorF(0xF6FF83) : base.accent, evolved ? 5.0f : 3.0f);
    SaveProgress();
}

void PawlineGameImpl::SetLoadoutUnit(PlayerUnit unit)
{
    if (!IsUnitUnlocked(unit))
    {
        SetMessage(L"잠긴 유닛이야. 상점에서 먼저 구매해줘.");
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

    if (key == VK_F2)
    {
        m_debugMode = !m_debugMode;
        SetMessage(m_debugMode ? L"테스트 모드 켜짐." : L"테스트 모드 꺼짐.");
        return;
    }

    if (m_debugMode && key == VK_F6)
    {
        TriggerDebugUnlockAll();
        return;
    }

    if (m_screen == GameScreen::Archive)
    {
        if (key == VK_ESCAPE || key == VK_BACK || key == 'M')
        {
            ResetToMenu();
        }
        else if (key == VK_LEFT)
        {
            m_archiveTab = (m_archiveTab + 2) % 3;
        }
        else if (key == VK_RIGHT || key == VK_TAB)
        {
            m_archiveTab = (m_archiveTab + 1) % 3;
        }
        else if (key >= '1' && key <= '3')
        {
            m_archiveTab = static_cast<int>(key - '1');
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
            SetMessage(L"난이도 쉬움.");
        }
        else if (key == 'N')
        {
            m_difficulty = Difficulty::Normal;
            SetMessage(L"난이도 보통.");
        }
        else if (key == 'H')
        {
            m_difficulty = Difficulty::Hard;
            SetMessage(L"난이도 어려움.");
        }
        return;
    }

    if (key == VK_ESCAPE &&
        m_screen != GameScreen::Title &&
        m_screen != GameScreen::StoryIntro &&
        m_screen != GameScreen::Ending &&
        m_screen != GameScreen::Options)
    {
        OpenEscapeMenu();
        return;
    }

    if (m_screen == GameScreen::Title)
    {
        if (key == VK_RETURN || key == VK_SPACE)
        {
            if (!m_introViewedThisSession && !HasAnyProgressFile())
            {
                BeginStoryCrawl(true);
            }
            else
            {
                ResetToMenu();
            }
        }
        else if (key == 'S')
        {
            BeginStoryCrawl(false);
        }
        else if (key == 'D')
        {
            StartDemoRun();
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

    if (m_screen == GameScreen::StoryIntro)
    {
        if (key == VK_RETURN || key == VK_SPACE || key == VK_ESCAPE || key == VK_BACK)
        {
            FinishStoryCrawl();
        }
        return;
    }

    if (m_screen == GameScreen::Ending)
    {
        if (key == VK_RETURN || key == VK_SPACE || key == VK_ESCAPE || key == VK_BACK)
        {
            FinishEndingScene();
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
        else if (key == 'F')
        {
            m_reduceFlashes = !m_reduceFlashes;
            SetMessage(m_reduceFlashes ? L"눈부심 줄이기 켜짐." : L"눈부심 줄이기 꺼짐.");
        }
        else if (key == 'M')
        {
            AdjustSfxVolume(-0.10f);
        }
        else if (key == 'N')
        {
            AdjustSfxVolume(0.10f);
        }
        else if (key == 'B')
        {
            AdjustBgmVolume(-0.10f);
        }
        else if (key == 'V')
        {
            AdjustBgmVolume(0.10f);
        }
        else if (key == 'R')
        {
            ResetAudioVolumes();
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
            m_userViewScale = 1.0f;
            UpdateViewMetrics();
        }
        else if (key >= '1' && key <= '3')
        {
            SelectSaveSlot(static_cast<int>(key - '1'));
        }
        else if (key == 'S')
        {
            SaveProgress();
            SetMessage(SaveSlotLabel() + L"에 저장했어.");
        }
        else if (key == 'L')
        {
            LoadProgress();
            UpdateViewMetrics();
            SetMessage(SaveSlotLabel() + L"에서 불러왔어.");
        }
        else if (key == 'D')
        {
            if (m_deleteConfirmTimer > 0.0f)
            {
                DeleteSelectedSaveSlot();
            }
            else
            {
                m_deleteConfirmTimer = 5.0f;
                SetMessage(SaveSlotLabel() + L"을 D로 한 번 더 누르면 삭제돼.");
            }
        }
        else if (key == 'X')
        {
            if (m_resetConfirmTimer > 0.0f)
            {
                ResetProgressData();
            }
            else
            {
                m_resetConfirmTimer = 5.0f;
                SetMessage(L"X를 한 번 더 누르면 진행 데이터가 초기화돼.");
            }
        }
        return;
    }

    if (m_screen == GameScreen::Menu)
    {
        if (key >= '1' && key <= '9')
        {
            SelectStage(static_cast<int>(key - '1'));
        }
        else if (key == '0')
        {
            SelectStage(9);
        }
        else if (key == VK_LEFT)
        {
            SelectStage(m_selectedStage - 1);
        }
        else if (key == VK_RIGHT)
        {
            SelectStage(m_selectedStage + 1);
        }
        else if (key == VK_RETURN || key == VK_SPACE)
        {
            if (!IsStageUnlocked(m_selectedStage))
            {
                SetMessage(L"잠긴 스테이지야. 이전 행성을 먼저 클리어해줘.");
                return;
            }
            m_screen = GameScreen::Briefing;
            SetMessage(L"출격 전에 편성을 확인해줘.");
        }
        else if (key == 'S' || key == 'B')
        {
            m_screen = GameScreen::Shop;
        }
        else if (key == 'D' || key == 'C')
        {
            m_screen = GameScreen::Archive;
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

    if (m_debugMode && key == VK_F3)
    {
        m_energy = MaxEnergy();
        m_lumen += 500;
        m_cannonCharge = 100.0f;
        SetMessage(L"테스트 자원 충전.");
        return;
    }

    if (m_debugMode && key == VK_F4 && m_screen == GameScreen::Playing)
    {
        SpawnEnemy(StageBossType(), true);
        if (!m_units.empty())
        {
            Unit& boss = m_units.back();
            PromoteBossUnit(boss);
            TriggerBossEntrance(boss, GetEnemyStats(static_cast<EnemyUnit>(boss.kind), ThreatLevel()).accent);
        }
        m_bossSpawned = true;
        SetMessage(L"테스트 보스 소환.");
        return;
    }

    if (m_debugMode && key == VK_F5)
    {
        TriggerDebugClear();
        return;
    }

    if (key == VK_F1 && m_screen == GameScreen::Playing)
    {
        m_showcaseMode = !m_showcaseMode;
        m_showcaseTimer = 0.0f;
        m_demoSpawnTimer = 0.35f;
        m_demoWalletTimer = 1.4f;
        SetMessage(m_showcaseMode ? L"데모 모드 켜짐." : L"데모 모드 꺼짐.");
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
        SetMessage(m_paused ? L"일시정지." : L"전투 재개.");
        break;
    case 'R':
        ResetGame();
        break;
    default:
        break;
    }
}
