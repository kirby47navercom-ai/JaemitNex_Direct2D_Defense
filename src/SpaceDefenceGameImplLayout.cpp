#include "SpaceDefenceGameImpl.h"

// Stable UI hit rectangles shared by input and rendering.
D2D1_RECT_F SpaceDefenceGameImpl::CardRect(int index) const
{
    const float x = 22.0f + static_cast<float>(index) * 126.0f;
    return D2D1::RectF(x, 628.0f, x + 118.0f, 774.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::TitleStartButtonRect() const
{
    return D2D1::RectF(492.0f, 464.0f, 788.0f, 512.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::TitleDemoButtonRect() const
{
    return D2D1::RectF(492.0f, 572.0f, 788.0f, 620.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::TitleStoryButtonRect() const
{
    // 타이틀에서 프롤로그를 다시 볼 수 있는 전용 버튼 영역이다.
    return D2D1::RectF(492.0f, 518.0f, 788.0f, 566.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::TitleOptionsButtonRect() const
{
    return D2D1::RectF(492.0f, 626.0f, 788.0f, 674.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::TitleQuitButtonRect() const
{
    return D2D1::RectF(492.0f, 680.0f, 788.0f, 728.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::StorySkipButtonRect() const
{
    // 프롤로그 화면에서 마우스로 즉시 넘길 수 있는 명확한 스킵 버튼 영역이다.
    return D2D1::RectF(510.0f, 712.0f, 770.0f, 760.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsShakeButtonRect() const
{
    return D2D1::RectF(270.0f, 288.0f, 470.0f, 336.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsFlashButtonRect() const
{
    return D2D1::RectF(490.0f, 288.0f, 690.0f, 336.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsSfxDownButtonRect() const
{
    return D2D1::RectF(740.0f, 288.0f, 782.0f, 330.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsSfxUpButtonRect() const
{
    return D2D1::RectF(1018.0f, 288.0f, 1060.0f, 330.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsSfxSliderRect() const
{
    // 옵션 화면에서 효과음 볼륨을 클릭으로 바로 조정하는 바 영역이다.
    return D2D1::RectF(762.0f, 324.0f, 1022.0f, 344.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsUiSliderRect() const
{
    // 버튼과 메뉴 피드백 소리만 따로 조정하는 바 영역이다.
    return D2D1::RectF(762.0f, 404.0f, 1022.0f, 424.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsBgmDownButtonRect() const
{
    return D2D1::RectF(740.0f, 392.0f, 782.0f, 434.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsBgmUpButtonRect() const
{
    return D2D1::RectF(1018.0f, 392.0f, 1060.0f, 434.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsBgmSliderRect() const
{
    // 옵션 화면에서 배경음악 볼륨을 클릭으로 바로 조정하는 바 영역이다.
    return D2D1::RectF(762.0f, 484.0f, 1022.0f, 504.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsAudioResetButtonRect() const
{
    return D2D1::RectF(782.0f, 538.0f, 1002.0f, 584.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsSpeedDownButtonRect() const
{
    return D2D1::RectF(270.0f, 412.0f, 312.0f, 454.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsSpeedUpButtonRect() const
{
    return D2D1::RectF(648.0f, 412.0f, 690.0f, 454.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsSpeedSliderRect() const
{
    // 기본 게임 속도는 0.5x~3.0x 사이를 단계적으로 고르는 바다.
    return D2D1::RectF(330.0f, 424.0f, 630.0f, 444.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsViewDownButtonRect() const
{
    return D2D1::RectF(270.0f, 512.0f, 312.0f, 554.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsViewUpButtonRect() const
{
    return D2D1::RectF(648.0f, 512.0f, 690.0f, 554.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsViewSliderRect() const
{
    // 전체 화면에서 잘림을 줄이는 안전 여백 스케일 바다.
    return D2D1::RectF(330.0f, 524.0f, 630.0f, 544.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsViewResetButtonRect() const
{
    return D2D1::RectF(390.0f, 554.0f, 570.0f, 598.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsSaveSlotButtonRect(int index) const
{
    const float x = 470.0f + static_cast<float>(index) * 180.0f;
    return D2D1::RectF(x, 164.0f, x + 160.0f, 208.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsDeleteProgressButtonRect() const
{
    return D2D1::RectF(350.0f, 650.0f, 530.0f, 692.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsResetProgressButtonRect() const
{
    return D2D1::RectF(750.0f, 650.0f, 930.0f, 692.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::OptionsBackButtonRect() const
{
    return D2D1::RectF(490.0f, 700.0f, 790.0f, 740.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::MenuStageRect(int index) const
{
    const int col = index % 5;
    const int row = index / 5;
    const float x = 48.0f + static_cast<float>(col) * 240.0f;
    const float y = 138.0f + static_cast<float>(row) * 94.0f;
    return D2D1::RectF(x, y, x + 216.0f, y + 82.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::MenuLoadoutSlotRect(int index) const
{
    const float x = 58.0f + static_cast<float>(index) * 116.0f;
    return D2D1::RectF(x, 358.0f, x + 102.0f, 476.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::RosterCardRect(int index) const
{
    const int col = index % 4;
    const int row = index / 4;
    const float x = 650.0f + static_cast<float>(col) * 145.0f;
    const float y = 348.0f + static_cast<float>(row) * 86.0f;
    return D2D1::RectF(x, y, x + 132.0f, y + 78.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::StartGameButtonRect() const
{
    return D2D1::RectF(986.0f, 720.0f, 1228.0f, 774.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::MenuShopButtonRect() const
{
    return D2D1::RectF(818.0f, 720.0f, 974.0f, 774.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::MenuArchiveButtonRect() const
{
    return D2D1::RectF(650.0f, 720.0f, 806.0f, 774.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::ArchiveBackButtonRect() const
{
    return D2D1::RectF(58.0f, 716.0f, 244.0f, 768.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::ArchiveTabRect(int index) const
{
    const float x = 410.0f + static_cast<float>(index) * 160.0f;
    return D2D1::RectF(x, 116.0f, x + 144.0f, 166.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::BriefingStartButtonRect() const
{
    return D2D1::RectF(786.0f, 724.0f, 1012.0f, 778.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::BriefingBackButtonRect() const
{
    return D2D1::RectF(268.0f, 724.0f, 486.0f, 778.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::BriefingShopButtonRect() const
{
    return D2D1::RectF(528.0f, 724.0f, 746.0f, 778.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::BriefingDifficultyRect(int index) const
{
    const float x = 692.0f + static_cast<float>(index) * 152.0f;
    return D2D1::RectF(x, 646.0f, x + 112.0f, 696.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::BriefingLoadoutSlotRect(int index) const
{
    const float x = 604.0f + static_cast<float>(index) * 112.0f;
    return D2D1::RectF(x, 236.0f, x + 96.0f, 358.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::ShopBackButtonRect() const
{
    return D2D1::RectF(58.0f, 716.0f, 244.0f, 768.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::ShopUnitRect(int index) const
{
    const int col = index % 4;
    const int row = index / 4;
    const float x = 58.0f + static_cast<float>(col) * 306.0f;
    const float y = 154.0f + static_cast<float>(row) * 132.0f;
    return D2D1::RectF(x, y, x + 286.0f, y + 114.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::SpeedDownButtonRect() const
{
    return D2D1::RectF(1006.0f, 720.0f, 1058.0f, 766.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::SpeedUpButtonRect() const
{
    return D2D1::RectF(1190.0f, 720.0f, 1242.0f, 766.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::ResultRetryButtonRect() const
{
    return D2D1::RectF(418.0f, 558.0f, 560.0f, 608.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::ResultNextButtonRect() const
{
    return D2D1::RectF(570.0f, 558.0f, 712.0f, 608.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::ResultMenuButtonRect() const
{
    return D2D1::RectF(722.0f, 558.0f, 864.0f, 608.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::WalletButtonRect() const
{
    return D2D1::RectF(664.0f, 628.0f, 808.0f, 694.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::CannonButtonRect() const
{
    return D2D1::RectF(664.0f, 708.0f, 808.0f, 774.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::MessageToastRect() const
{
    // 메시지는 전투 카드와 브리핑 본문을 가리지 않도록 화면 상단의 얇은 토스트로 고정한다.
    if (m_screen == GameScreen::Briefing)
    {
        return D2D1::RectF(604.0f, 100.0f, 1196.0f, 130.0f);
    }

    float y = (m_screen == GameScreen::Playing || m_screen == GameScreen::Result) ? 120.0f : 100.0f;
    if ((m_screen == GameScreen::Playing || m_screen == GameScreen::Result) && FindBossUnit())
    {
        // 보스 체력바는 104~142 영역을 사용한다. 보스전 로그는 바로 아래로 내려 겹침을 피한다.
        y = 152.0f;
    }
    return D2D1::RectF(392.0f, y, 888.0f, y + 34.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::PauseButtonRect() const
{
    return D2D1::RectF(1006.0f, 658.0f, 1118.0f, 704.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::RestartButtonRect() const
{
    return D2D1::RectF(1130.0f, 658.0f, 1242.0f, 704.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeResumeButtonRect() const
{
    return D2D1::RectF(332.0f, 218.0f, 608.0f, 268.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeShakeButtonRect() const
{
    return D2D1::RectF(332.0f, 282.0f, 608.0f, 332.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeSpeedDownButtonRect() const
{
    return D2D1::RectF(332.0f, 394.0f, 386.0f, 444.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeSpeedUpButtonRect() const
{
    return D2D1::RectF(554.0f, 394.0f, 608.0f, 444.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeStoryButtonRect() const
{
    return D2D1::RectF(332.0f, 480.0f, 608.0f, 530.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeSfxSliderRect() const
{
    return D2D1::RectF(682.0f, 264.0f, 940.0f, 284.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeUiSliderRect() const
{
    return D2D1::RectF(682.0f, 356.0f, 940.0f, 376.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeBgmSliderRect() const
{
    return D2D1::RectF(682.0f, 448.0f, 940.0f, 468.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeAudioResetButtonRect() const
{
    return D2D1::RectF(700.0f, 526.0f, 922.0f, 574.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeStageButtonRect() const
{
    return D2D1::RectF(332.0f, 548.0f, 608.0f, 598.0f);
}

D2D1_RECT_F SpaceDefenceGameImpl::EscapeQuitButtonRect() const
{
    return D2D1::RectF(332.0f, 616.0f, 608.0f, 666.0f);
}
