#include "PawlineGameImpl.h"

// Stable UI hit rectangles shared by input and rendering.
D2D1_RECT_F PawlineGameImpl::CardRect(int index) const
{
    const float x = 22.0f + static_cast<float>(index) * 126.0f;
    return D2D1::RectF(x, 628.0f, x + 118.0f, 774.0f);
}

D2D1_RECT_F PawlineGameImpl::TitleStartButtonRect() const
{
    return D2D1::RectF(492.0f, 514.0f, 788.0f, 574.0f);
}

D2D1_RECT_F PawlineGameImpl::TitleOptionsButtonRect() const
{
    return D2D1::RectF(492.0f, 586.0f, 788.0f, 646.0f);
}

D2D1_RECT_F PawlineGameImpl::TitleQuitButtonRect() const
{
    return D2D1::RectF(492.0f, 658.0f, 788.0f, 718.0f);
}

D2D1_RECT_F PawlineGameImpl::OptionsShakeButtonRect() const
{
    return D2D1::RectF(490.0f, 330.0f, 790.0f, 386.0f);
}

D2D1_RECT_F PawlineGameImpl::OptionsSpeedDownButtonRect() const
{
    return D2D1::RectF(490.0f, 430.0f, 544.0f, 486.0f);
}

D2D1_RECT_F PawlineGameImpl::OptionsSpeedUpButtonRect() const
{
    return D2D1::RectF(736.0f, 430.0f, 790.0f, 486.0f);
}

D2D1_RECT_F PawlineGameImpl::OptionsViewDownButtonRect() const
{
    return D2D1::RectF(490.0f, 540.0f, 544.0f, 596.0f);
}

D2D1_RECT_F PawlineGameImpl::OptionsViewUpButtonRect() const
{
    return D2D1::RectF(736.0f, 540.0f, 790.0f, 596.0f);
}

D2D1_RECT_F PawlineGameImpl::OptionsViewResetButtonRect() const
{
    return D2D1::RectF(552.0f, 604.0f, 728.0f, 650.0f);
}

D2D1_RECT_F PawlineGameImpl::OptionsBackButtonRect() const
{
    return D2D1::RectF(492.0f, 678.0f, 788.0f, 738.0f);
}

D2D1_RECT_F PawlineGameImpl::MenuStageRect(int index) const
{
    const int col = index % 5;
    const int row = index / 5;
    const float x = 48.0f + static_cast<float>(col) * 240.0f;
    const float y = 138.0f + static_cast<float>(row) * 94.0f;
    return D2D1::RectF(x, y, x + 216.0f, y + 82.0f);
}

D2D1_RECT_F PawlineGameImpl::MenuLoadoutSlotRect(int index) const
{
    const float x = 58.0f + static_cast<float>(index) * 116.0f;
    return D2D1::RectF(x, 358.0f, x + 102.0f, 476.0f);
}

D2D1_RECT_F PawlineGameImpl::RosterCardRect(int index) const
{
    const int col = index % 4;
    const int row = index / 4;
    const float x = 650.0f + static_cast<float>(col) * 145.0f;
    const float y = 348.0f + static_cast<float>(row) * 86.0f;
    return D2D1::RectF(x, y, x + 132.0f, y + 78.0f);
}

D2D1_RECT_F PawlineGameImpl::StartGameButtonRect() const
{
    return D2D1::RectF(936.0f, 720.0f, 1228.0f, 774.0f);
}

D2D1_RECT_F PawlineGameImpl::MenuShopButtonRect() const
{
    return D2D1::RectF(650.0f, 720.0f, 928.0f, 774.0f);
}

D2D1_RECT_F PawlineGameImpl::BriefingStartButtonRect() const
{
    return D2D1::RectF(840.0f, 690.0f, 1196.0f, 754.0f);
}

D2D1_RECT_F PawlineGameImpl::BriefingBackButtonRect() const
{
    return D2D1::RectF(84.0f, 690.0f, 302.0f, 754.0f);
}

D2D1_RECT_F PawlineGameImpl::BriefingShopButtonRect() const
{
    return D2D1::RectF(316.0f, 690.0f, 534.0f, 754.0f);
}

D2D1_RECT_F PawlineGameImpl::BriefingDifficultyRect(int index) const
{
    const float x = 604.0f + static_cast<float>(index) * 128.0f;
    return D2D1::RectF(x, 520.0f, x + 112.0f, 568.0f);
}

D2D1_RECT_F PawlineGameImpl::ShopBackButtonRect() const
{
    return D2D1::RectF(58.0f, 716.0f, 244.0f, 768.0f);
}

D2D1_RECT_F PawlineGameImpl::ShopUnitRect(int index) const
{
    const int col = index % 4;
    const int row = index / 4;
    const float x = 58.0f + static_cast<float>(col) * 306.0f;
    const float y = 154.0f + static_cast<float>(row) * 132.0f;
    return D2D1::RectF(x, y, x + 286.0f, y + 114.0f);
}

D2D1_RECT_F PawlineGameImpl::SpeedDownButtonRect() const
{
    return D2D1::RectF(1006.0f, 720.0f, 1058.0f, 766.0f);
}

D2D1_RECT_F PawlineGameImpl::SpeedUpButtonRect() const
{
    return D2D1::RectF(1190.0f, 720.0f, 1242.0f, 766.0f);
}

D2D1_RECT_F PawlineGameImpl::ResultRetryButtonRect() const
{
    return D2D1::RectF(418.0f, 470.0f, 560.0f, 520.0f);
}

D2D1_RECT_F PawlineGameImpl::ResultNextButtonRect() const
{
    return D2D1::RectF(570.0f, 470.0f, 712.0f, 520.0f);
}

D2D1_RECT_F PawlineGameImpl::ResultMenuButtonRect() const
{
    return D2D1::RectF(722.0f, 470.0f, 864.0f, 520.0f);
}

D2D1_RECT_F PawlineGameImpl::WalletButtonRect() const
{
    return D2D1::RectF(664.0f, 628.0f, 808.0f, 694.0f);
}

D2D1_RECT_F PawlineGameImpl::CannonButtonRect() const
{
    return D2D1::RectF(664.0f, 708.0f, 808.0f, 774.0f);
}

D2D1_RECT_F PawlineGameImpl::PauseButtonRect() const
{
    return D2D1::RectF(1006.0f, 658.0f, 1118.0f, 704.0f);
}

D2D1_RECT_F PawlineGameImpl::RestartButtonRect() const
{
    return D2D1::RectF(1130.0f, 658.0f, 1242.0f, 704.0f);
}

D2D1_RECT_F PawlineGameImpl::EscapeResumeButtonRect() const
{
    return D2D1::RectF(492.0f, 238.0f, 788.0f, 292.0f);
}

D2D1_RECT_F PawlineGameImpl::EscapeShakeButtonRect() const
{
    return D2D1::RectF(492.0f, 318.0f, 788.0f, 372.0f);
}

D2D1_RECT_F PawlineGameImpl::EscapeSpeedDownButtonRect() const
{
    return D2D1::RectF(492.0f, 426.0f, 546.0f, 480.0f);
}

D2D1_RECT_F PawlineGameImpl::EscapeSpeedUpButtonRect() const
{
    return D2D1::RectF(734.0f, 426.0f, 788.0f, 480.0f);
}

D2D1_RECT_F PawlineGameImpl::EscapeStageButtonRect() const
{
    return D2D1::RectF(492.0f, 524.0f, 788.0f, 578.0f);
}

D2D1_RECT_F PawlineGameImpl::EscapeQuitButtonRect() const
{
    return D2D1::RectF(492.0f, 596.0f, 788.0f, 650.0f);
}
