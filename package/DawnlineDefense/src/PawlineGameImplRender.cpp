#include "PawlineGameImpl.h"

namespace
{
D2D1_RECT_F OffsetRectF(D2D1_RECT_F rect, float dx, float dy)
{
    return D2D1::RectF(rect.left + dx, rect.top + dy, rect.right + dx, rect.bottom + dy);
}

D2D1_RECT_F InflateRectF(D2D1_RECT_F rect, float dx, float dy)
{
    return D2D1::RectF(rect.left - dx, rect.top - dy, rect.right + dx, rect.bottom + dy);
}

D2D1_RECT_F InsetRectF(D2D1_RECT_F rect, float dx, float dy)
{
    return D2D1::RectF(rect.left + dx, rect.top + dy, rect.right - dx, rect.bottom - dy);
}

wchar_t PixelUpper(wchar_t c)
{
    if (c >= L'a' && c <= L'z')
    {
        return static_cast<wchar_t>(c - L'a' + L'A');
    }
    return c;
}

std::array<std::wstring, 7> PixelGlyph(wchar_t c)
{
    switch (PixelUpper(c))
    {
    case L'A': return {L"01110", L"10001", L"10001", L"11111", L"10001", L"10001", L"10001"};
    case L'B': return {L"11110", L"10001", L"10001", L"11110", L"10001", L"10001", L"11110"};
    case L'C': return {L"01111", L"10000", L"10000", L"10000", L"10000", L"10000", L"01111"};
    case L'D': return {L"11110", L"10001", L"10001", L"10001", L"10001", L"10001", L"11110"};
    case L'E': return {L"11111", L"10000", L"10000", L"11110", L"10000", L"10000", L"11111"};
    case L'F': return {L"11111", L"10000", L"10000", L"11110", L"10000", L"10000", L"10000"};
    case L'G': return {L"01111", L"10000", L"10000", L"10011", L"10001", L"10001", L"01111"};
    case L'H': return {L"10001", L"10001", L"10001", L"11111", L"10001", L"10001", L"10001"};
    case L'I': return {L"11111", L"00100", L"00100", L"00100", L"00100", L"00100", L"11111"};
    case L'J': return {L"00111", L"00010", L"00010", L"00010", L"10010", L"10010", L"01100"};
    case L'K': return {L"10001", L"10010", L"10100", L"11000", L"10100", L"10010", L"10001"};
    case L'L': return {L"10000", L"10000", L"10000", L"10000", L"10000", L"10000", L"11111"};
    case L'M': return {L"10001", L"11011", L"10101", L"10101", L"10001", L"10001", L"10001"};
    case L'N': return {L"10001", L"11001", L"10101", L"10011", L"10001", L"10001", L"10001"};
    case L'O': return {L"01110", L"10001", L"10001", L"10001", L"10001", L"10001", L"01110"};
    case L'P': return {L"11110", L"10001", L"10001", L"11110", L"10000", L"10000", L"10000"};
    case L'Q': return {L"01110", L"10001", L"10001", L"10001", L"10101", L"10010", L"01101"};
    case L'R': return {L"11110", L"10001", L"10001", L"11110", L"10100", L"10010", L"10001"};
    case L'S': return {L"01111", L"10000", L"10000", L"01110", L"00001", L"00001", L"11110"};
    case L'T': return {L"11111", L"00100", L"00100", L"00100", L"00100", L"00100", L"00100"};
    case L'U': return {L"10001", L"10001", L"10001", L"10001", L"10001", L"10001", L"01110"};
    case L'V': return {L"10001", L"10001", L"10001", L"10001", L"10001", L"01010", L"00100"};
    case L'W': return {L"10001", L"10001", L"10001", L"10101", L"10101", L"10101", L"01010"};
    case L'X': return {L"10001", L"10001", L"01010", L"00100", L"01010", L"10001", L"10001"};
    case L'Y': return {L"10001", L"10001", L"01010", L"00100", L"00100", L"00100", L"00100"};
    case L'Z': return {L"11111", L"00001", L"00010", L"00100", L"01000", L"10000", L"11111"};
    case L'0': return {L"01110", L"10001", L"10011", L"10101", L"11001", L"10001", L"01110"};
    case L'1': return {L"00100", L"01100", L"00100", L"00100", L"00100", L"00100", L"01110"};
    case L'2': return {L"01110", L"10001", L"00001", L"00010", L"00100", L"01000", L"11111"};
    case L'3': return {L"11110", L"00001", L"00001", L"01110", L"00001", L"00001", L"11110"};
    case L'4': return {L"00010", L"00110", L"01010", L"10010", L"11111", L"00010", L"00010"};
    case L'5': return {L"11111", L"10000", L"10000", L"11110", L"00001", L"00001", L"11110"};
    case L'6': return {L"01110", L"10000", L"10000", L"11110", L"10001", L"10001", L"01110"};
    case L'7': return {L"11111", L"00001", L"00010", L"00100", L"01000", L"01000", L"01000"};
    case L'8': return {L"01110", L"10001", L"10001", L"01110", L"10001", L"10001", L"01110"};
    case L'9': return {L"01110", L"10001", L"10001", L"01111", L"00001", L"00001", L"01110"};
    case L'-': return {L"00000", L"00000", L"00000", L"11111", L"00000", L"00000", L"00000"};
    case L'+': return {L"00000", L"00100", L"00100", L"11111", L"00100", L"00100", L"00000"};
    case L'/': return {L"00001", L"00001", L"00010", L"00100", L"01000", L"10000", L"10000"};
    case L'.': return {L"00000", L"00000", L"00000", L"00000", L"00000", L"01100", L"01100"};
    case L':': return {L"00000", L"01100", L"01100", L"00000", L"01100", L"01100", L"00000"};
    case L'%': return {L"11001", L"11010", L"00100", L"01000", L"10110", L"00110", L"00000"};
    case L'!': return {L"00100", L"00100", L"00100", L"00100", L"00100", L"00000", L"00100"};
    default: return {L"00000", L"00000", L"00000", L"00000", L"00000", L"00000", L"00000"};
    }
}

bool PixelHasInk(wchar_t c)
{
    if (c == L' ')
    {
        return true;
    }
    const auto rows = PixelGlyph(c);
    for (const std::wstring& row : rows)
    {
        if (row.find(L'1') != std::wstring::npos)
        {
            return true;
        }
    }
    return false;
}

float Smooth01(float t)
{
    t = Clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

struct ImageVfxSheetSpec
{
    int columns = 1;
    int rows = 8;
    float frameWidth = 64.0f;
    float frameHeight = 64.0f;
};

ImageVfxSheetSpec ImageVfxSpec(ImageVfxKind kind)
{
    // 렌더러가 VFX 시트의 행/열 정보를 한 곳에서 해석하도록 만든다.
    // DrawImageVfxFrame과 DrawImageVfxSprites가 같은 기준을 써야 프레임이 밀리지 않는다.
    switch (kind)
    {
    case ImageVfxKind::Heal:
    case ImageVfxKind::HealSoft:
        return {4, 4, 128.0f, 128.0f};
    case ImageVfxKind::Fire:
    case ImageVfxKind::Water:
        return {4, 4, 64.0f, 64.0f};
    case ImageVfxKind::Dark:
        return {16, 1, 48.0f, 64.0f};
    case ImageVfxKind::Acid:
        return {16, 1, 32.0f, 32.0f};
    case ImageVfxKind::Holy:
        return {16, 1, 48.0f, 48.0f};
    case ImageVfxKind::Ice:
        return {5, 4, 192.0f, 192.0f};
    case ImageVfxKind::Thunder:
        return {13, 1, 64.0f, 64.0f};
    case ImageVfxKind::Smoke:
        return {13, 1, 64.0f, 64.0f};
    case ImageVfxKind::Earth:
        return {7, 1, 48.0f, 48.0f};
    case ImageVfxKind::Wood:
        return {7, 1, 32.0f, 32.0f};
    case ImageVfxKind::HitFlash:
        return {7, 1, 48.0f, 48.0f};
    case ImageVfxKind::Wind:
        return {18, 1, 32.0f, 32.0f};
    case ImageVfxKind::WindHit:
        return {3, 1, 32.0f, 64.0f};
    case ImageVfxKind::Thrust:
        return {1, 3, 64.0f, 64.0f};
    case ImageVfxKind::Smear:
        return {5, 1, 48.0f, 48.0f};
    case ImageVfxKind::Explosion:
        return {18, 1, 48.0f, 48.0f};
    case ImageVfxKind::FireBreath:
        return {8, 3, 48.0f, 48.0f};
    case ImageVfxKind::MagicMirror:
        return {1, 5, 128.0f, 128.0f};
    case ImageVfxKind::EnergyImpact:
        return {1, 8, 128.0f, 128.0f};
    case ImageVfxKind::Crystal:
        return {6, 1, 128.0f, 128.0f};
    case ImageVfxKind::AirBurst:
        return {3, 3, 48.0f, 48.0f};
    case ImageVfxKind::ThunderSplash:
        return {14, 1, 48.0f, 48.0f};
    case ImageVfxKind::WaterBallImpact:
        return {4, 4, 64.0f, 64.0f};
    case ImageVfxKind::SmokeDust:
        return {15, 1, 48.0f, 64.0f};
    default:
        return {};
    }
}

int ImageVfxFrameCount(ImageVfxKind kind)
{
    const ImageVfxSheetSpec spec = ImageVfxSpec(kind);
    return spec.columns * spec.rows;
}

float UnitMoveStride(const Unit& unit)
{
    return unit.animState == UnitAnimState::Move ? std::sin(unit.walkCycle + unit.shakePhase * 0.15f) : 0.0f;
}

float UnitMoveStep(const Unit& unit)
{
    return std::abs(UnitMoveStride(unit));
}

float UnitHitPose(const Unit& unit)
{
    return std::max(Clamp01(unit.hitFlash / 0.12f), Clamp01(unit.knockbackTimer / 0.34f));
}

float UnitDeathPose(const Unit& unit)
{
    if (unit.animState != UnitAnimState::Death && unit.alive)
    {
        return 0.0f;
    }
    return Smooth01(unit.stateTime / 0.55f);
}

std::wstring UnitRoleLabel(PlayerUnit unit, const UnitStats& stats)
{
    // 카드와 브리핑에서 유닛 역할을 같은 기준으로 보여주기 위한 분류다.
    // 실제 수치 데이터는 GameData.cpp에 두고, 화면용 역할명만 여기서 해석한다.
    switch (unit)
    {
    case PlayerUnit::Box:
    case PlayerUnit::Frost:
    case PlayerUnit::Titan:
        return L"방어";
    case PlayerUnit::Dash:
    case PlayerUnit::Comet:
        return L"돌파";
    case PlayerUnit::Bell:
    case PlayerUnit::Mint:
        return L"지원";
    case PlayerUnit::Drill:
    case PlayerUnit::Solar:
        return L"공성";
    case PlayerUnit::Spark:
    case PlayerUnit::Orbit:
    case PlayerUnit::Prism:
    case PlayerUnit::Nebula:
        return L"원격";
    case PlayerUnit::Paw:
    default:
        return stats.ranged ? L"원격" : L"근접";
    }
}

D2D1_COLOR_F UnitRoleColor(PlayerUnit unit, const UnitStats& stats)
{
    // 역할 배지는 작은 UI라서 유닛 고유색보다 더 또렷한 계열색을 쓴다.
    switch (unit)
    {
    case PlayerUnit::Box:
    case PlayerUnit::Frost:
    case PlayerUnit::Titan:
        return D2D1::ColorF(0x65B8FF);
    case PlayerUnit::Dash:
    case PlayerUnit::Comet:
        return D2D1::ColorF(0x62DD88);
    case PlayerUnit::Bell:
    case PlayerUnit::Mint:
        return D2D1::ColorF(0xF6FF83);
    case PlayerUnit::Drill:
    case PlayerUnit::Solar:
        return D2D1::ColorF(0xFFB347);
    default:
        return stats.ranged ? D2D1::ColorF(0xBA7BFF) : stats.accent;
    }
}

float MissingHpRatio(float hp, float maxHp)
{
    return 1.0f - Clamp01(hp / std::max(1.0f, maxHp));
}
}

// Direct2D drawing, including the shader-style procedural VFX pass.
void PawlineGameImpl::Render()
{
    HRESULT hr = CreateDeviceResources();
    if (FAILED(hr))
    {
        return;
    }

    m_renderTarget->BeginDraw();
    m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_renderTarget->Clear(D2D1::ColorF(0x071017));
    UpdateViewMetrics();
    SetViewTransform();

    if (m_screen == GameScreen::Title)
    {
        DrawTitle();
        DrawUiPulses();
        DrawSceneTransition();
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    if (m_screen == GameScreen::StoryIntro)
    {
        DrawStoryCrawl();
        DrawUiPulses();
        DrawSceneTransition();
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    if (m_screen == GameScreen::Ending)
    {
        DrawEndingScene();
        DrawUiPulses();
        DrawSceneTransition();
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    if (m_screen == GameScreen::Options)
    {
        DrawOptions();
        DrawUiPulses();
        DrawMessage();
        DrawSceneTransition();
        if (m_escapeMenuOpen)
        {
            DrawEscapeMenuClean();
        }
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    if (m_screen == GameScreen::Menu)
    {
        DrawMenu();
        DrawMessage();
        DrawUiPulses();
        DrawSceneTransition();
        if (m_escapeMenuOpen)
        {
            DrawEscapeMenuClean();
        }
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    if (m_screen == GameScreen::Archive)
    {
        DrawArchive();
        DrawMessage();
        DrawUiPulses();
        DrawSceneTransition();
        if (m_escapeMenuOpen)
        {
            DrawEscapeMenuClean();
        }
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    if (m_screen == GameScreen::Briefing)
    {
        DrawBriefing();
        DrawMessage();
        DrawUiPulses();
        DrawSceneTransition();
        if (m_escapeMenuOpen)
        {
            DrawEscapeMenuClean();
        }
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    if (m_screen == GameScreen::Shop)
    {
        DrawShop();
        DrawRings();
        DrawParticles();
        DrawFloatTexts();
        DrawMessage();
        DrawUiPulses();
        DrawSceneTransition();
        if (m_escapeMenuOpen)
        {
            DrawEscapeMenuClean();
        }
        hr = m_renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        return;
    }

    // Draw order is back-to-front: arena, actors, transient VFX, UI, overlays.
    // That keeps combat effects vivid without covering readable controls.
    SetViewTransform(m_cameraX, true);
    DrawArena();
    DrawBases();
    DrawTelegraphs();
    DrawUnitLighting();
    DrawUnits();
    DrawProjectiles();
    DrawBeams();
    DrawRings();
    DrawImageVfxSprites();
    DrawSparkLines();
    DrawParticles();
    DrawFloatTexts();
    SetViewTransform();

    DrawShaderPostProcess();
    DrawStageGimmickOverlay();
    DrawHeader();
    DrawCameraHud();
    DrawBossPresentation();
    DrawTutorialTips();
    DrawShowcaseBadge();
    DrawDebugBadge();
    DrawCommandBar();
    DrawFullscreenFrameExtensions();
    DrawUiPulses();
    DrawMessage();
    DrawOverlay();
    DrawScreenFlash();
    if (m_screen == GameScreen::Result)
    {
        DrawResultScreen();
    }
    DrawSceneTransition();
    if (m_escapeMenuOpen)
    {
        DrawEscapeMenuClean();
    }

    hr = m_renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }
}

void PawlineGameImpl::SetColor(D2D1_COLOR_F color)
{
    m_brush->SetColor(color);
}

void PawlineGameImpl::FillRect(D2D1_RECT_F rect, D2D1_COLOR_F color)
{
    SetColor(color);
    m_renderTarget->FillRectangle(rect, m_brush.Get());
}

void PawlineGameImpl::StrokeRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawRectangle(rect, m_brush.Get(), width);
}

D2D1_RECT_F PawlineGameImpl::FullViewportRect() const
{
    if (!m_renderTarget)
    {
        return D2D1::RectF(0.0f, 0.0f, kWidth, kHeight);
    }

    const D2D1_SIZE_F size = m_renderTarget->GetSize();
    return D2D1::RectF(0.0f, 0.0f, size.width, size.height);
}

void PawlineGameImpl::FillViewport(D2D1_COLOR_F color)
{
    if (!m_renderTarget)
    {
        return;
    }

    m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    FillRect(FullViewportRect(), color);
    SetViewTransform();
}

void PawlineGameImpl::FillRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color)
{
    SetColor(color);
    m_renderTarget->FillRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), m_brush.Get());
}

void PawlineGameImpl::StrokeRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), m_brush.Get(), width);
}

void PawlineGameImpl::FillEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color)
{
    SetColor(color);
    m_renderTarget->FillEllipse(Ellipse(pos, rx, ry), m_brush.Get());
}

void PawlineGameImpl::StrokeEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawEllipse(Ellipse(pos, rx, ry), m_brush.Get(), width);
}

void PawlineGameImpl::DrawLine(Vec2 a, Vec2 b, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawLine(Point(a), Point(b), m_brush.Get(), width, m_roundStroke.Get());
}

void PawlineGameImpl::DrawBitmap(ID2D1Bitmap* bitmap, D2D1_RECT_F destination, float opacity, const D2D1_RECT_F* source)
{
    if (!bitmap)
    {
        return;
    }
    m_renderTarget->DrawBitmap(bitmap, destination, Clamp01(opacity), D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, source);
}

void PawlineGameImpl::DrawWeaponBitmap(ID2D1Bitmap* bitmap, Vec2 center, float width, float height, float angleDegrees, float opacity, bool flipX)
{
    if (!bitmap || opacity <= 0.001f || !m_renderTarget)
    {
        return;
    }

    D2D1_MATRIX_3X2_F previous = D2D1::Matrix3x2F::Identity();
    m_renderTarget->GetTransform(&previous);
    const D2D1_POINT_2F pivot = D2D1::Point2F(center.x, center.y);
    const D2D1_MATRIX_3X2_F flip = D2D1::Matrix3x2F::Scale(flipX ? -1.0f : 1.0f, 1.0f, pivot);
    const D2D1_MATRIX_3X2_F rotate = D2D1::Matrix3x2F::Rotation(angleDegrees, pivot);
    m_renderTarget->SetTransform(flip * rotate * previous);
    DrawBitmap(bitmap, D2D1::RectF(center.x - width * 0.5f, center.y - height * 0.5f, center.x + width * 0.5f, center.y + height * 0.5f), opacity);
    m_renderTarget->SetTransform(previous);
}

void PawlineGameImpl::DrawBitmapCover(ID2D1Bitmap* bitmap, D2D1_RECT_F area, float opacity, float time, float cameraX, float motionScale)
{
    if (!bitmap || !m_renderTarget)
    {
        return;
    }

    const D2D1_SIZE_F sourceSize = bitmap->GetSize();
    const float areaWidth = std::max(1.0f, area.right - area.left);
    const float areaHeight = std::max(1.0f, area.bottom - area.top);
    if (sourceSize.width <= 0.0f || sourceSize.height <= 0.0f)
    {
        DrawBitmap(bitmap, area, opacity);
        return;
    }

    // 원본 비율을 유지한 채 화면보다 살짝 크게 그려서, 넓은 화면에서도 이미지가 늘어나지 않게 한다.
    // 남는 바깥 영역은 클리핑하고, 시간과 카메라에 맞춰 천천히 움직여 살아 있는 우주 배경처럼 보이게 한다.
    constexpr float kBackdropZoom = 1.18f;
    const float scale = std::max(areaWidth / sourceSize.width, areaHeight / sourceSize.height) * kBackdropZoom;
    const float drawWidth = sourceSize.width * scale;
    const float drawHeight = sourceSize.height * scale;
    const float overflowX = std::max(0.0f, drawWidth - areaWidth);
    const float overflowY = std::max(0.0f, drawHeight - areaHeight);
    const float motion = std::max(0.0f, motionScale);
    const float panRangeX = std::min(0.48f, 0.18f + motion * 0.085f);
    const float panRangeY = std::min(0.44f, 0.16f + motion * 0.070f);
    const float panX = std::sin(time * (0.022f + motion * 0.042f) + cameraX * (0.00045f + motion * 0.00016f)) * overflowX * panRangeX;
    const float panY = std::cos(time * (0.017f + motion * 0.031f) + cameraX * (0.00030f + motion * 0.00011f)) * overflowY * panRangeY;
    const D2D1_RECT_F dest = D2D1::RectF(area.left + (areaWidth - drawWidth) * 0.5f + panX,
                                         area.top + (areaHeight - drawHeight) * 0.5f + panY,
                                         area.left + (areaWidth + drawWidth) * 0.5f + panX,
                                         area.top + (areaHeight + drawHeight) * 0.5f + panY);

    m_renderTarget->PushAxisAlignedClip(area, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    DrawBitmap(bitmap, dest, opacity);
    m_renderTarget->PopAxisAlignedClip();
}

void PawlineGameImpl::DrawString(const std::wstring& text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color)
{
    SetColor(color);
    m_renderTarget->DrawTextW(text.c_str(), static_cast<UINT32>(text.size()), format, rect, m_brush.Get());
}

void PawlineGameImpl::DrawString(const std::wstring& text, D2D1_RECT_F rect, const Microsoft::WRL::ComPtr<IDWriteTextFormat>& format, D2D1_COLOR_F color)
{
    DrawString(text, rect, format.Get(), color);
}

void PawlineGameImpl::DrawOutlinedString(const std::wstring& text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color, float outlineAlpha)
{
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, outlineAlpha);
    DrawString(text, OffsetRectF(rect, 2.0f, 2.0f), format, D2D1::ColorF(0x000000, outlineAlpha * 0.58f));
    DrawString(text, OffsetRectF(rect, -1.0f, 0.0f), format, ink);
    DrawString(text, OffsetRectF(rect, 1.0f, 0.0f), format, ink);
    DrawString(text, OffsetRectF(rect, 0.0f, -1.0f), format, ink);
    DrawString(text, OffsetRectF(rect, 0.0f, 1.0f), format, ink);
    DrawString(text, rect, format, color);
}

void PawlineGameImpl::DrawOutlinedString(const std::wstring& text, D2D1_RECT_F rect, const Microsoft::WRL::ComPtr<IDWriteTextFormat>& format, D2D1_COLOR_F color, float outlineAlpha)
{
    DrawOutlinedString(text, rect, format.Get(), color, outlineAlpha);
}

void PawlineGameImpl::DrawCartoonPanel(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F accent, bool hover)
{
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, 0.94f);
    FillRoundRect(OffsetRectF(rect, 4.0f, 5.0f), 8.0f, D2D1::ColorF(0x000000, 0.30f));
    FillRoundRect(rect, 8.0f, fill);
    DrawUiPanelAsset(rect, hover ? 1 : 0, hover ? 0.34f : 0.20f);
    FillRoundRect(D2D1::RectF(rect.left + 6.0f, rect.top + 6.0f, rect.right - 6.0f, rect.top + 18.0f), 6.0f, D2D1::ColorF(0xFFFFFF, hover ? 0.12f : 0.075f));
    StrokeRoundRect(rect, 8.0f, ink, hover ? 3.6f : 3.0f);
    StrokeRoundRect(InsetRectF(rect, 3.0f, 3.0f), 6.0f, D2D1::ColorF(accent.r, accent.g, accent.b, hover ? 0.72f : 0.45f), hover ? 2.0f : 1.2f);
}

void PawlineGameImpl::DrawBriefingPanel(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F accent, bool hover)
{
    // 브리핑 화면 전용 패널이다. 내부 큰 사각 프레임을 빼고, 외곽선과 얇은 광택만 남겨 답답함을 줄인다.
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, 0.94f);
    FillRoundRect(OffsetRectF(rect, 4.0f, 5.0f), 8.0f, D2D1::ColorF(0x000000, 0.28f));
    FillRoundRect(rect, 8.0f, fill);
    FillRoundRect(D2D1::RectF(rect.left + 8.0f, rect.top + 7.0f, rect.right - 8.0f, rect.top + 19.0f),
                  6.0f,
                  D2D1::ColorF(0xFFFFFF, hover ? 0.13f : 0.08f));
    FillRoundRect(D2D1::RectF(rect.left + 22.0f, rect.bottom - 19.0f, rect.right - 22.0f, rect.bottom - 13.0f),
                  3.0f,
                  D2D1::ColorF(accent.r, accent.g, accent.b, hover ? 0.20f : 0.12f));
    StrokeRoundRect(rect, 8.0f, ink, hover ? 3.4f : 2.9f);
    StrokeRoundRect(InsetRectF(rect, 2.0f, 2.0f), 7.0f, D2D1::ColorF(accent.r, accent.g, accent.b, hover ? 0.72f : 0.48f), hover ? 1.8f : 1.2f);
}

void PawlineGameImpl::DrawBriefingButton(D2D1_RECT_F rect, const std::wstring& label, bool enabled, D2D1_COLOR_F fill, bool active)
{
    // 브리핑 버튼은 일반 버튼보다 내부 장식을 줄여 선택 상태만 또렷하게 보이도록 그린다.
    const bool hover = Contains(rect, m_mouse);
    D2D1_COLOR_F actualFill = enabled ? fill : D2D1::ColorF(0x171D23);
    if ((hover || active) && enabled)
    {
        actualFill.r = std::min(1.0f, actualFill.r + (active ? 0.07f : 0.04f));
        actualFill.g = std::min(1.0f, actualFill.g + (active ? 0.07f : 0.04f));
        actualFill.b = std::min(1.0f, actualFill.b + (active ? 0.07f : 0.04f));
    }

    if ((hover || active) && enabled)
    {
        FillRoundRect(InflateRectF(rect, active ? 7.0f : 5.0f, active ? 7.0f : 5.0f), 10.0f, D2D1::ColorF(active ? 0xB8FF89 : 0x65B8FF, active ? 0.075f : 0.055f));
    }

    const D2D1_COLOR_F accent = enabled ? (active ? D2D1::ColorF(0xB8FF89) : (hover ? D2D1::ColorF(0x9EDFFF) : D2D1::ColorF(0x476779))) : D2D1::ColorF(0x2B333A);
    FillRoundRect(OffsetRectF(rect, 3.0f, 4.0f), 8.0f, D2D1::ColorF(0x000000, 0.25f));
    FillRoundRect(rect, 8.0f, actualFill);
    FillRoundRect(D2D1::RectF(rect.left + 8.0f, rect.top + 7.0f, rect.right - 8.0f, rect.top + 15.0f), 4.0f, D2D1::ColorF(0xFFFFFF, hover ? 0.14f : 0.085f));
    StrokeRoundRect(rect, 8.0f, D2D1::ColorF(0x061019, 0.94f), active ? 3.4f : 2.8f);
    StrokeRoundRect(InsetRectF(rect, 2.0f, 2.0f), 6.0f, D2D1::ColorF(accent.r, accent.g, accent.b, active ? 0.78f : 0.48f), active ? 1.7f : 1.1f);
    DrawPixelTextCentered(label, rect, 2.55f, enabled ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x7E8B94), enabled ? 1.0f : 0.72f);
}

float PawlineGameImpl::PixelTextWidth(const std::wstring& text, float cell) const
{
    float width = 0.0f;
    for (wchar_t raw : text)
    {
        const wchar_t c = PixelUpper(raw);
        width += c == L' ' ? cell * 3.0f : cell * 6.0f;
    }
    return std::max(0.0f, width - cell);
}

void PawlineGameImpl::DrawPixelText(const std::wstring& text, Vec2 pos, float cell, D2D1_COLOR_F color, float alpha, bool shadow)
{
    float cursor = pos.x;
    const float block = std::max(1.0f, cell - 0.8f);
    for (wchar_t raw : text)
    {
        const wchar_t c = PixelUpper(raw);
        if (c == L' ')
        {
            cursor += cell * 3.0f;
            continue;
        }

        const auto rows = PixelGlyph(c);
        for (int r = 0; r < 7; ++r)
        {
            for (int col = 0; col < 5; ++col)
            {
                if (rows[r][col] != L'1')
                {
                    continue;
                }

                const float x = cursor + static_cast<float>(col) * cell;
                const float y = pos.y + static_cast<float>(r) * cell;
                if (shadow)
                {
                    FillRoundRect(D2D1::RectF(x + cell * 0.32f, y + cell * 0.38f, x + cell * 0.32f + block, y + cell * 0.38f + block), cell * 0.16f, D2D1::ColorF(0x000000, 0.54f * alpha));
                    FillRoundRect(D2D1::RectF(x - cell * 0.16f, y - cell * 0.16f, x + block + cell * 0.16f, y + block + cell * 0.16f), cell * 0.16f, D2D1::ColorF(0x061019, 0.82f * alpha));
                }
                FillRoundRect(D2D1::RectF(x, y, x + block, y + block), cell * 0.16f, D2D1::ColorF(color.r, color.g, color.b, color.a * alpha));
            }
        }
        cursor += cell * 6.0f;
    }
}

void PawlineGameImpl::DrawPixelTextCentered(const std::wstring& text, D2D1_RECT_F rect, float cell, D2D1_COLOR_F color, float alpha)
{
    bool pixelReady = true;
    for (wchar_t c : text)
    {
        if (!PixelHasInk(c))
        {
            pixelReady = false;
            break;
        }
    }

    if (!pixelReady)
    {
        DrawOutlinedString(text, rect, m_buttonFormat, D2D1::ColorF(color.r, color.g, color.b, color.a * alpha), 0.70f * alpha);
        return;
    }

    const float width = PixelTextWidth(text, cell);
    const float height = cell * 7.0f;
    const Vec2 pos = {
        rect.left + ((rect.right - rect.left) - width) * 0.5f,
        rect.top + ((rect.bottom - rect.top) - height) * 0.5f};
    DrawPixelText(text, pos, cell, color, alpha, true);
}

Vec2 PawlineGameImpl::WorldToScreen(Vec2 pos) const
{
    return {pos.x - m_cameraX, pos.y};
}

D2D1_RECT_F PawlineGameImpl::WorldRect(float left, float top, float right, float bottom) const
{
    return D2D1::RectF(left - m_cameraX, top, right - m_cameraX, bottom);
}

void PawlineGameImpl::DrawVfxAtlasTile(int tileX, int tileY, Vec2 center, float size, float opacity)
{
    if (!m_vfxAtlas)
    {
        return;
    }
    const D2D1_RECT_F source = D2D1::RectF(static_cast<float>(tileX * 128), static_cast<float>(tileY * 128),
                                           static_cast<float>(tileX * 128 + 128), static_cast<float>(tileY * 128 + 128));
    const D2D1_RECT_F destination = D2D1::RectF(center.x - size * 0.5f, center.y - size * 0.5f,
                                                center.x + size * 0.5f, center.y + size * 0.5f);
    DrawBitmap(m_vfxAtlas.Get(), destination, opacity, &source);
}

void PawlineGameImpl::DrawImageVfxFrame(ImageVfxKind kind, int frame, Vec2 center, float size, float opacity)
{
    ID2D1Bitmap* sheet = nullptr;
    const ImageVfxSheetSpec spec = ImageVfxSpec(kind);

    switch (kind)
    {
    case ImageVfxKind::Slash:
        sheet = m_slashEffectSheet.Get();
        break;
    case ImageVfxKind::EnemySlash:
        sheet = m_enemySlashEffectSheet ? m_enemySlashEffectSheet.Get() : m_slashEffectSheet.Get();
        break;
    case ImageVfxKind::Heal:
        sheet = m_healEffectSheet.Get();
        break;
    case ImageVfxKind::HealSoft:
        sheet = m_healSoftEffectSheet ? m_healSoftEffectSheet.Get() : m_healEffectSheet.Get();
        break;
    case ImageVfxKind::Fire:
        sheet = m_fireEffectSheet.Get();
        break;
    case ImageVfxKind::Ice:
        sheet = m_iceEffectSheet.Get();
        break;
    case ImageVfxKind::Thunder:
        sheet = m_thunderEffectSheet.Get();
        break;
    case ImageVfxKind::Water:
        sheet = m_waterEffectSheet.Get();
        break;
    case ImageVfxKind::Dark:
        sheet = m_darkEffectSheet.Get();
        break;
    case ImageVfxKind::Acid:
        sheet = m_acidEffectSheet.Get();
        break;
    case ImageVfxKind::Earth:
        sheet = m_earthEffectSheet.Get();
        break;
    case ImageVfxKind::Smoke:
        sheet = m_smokeEffectSheet.Get();
        break;
    case ImageVfxKind::Holy:
        sheet = m_holyEffectSheet.Get();
        break;
    case ImageVfxKind::Wind:
        sheet = m_windEffectSheet.Get();
        break;
    case ImageVfxKind::WindHit:
        sheet = m_windHitEffectSheet.Get();
        break;
    case ImageVfxKind::Wood:
        sheet = m_woodEffectSheet.Get();
        break;
    case ImageVfxKind::HitFlash:
        sheet = m_hitFlashEffectSheet.Get();
        break;
    case ImageVfxKind::Smear:
        sheet = m_smearEffectSheet.Get();
        break;
    case ImageVfxKind::Thrust:
        sheet = m_thrustEffectSheet.Get();
        break;
    case ImageVfxKind::Explosion:
        sheet = m_explosionEffectSheet.Get();
        break;
    case ImageVfxKind::FireBreath:
        sheet = m_fireBreathEffectSheet.Get();
        break;
    case ImageVfxKind::MagicMirror:
        sheet = m_magicMirrorEffectSheet.Get();
        break;
    case ImageVfxKind::EnergyImpact:
        sheet = m_energyImpactEffectSheet.Get();
        break;
    case ImageVfxKind::Crystal:
        sheet = m_crystalEffectSheet.Get();
        break;
    case ImageVfxKind::AirBurst:
        sheet = m_airBurstEffectSheet.Get();
        break;
    case ImageVfxKind::ThunderSplash:
        sheet = m_thunderSplashEffectSheet.Get();
        break;
    case ImageVfxKind::WaterBallImpact:
        sheet = m_waterBallImpactEffectSheet.Get();
        break;
    case ImageVfxKind::SmokeDust:
        sheet = m_smokeDustEffectSheet.Get();
        break;
    }

    if (!sheet)
    {
        return;
    }

    const int frameCount = spec.columns * spec.rows;
    const int safeFrame = std::clamp(frame, 0, frameCount - 1);
    const int x = safeFrame % spec.columns;
    const int y = safeFrame / spec.columns;
    const D2D1_RECT_F source = D2D1::RectF(static_cast<float>(x) * spec.frameWidth, static_cast<float>(y) * spec.frameHeight,
                                           static_cast<float>(x + 1) * spec.frameWidth, static_cast<float>(y + 1) * spec.frameHeight);
    const float aspect = spec.frameWidth / spec.frameHeight;
    const D2D1_RECT_F destination = D2D1::RectF(center.x - size * aspect * 0.5f, center.y - size * 0.5f,
                                                center.x + size * aspect * 0.5f, center.y + size * 0.5f);
    DrawBitmap(sheet, destination, std::min(1.0f, opacity * 1.58f), &source);
}

void PawlineGameImpl::DrawImageVfxSprites()
{
    for (const ImageVfx& effect : m_imageVfx)
    {
        const float alpha = Clamp01(effect.life / effect.maxLife);
        const float progress = 1.0f - alpha;
        const bool heal = effect.kind == ImageVfxKind::Heal || effect.kind == ImageVfxKind::HealSoft;
        const bool smoke = effect.kind == ImageVfxKind::Smoke || effect.kind == ImageVfxKind::SmokeDust;
        const int frameCount = ImageVfxFrameCount(effect.kind);
        const int frame = std::clamp(static_cast<int>((progress + effect.frameOffset) * static_cast<float>(frameCount)), 0, frameCount - 1);
        const float pulse = std::sin(progress * kPi);
        const float pop = 1.0f + pulse * (heal ? 0.10f : 0.20f);
        const float lightAlpha = smoke ? 0.055f : (heal ? 0.11f : 0.18f);

        // 실제 셰이더 대신 Direct2D 레이어를 여러 겹 합성해 로컬 블룸처럼 보이게 한다.
        FillEllipse(effect.pos, effect.size * (0.78f + pulse * 0.28f), effect.size * (0.42f + pulse * 0.18f),
                    D2D1::ColorF(effect.color.r, effect.color.g, effect.color.b, effect.color.a * alpha * lightAlpha));
        FillEllipse(effect.pos, effect.size * (0.48f + pulse * 0.16f), effect.size * (0.28f + pulse * 0.10f),
                    D2D1::ColorF(effect.color.r, effect.color.g, effect.color.b, effect.color.a * alpha * lightAlpha * 1.35f));
        if (!smoke)
        {
            DrawImageVfxFrame(effect.kind, frame, effect.pos, effect.size * (pop + 0.18f), effect.color.a * alpha * 0.22f);
        }
        DrawImageVfxFrame(effect.kind, frame, effect.pos, effect.size * pop, effect.color.a * alpha);
        if (!smoke)
        {
            FillEllipse({effect.pos.x, effect.pos.y - effect.size * 0.06f}, effect.size * 0.18f, effect.size * 0.10f,
                        D2D1::ColorF(0xFFFFFF, effect.color.a * alpha * 0.08f));
        }
    }
}

void PawlineGameImpl::DrawUiPanelAsset(D2D1_RECT_F rect, int tileIndex, float opacity)
{
    if (!m_uiAtlas)
    {
        return;
    }
    const int safeTile = std::clamp(tileIndex, 0, 3);
    const int tileX = safeTile % 2;
    const int tileY = safeTile / 2;
    const D2D1_RECT_F source = D2D1::RectF(static_cast<float>(tileX * 256), static_cast<float>(tileY * 128),
                                           static_cast<float>(tileX * 256 + 256), static_cast<float>(tileY * 128 + 128));
    DrawBitmap(m_uiAtlas.Get(), rect, opacity, &source);
}

void PawlineGameImpl::DrawPlayerIcon(PlayerUnit type, Vec2 center, float scale, bool enabled)
{
    const UnitStats stats = GetPlayerStats(type);
    D2D1_COLOR_F body = enabled ? stats.color : D2D1::ColorF(0x68737A);
    D2D1_COLOR_F accent = enabled ? stats.accent : D2D1::ColorF(0x8A969D);
    const float r = 20.0f * scale;

    FillEllipse(center, r, r, body);
    StrokeEllipse(center, r, r, accent, 2.0f * scale);
    FillEllipse({center.x - r * 0.34f, center.y - r * 0.12f}, 2.8f * scale, 4.2f * scale, D2D1::ColorF(0x071017));
    FillEllipse({center.x + r * 0.34f, center.y - r * 0.12f}, 2.8f * scale, 4.2f * scale, D2D1::ColorF(0x071017));
    DrawLine({center.x - r * 0.24f, center.y + r * 0.34f}, {center.x + r * 0.24f, center.y + r * 0.34f}, D2D1::ColorF(0x071017), 1.7f * scale);

    switch (type)
    {
    case PlayerUnit::Box:
        FillRoundRect(D2D1::RectF(center.x - 14.0f * scale, center.y + 6.0f * scale, center.x + 14.0f * scale, center.y + 22.0f * scale), 4.0f * scale, D2D1::ColorF(0xDCA85B, enabled ? 0.85f : 0.38f));
        break;
    case PlayerUnit::Spark:
        DrawLine({center.x + 9.0f * scale, center.y - 16.0f * scale}, {center.x + 19.0f * scale, center.y - 31.0f * scale}, accent, 2.8f * scale);
        FillEllipse({center.x + 21.0f * scale, center.y - 34.0f * scale}, 4.8f * scale, 4.8f * scale, D2D1::ColorF(0xF6FF83, enabled ? 1.0f : 0.45f));
        break;
    case PlayerUnit::Dash:
        DrawLine({center.x - 28.0f * scale, center.y + 9.0f * scale}, {center.x - 16.0f * scale, center.y + 9.0f * scale}, accent, 2.5f * scale);
        DrawLine({center.x - 30.0f * scale, center.y - 1.0f * scale}, {center.x - 18.0f * scale, center.y - 1.0f * scale}, accent, 2.5f * scale);
        break;
    case PlayerUnit::Bell:
        FillEllipse({center.x, center.y - 25.0f * scale}, 7.0f * scale, 5.0f * scale, D2D1::ColorF(0xF2C94C, enabled ? 1.0f : 0.5f));
        DrawLine({center.x, center.y - 20.0f * scale}, {center.x, center.y - 11.0f * scale}, accent, 2.0f * scale);
        break;
    case PlayerUnit::Titan:
        FillEllipse({center.x - 19.0f * scale, center.y + 2.0f * scale}, 7.0f * scale, 9.0f * scale, body);
        FillEllipse({center.x + 19.0f * scale, center.y + 2.0f * scale}, 7.0f * scale, 9.0f * scale, body);
        break;
    case PlayerUnit::Frost:
        StrokeRoundRect(D2D1::RectF(center.x - 18.0f * scale, center.y + 6.0f * scale, center.x + 18.0f * scale, center.y + 24.0f * scale), 5.0f * scale, accent, 2.2f * scale);
        DrawLine({center.x - 11.0f * scale, center.y + 13.0f * scale}, {center.x + 11.0f * scale, center.y + 13.0f * scale}, accent, 1.8f * scale);
        break;
    case PlayerUnit::Comet:
        DrawLine({center.x - 29.0f * scale, center.y + 10.0f * scale}, {center.x - 14.0f * scale, center.y + 5.0f * scale}, accent, 2.6f * scale);
        DrawLine({center.x - 31.0f * scale, center.y - 1.0f * scale}, {center.x - 16.0f * scale, center.y - 4.0f * scale}, accent, 2.2f * scale);
        break;
    case PlayerUnit::Orbit:
        StrokeEllipse(center, 27.0f * scale, 11.0f * scale, accent, 1.9f * scale);
        FillEllipse({center.x + 24.0f * scale, center.y - 3.0f * scale}, 3.5f * scale, 3.5f * scale, accent);
        break;
    case PlayerUnit::Solar:
        DrawLine({center.x, center.y - 30.0f * scale}, {center.x, center.y - 43.0f * scale}, accent, 2.6f * scale);
        DrawLine({center.x - 24.0f * scale, center.y - 18.0f * scale}, {center.x - 34.0f * scale, center.y - 28.0f * scale}, accent, 2.2f * scale);
        DrawLine({center.x + 24.0f * scale, center.y - 18.0f * scale}, {center.x + 34.0f * scale, center.y - 28.0f * scale}, accent, 2.2f * scale);
        break;
    case PlayerUnit::Mint:
        DrawLine({center.x - 17.0f * scale, center.y - 25.0f * scale}, {center.x + 17.0f * scale, center.y - 25.0f * scale}, accent, 2.2f * scale);
        DrawLine({center.x, center.y - 39.0f * scale}, {center.x, center.y - 11.0f * scale}, accent, 2.2f * scale);
        break;
    case PlayerUnit::Drill:
        FillEllipse({center.x + 19.0f * scale, center.y + 2.0f * scale}, 12.0f * scale, 6.0f * scale, accent);
        DrawLine({center.x + 10.0f * scale, center.y + 2.0f * scale}, {center.x + 34.0f * scale, center.y + 2.0f * scale}, D2D1::ColorF(0xFFF0C8, enabled ? 1.0f : 0.4f), 2.6f * scale);
        break;
    case PlayerUnit::Prism:
        StrokeEllipse(center, 8.0f * scale, 28.0f * scale, accent, 1.8f * scale);
        DrawLine({center.x + 12.0f * scale, center.y - 18.0f * scale}, {center.x + 29.0f * scale, center.y - 34.0f * scale}, accent, 2.2f * scale);
        break;
    case PlayerUnit::Nebula:
        StrokeEllipse(center, 33.0f * scale, 18.0f * scale, accent, 2.0f * scale);
        StrokeEllipse(center, 20.0f * scale, 32.0f * scale, D2D1::ColorF(0xC8B7FF, enabled ? 0.82f : 0.34f), 1.6f * scale);
        break;
    default:
        break;
    }
}

bool PawlineGameImpl::IsUnitInLoadout(PlayerUnit unit) const
{
    for (PlayerUnit item : m_loadout)
    {
        if (item == unit)
        {
            return true;
        }
    }
    return false;
}

void PawlineGameImpl::DrawSpaceDepthGrid(D2D1_RECT_F area, int stageIndex, float time, float cameraX)
{
    const float width = area.right - area.left;
    const float height = area.bottom - area.top;
    const Vec2 vanishing = {
        area.left + width * (0.50f + std::sin(time * 0.07f + static_cast<float>(stageIndex)) * 0.06f),
        area.top + height * 0.34f};
    const float horizon = area.top + height * 0.62f;
    const D2D1_COLOR_F gridColor = D2D1::ColorF(0x6EA8D9);

    // 모든 화면 배경은 특정 행성이 아니라 중립 우주로 보이도록 고정 색의 낮은 알파만 사용한다.
    // 행성별 개성은 전투 무대 바닥 패턴에서만 표현한다.
    for (int i = -8; i <= 8; ++i)
    {
        const float edgeX = area.left + width * 0.5f + static_cast<float>(i) * width * 0.125f - std::fmod(cameraX * 0.040f, width * 0.125f);
        DrawLine(vanishing, {edgeX, area.bottom + 44.0f}, D2D1::ColorF(gridColor.r, gridColor.g, gridColor.b, 0.038f), 1.0f);
    }

    for (int i = 0; i < 12; ++i)
    {
        const float depth = static_cast<float>(i) / 11.0f;
        const float curve = depth * depth;
        const float y = Lerp(horizon, area.bottom + 32.0f, curve);
        const float alpha = 0.018f + curve * 0.046f;
        DrawLine({area.left - 20.0f, y}, {area.right + 20.0f, y}, D2D1::ColorF(gridColor.r, gridColor.g, gridColor.b, alpha), 1.0f + curve * 1.4f);
    }

    for (int i = 0; i < 6; ++i)
    {
        const float drift = std::sin(time * (0.18f + static_cast<float>(i) * 0.03f) + static_cast<float>(i)) * width * 0.035f;
        const float y = horizon - 26.0f - static_cast<float>(i) * height * 0.035f;
        DrawLine({area.left + width * 0.14f + drift, y},
                 {area.left + width * 0.86f + drift * 0.35f, y + height * 0.018f},
                 D2D1::ColorF(0xCFE8F5, 0.018f),
                 1.0f);
    }
}

void PawlineGameImpl::DrawDeepSpaceBackdrop(D2D1_RECT_F area, int stageIndex, float time, float cameraX, bool showRoute)
{
    const int safeStage = std::max(0, std::min(kStageCount - 1, stageIndex));
    const float width = area.right - area.left;
    const float height = area.bottom - area.top;

    FillRect(area, D2D1::ColorF(0x01050B));
    if (m_deepSpaceBitmap)
    {
        const bool gameplayBackdrop = (area.right - area.left) > kWidth + 180.0f || std::abs(cameraX) > 0.01f;
        DrawBitmapCover(m_deepSpaceBitmap.Get(), area, 0.98f, time, cameraX, gameplayBackdrop ? 3.0f : 0.45f);
    }
    FillRect(area, D2D1::ColorF(0x01050B, 0.18f));
    // 고화질 우주 사진 자체가 깊이감을 만들기 때문에, 배경 격자는 전면에서 빼서 선이 튀지 않게 한다.

    // 실제 우주 사진 위에는 위치감을 위한 먼지 입자만 얇게 얹는다.
    // 큰 별, 행성, 소행성처럼 보이는 배경 장식은 사용하지 않는다.
    for (int layer = 0; layer < 2; ++layer)
    {
        const int count = 8 + layer * 6;
        const float depth = 0.38f + static_cast<float>(layer) * 0.36f;
        const float drift = time * (1.8f + static_cast<float>(layer) * 3.0f) - cameraX * (0.006f + depth * 0.010f);
        for (int i = 0; i < count; ++i)
        {
            const float rawX = static_cast<float>(i * (137 + layer * 31) + safeStage * 73);
            const float rawY = static_cast<float>(i * (83 + layer * 19) + safeStage * 47);
            const float x = area.left - 90.0f + std::fmod(rawX + drift, width + 180.0f);
            const float y = area.top + 12.0f + std::fmod(rawY + std::sin(time * 0.18f + rawX) * 8.0f, height - 24.0f);
            const float twinkle = 0.45f + 0.55f * Hash01(static_cast<float>(i), static_cast<float>(layer + safeStage), std::floor(time * (1.6f + depth)));
            const float radius = (0.55f + static_cast<float>((i + layer) % 3) * 0.18f) * (0.72f + depth);
            const float alpha = (0.010f + depth * 0.018f) * twinkle;
            FillEllipse({x, y}, radius, radius, D2D1::ColorF(0xEAF7FF, alpha));
        }
    }

    if (showRoute)
    {
        Vec2 previous = {};
        for (int i = 0; i < kStageCount; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(kStageCount - 1);
            const float phase = time * 0.25f + t * kPi * 2.0f;
            const Vec2 node = {area.left + width * (0.12f + t * 0.76f), area.top + height * (0.86f + std::sin(phase) * 0.022f)};
            if (i > 0)
            {
                DrawLine(previous, node, D2D1::ColorF(0xCFE8F5, 0.20f), 2.0f);
            }
            const StageDefinition routeStage = GetStageDefinition(i);
            const float active = i == safeStage ? 1.0f : 0.0f;
            FillEllipse(node, 6.5f + active * 4.5f, 6.5f + active * 4.5f, D2D1::ColorF(routeStage.laneColor.r, routeStage.laneColor.g, routeStage.laneColor.b, 0.72f));
            StrokeEllipse(node, 6.5f + active * 4.5f, 6.5f + active * 4.5f, D2D1::ColorF(routeStage.lineColor.r, routeStage.lineColor.g, routeStage.lineColor.b, 0.84f), 1.6f + active);
            previous = node;
        }
    }
}

void PawlineGameImpl::DrawTitle()
{
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), 9, m_uiTime, 0.0f, false);

    for (int i = 0; i < kStageCount; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(kStageCount - 1);
        const Vec2 node = {146.0f + t * 988.0f, 704.0f + std::sin(t * kPi * 2.0f) * 24.0f};
        const StageDefinition routeStage = GetStageDefinition(i);
        if (i > 0)
        {
            const float prevT = static_cast<float>(i - 1) / static_cast<float>(kStageCount - 1);
            const Vec2 prev = {146.0f + prevT * 988.0f, 704.0f + std::sin(prevT * kPi * 2.0f) * 24.0f};
            DrawLine(prev, node, D2D1::ColorF(0xCFE8F5, 0.24f), 2.0f);
        }
        FillEllipse(node, 11.0f + static_cast<float>(i == 9) * 7.0f, 11.0f + static_cast<float>(i == 9) * 7.0f, D2D1::ColorF(routeStage.laneColor.r, routeStage.laneColor.g, routeStage.laneColor.b, 0.92f));
        StrokeEllipse(node, 11.0f + static_cast<float>(i == 9) * 7.0f, 11.0f + static_cast<float>(i == 9) * 7.0f, routeStage.lineColor, 2.0f);
    }

    FillEllipse({640.0f, 272.0f}, 70.0f, 22.0f, D2D1::ColorF(0x000000, 0.28f));
    FillEllipse({640.0f, 264.0f}, 60.0f, 60.0f, D2D1::ColorF(0x65B8FF, 0.10f));
    FillEllipse({640.0f, 264.0f}, 44.0f, 44.0f, D2D1::ColorF(0xF3FBFF));
    StrokeEllipse({640.0f, 264.0f}, 44.0f, 44.0f, D2D1::ColorF(0x65B8FF), 3.0f);
    FillEllipse({626.0f, 255.0f}, 4.0f, 6.0f, D2D1::ColorF(0x071017));
    FillEllipse({654.0f, 255.0f}, 4.0f, 6.0f, D2D1::ColorF(0x071017));
    DrawLine({623.0f, 281.0f}, {657.0f, 281.0f}, D2D1::ColorF(0x071017), 2.0f);

    DrawPixelTextCentered(L"PAWLINE DEFENSE", D2D1::RectF(166.0f, 350.0f, 1114.0f, 424.0f), 6.2f, D2D1::ColorF(0xF3FBFF), 1.0f);
    DrawOutlinedString(L"수성에서 태양까지 이어지는 탑뷰 라인 디펜스", D2D1::RectF(250.0f, 428.0f, 1030.0f, 458.0f), m_centerFormat, D2D1::ColorF(0xCFE8F5), 0.72f);

    DrawButton(TitleStartButtonRect(), L"START", true, D2D1::ColorF(0x173C4B));
    DrawButton(TitleStoryButtonRect(), L"STORY", true, D2D1::ColorF(0x22324D));
    DrawButton(TitleDemoButtonRect(), L"DEMO", true, D2D1::ColorF(0x4B4321));
    DrawButton(TitleOptionsButtonRect(), L"OPTIONS", true, D2D1::ColorF(0x283B27));
    DrawButton(TitleQuitButtonRect(), L"QUIT", true, D2D1::ColorF(0x332337));

    const std::wstring route = L"수성  금성  지구  화성  목성  토성  천왕성  해왕성  명왕성  태양";
    DrawOutlinedString(route, D2D1::RectF(120.0f, 748.0f, 1160.0f, 778.0f), m_centerFormat, D2D1::ColorF(0xF6FF83), 0.60f);
}

void PawlineGameImpl::DrawStoryCrawl()
{
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), 9, m_uiTime, 0.0f, true);
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x01050B, 0.42f));

    const std::array<std::wstring, 11> lines = {
        L"먼 행성들의 항로가 하나씩 꺼지던 밤,",
        L"작은 기지 하나가 수성 궤도에 남았다.",
        L"그곳의 이름은 파울라인.",
        L"병사도 함대도 충분하지 않았지만,",
        L"아직 전선을 밀어낼 불빛은 남아 있었다.",
        L"너는 다섯 유닛을 골라 길어진 방어선을 열고,",
        L"수성에서 태양까지 이어진 침묵의 항로를 되찾아야 한다.",
        L"적은 행성마다 다른 방식으로 몰려오고,",
        L"마지막 빛 앞에는 태양의 보스가 기다린다.",
        L"전선이 밀리면 기록은 끝난다.",
        L"하지만 한 걸음이라도 앞으로 나아가면, 어둠은 다시 길이 된다."};

    // 첫 화면부터 본문이 보여야 사용자가 멈춘 화면으로 오해하지 않는다.
    // 이후에는 천천히 위로 올라가는 크롤 느낌만 유지한다.
    const float startY = 278.0f - m_storyTimer * 34.0f;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        const float y = startY + static_cast<float>(i) * 48.0f;
        if (y < 126.0f || y > 756.0f)
        {
            continue;
        }

        const float topFade = Clamp01((y - 126.0f) / 86.0f);
        const float bottomFade = Clamp01((756.0f - y) / 110.0f);
        const float alpha = topFade * bottomFade;
        const float depth = Clamp01((y - 126.0f) / 630.0f);
        const float inset = 164.0f + (1.0f - depth) * 142.0f;
        const D2D1_RECT_F textRect = D2D1::RectF(inset, y, kWidth - inset, y + 34.0f);
        DrawOutlinedString(lines[i], textRect, m_centerFormat, D2D1::ColorF(0xFFF0B5, 0.92f * alpha), 0.82f * alpha);
    }

    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, 116.0f), D2D1::ColorF(0x01050B, 0.62f));
    FillRect(D2D1::RectF(0.0f, 708.0f, kWidth, kHeight), D2D1::ColorF(0x01050B, 0.58f));
    const float titleFade = Clamp01(1.0f - std::max(0.0f, m_storyTimer - 4.2f) / 2.2f);
    DrawPixelTextCentered(L"MISSION LOG 00", D2D1::RectF(260.0f, 88.0f, 1020.0f, 138.0f), 4.2f, D2D1::ColorF(0xF6FF83), titleFade);
    DrawPixelTextCentered(L"DAWNLINE ARCHIVE", D2D1::RectF(260.0f, 142.0f, 1020.0f, 190.0f), 3.2f, D2D1::ColorF(0xCFE8F5), titleFade);
    DrawOutlinedString(L"SPACE / ENTER / CLICK", D2D1::RectF(390.0f, 690.0f, 890.0f, 714.0f), m_centerFormat, D2D1::ColorF(0xCFE8F5), 0.70f);
    DrawButton(StorySkipButtonRect(), m_storyAutoContinueToMenu ? L"SKIP & START" : L"SKIP", true, D2D1::ColorF(0x173C4B));
}

void PawlineGameImpl::DrawEndingScene()
{
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), kStageCount - 1, m_uiTime, 0.0f, true);
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x01050B, 0.34f));

    const Vec2 sun = {640.0f, 210.0f};
    const float pulse = 0.5f + 0.5f * std::sin(m_uiTime * 1.4f);
    for (int i = 0; i < 24; ++i)
    {
        const float angle = static_cast<float>(i) / 24.0f * kPi * 2.0f + m_uiTime * 0.16f;
        DrawLine({sun.x + std::cos(angle) * 54.0f, sun.y + std::sin(angle) * 54.0f},
                 {sun.x + std::cos(angle) * (160.0f + pulse * 18.0f), sun.y + std::sin(angle) * (160.0f + pulse * 18.0f)},
                 D2D1::ColorF(0xFFB347, 0.10f),
                 3.0f);
    }
    FillEllipse(sun, 128.0f + pulse * 18.0f, 128.0f + pulse * 18.0f, D2D1::ColorF(0xFFB347, 0.08f));
    FillEllipse(sun, 74.0f, 74.0f, D2D1::ColorF(0xFFB347, 0.28f));
    FillEllipse(sun, 42.0f, 42.0f, D2D1::ColorF(0xFFF0B5, 0.72f));

    DrawPixelTextCentered(L"SOLAR LINE RESTORED", D2D1::RectF(130.0f, 330.0f, 1150.0f, 392.0f), 4.6f, D2D1::ColorF(0xF6FF83), Clamp01(m_endingTimer / 1.2f));

    const std::array<std::wstring, 5> lines = {
        L"태양의 플레어가 잦아들고,",
        L"수성에서 명왕성까지 흩어진 신호가 하나의 선으로 이어졌다.",
        L"파울라인은 거대한 함대가 아니었다.",
        L"하지만 작은 불빛들은 끝까지 버텼고,",
        L"그 빛은 이제 항로가 되었다."};

    for (size_t i = 0; i < lines.size(); ++i)
    {
        const float alpha = Clamp01((m_endingTimer - 1.6f - static_cast<float>(i) * 1.35f) / 0.9f);
        DrawOutlinedString(lines[i],
                           D2D1::RectF(170.0f, 424.0f + static_cast<float>(i) * 42.0f, 1110.0f, 458.0f + static_cast<float>(i) * 42.0f),
                           m_centerFormat,
                           D2D1::ColorF(0xEAF7FF, alpha),
                           0.80f * alpha);
    }

    const float routeAlpha = Clamp01((m_endingTimer - 8.4f) / 1.4f);
    Vec2 previous = {};
    const float routeY = 676.0f;
    for (int i = 0; i < kStageCount; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(kStageCount - 1);
        const Vec2 node = {146.0f + t * 988.0f, routeY + std::sin(m_uiTime * 0.6f + t * kPi * 2.0f) * 12.0f};
        if (i > 0)
        {
            DrawLine(previous, node, D2D1::ColorF(0xF6FF83, 0.26f * routeAlpha), 2.4f);
        }
        const StageDefinition stage = GetStageDefinition(i);
        FillEllipse(node, 9.0f + static_cast<float>(i == kStageCount - 1) * 7.0f, 9.0f + static_cast<float>(i == kStageCount - 1) * 7.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.78f * routeAlpha));
        StrokeEllipse(node, 9.0f + static_cast<float>(i == kStageCount - 1) * 7.0f, 9.0f + static_cast<float>(i == kStageCount - 1) * 7.0f, D2D1::ColorF(0xF3FBFF, 0.62f * routeAlpha), 1.7f);
        previous = node;
    }

    DrawPixelTextCentered(L"ENTER / SPACE CLOSE", D2D1::RectF(380.0f, 734.0f, 900.0f, 770.0f), 2.2f, D2D1::ColorF(0xCFE8F5), 0.90f);
}

void PawlineGameImpl::DrawOptions()
{
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), m_selectedStage, m_uiTime, 0.0f, false);
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x01050B, 0.38f));

    const D2D1_RECT_F mainPanel = D2D1::RectF(186.0f, 48.0f, 1094.0f, 748.0f);
    FillRoundRect(OffsetRectF(mainPanel, 6.0f, 8.0f), 10.0f, D2D1::ColorF(0x000000, 0.35f));
    FillRoundRect(mainPanel, 10.0f, D2D1::ColorF(0x071017, 0.88f));
    StrokeRoundRect(mainPanel, 10.0f, D2D1::ColorF(0x65B8FF, 0.68f), 1.8f);
    FillRoundRect(D2D1::RectF(mainPanel.left + 12.0f, mainPanel.top + 10.0f, mainPanel.right - 12.0f, mainPanel.top + 24.0f), 6.0f, D2D1::ColorF(0xFFFFFF, 0.08f));

    const auto drawSection = [&](D2D1_RECT_F rect, const std::wstring& label, D2D1_COLOR_F accent) {
        FillRoundRect(OffsetRectF(rect, 4.0f, 5.0f), 9.0f, D2D1::ColorF(0x000000, 0.26f));
        FillRoundRect(rect, 9.0f, D2D1::ColorF(0x0A1921, 0.86f));
        FillRoundRect(D2D1::RectF(rect.left + 10.0f, rect.top + 8.0f, rect.right - 10.0f, rect.top + 20.0f), 6.0f, D2D1::ColorF(0xFFFFFF, 0.07f));
        StrokeRoundRect(rect, 9.0f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.60f), 1.4f);
        DrawPixelText(label, {rect.left + 20.0f, rect.top + 18.0f}, 2.6f, accent);
    };

    const auto drawSlider = [&](const std::wstring& label, D2D1_RECT_F bar, float normalized, const std::wstring& valueText, D2D1_COLOR_F accent) {
        const float value = Clamp01(normalized);
        const bool hover = Contains(bar, m_mouse);
        DrawOutlinedString(label, D2D1::RectF(bar.left, bar.top - 38.0f, bar.right - 96.0f, bar.top - 10.0f), m_bodyFormat, D2D1::ColorF(0xEAF7FF), 0.70f);
        DrawOutlinedString(valueText, D2D1::RectF(bar.right - 86.0f, bar.top - 38.0f, bar.right + 20.0f, bar.top - 10.0f), m_bodyFormat, D2D1::ColorF(0xF6FF83), 0.70f);
        FillRoundRect(bar, 10.0f, D2D1::ColorF(0x02080D, 0.92f));
        const float knobX = Lerp(bar.left, bar.right, value);
        FillRoundRect(D2D1::RectF(bar.left, bar.top, knobX, bar.bottom), 10.0f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.72f));
        StrokeRoundRect(bar, 10.0f, D2D1::ColorF(accent.r, accent.g, accent.b, hover ? 0.92f : 0.52f), hover ? 2.0f : 1.2f);
        for (int i = 0; i <= 5; ++i)
        {
            const float x = Lerp(bar.left, bar.right, static_cast<float>(i) / 5.0f);
            DrawLine({x, bar.top + 3.0f}, {x, bar.bottom - 3.0f}, D2D1::ColorF(0xFFFFFF, 0.12f), 1.0f);
        }
        FillEllipse({knobX, (bar.top + bar.bottom) * 0.5f}, hover ? 12.0f : 10.0f, hover ? 12.0f : 10.0f, D2D1::ColorF(0xF3FBFF, 0.96f));
        StrokeEllipse({knobX, (bar.top + bar.bottom) * 0.5f}, hover ? 12.0f : 10.0f, hover ? 12.0f : 10.0f, accent, 2.0f);
    };

    DrawString(L"옵션", D2D1::RectF(236.0f, 76.0f, 1044.0f, 120.0f), m_titleFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"소리, 전투 감각, 화면 안전 여백, 저장 데이터를 한 곳에서 조정해.", D2D1::RectF(236.0f, 116.0f, 1044.0f, 142.0f), m_centerFormat, D2D1::ColorF(0xBFD1DB));

    drawSection(D2D1::RectF(236.0f, 146.0f, 1044.0f, 224.0f), L"SAVE SLOT", D2D1::ColorF(0xB8FF89));
    for (int i = 0; i < kSaveSlotCount; ++i)
    {
        const bool active = i == m_saveSlot;
        DrawButton(OptionsSaveSlotButtonRect(i), L"SLOT " + ToWideInt(i + 1), true, active ? D2D1::ColorF(0x3F4A22) : D2D1::ColorF(0x202833));
    }

    drawSection(D2D1::RectF(236.0f, 238.0f, 704.0f, 604.0f), L"COMBAT", D2D1::ColorF(0x65B8FF));
    DrawButton(OptionsShakeButtonRect(), m_hitShakeEnabled ? L"SHAKE ON" : L"SHAKE OFF", true, m_hitShakeEnabled ? D2D1::ColorF(0x173C4B) : D2D1::ColorF(0x302735));
    DrawButton(OptionsFlashButtonRect(), m_reduceFlashes ? L"FLASH LESS" : L"FLASH FULL", true, m_reduceFlashes ? D2D1::ColorF(0x283B27) : D2D1::ColorF(0x302735));
    drawSlider(L"기본 게임 속도", OptionsSpeedSliderRect(), (m_defaultGameSpeed - 0.5f) / 2.5f, L"x" + ToWideFloat(m_defaultGameSpeed), D2D1::ColorF(0xF6FF83));
    drawSlider(L"화면 안전 여백", OptionsViewSliderRect(), (m_userViewScale - 0.82f) / 0.18f, ToWideInt(static_cast<int>(std::round(m_userViewScale * 100.0f))) + L"%", D2D1::ColorF(0xB8FF89));
    DrawButton(OptionsViewResetButtonRect(), L"AUTO FIT", true, D2D1::ColorF(0x2D3722));

    drawSection(D2D1::RectF(724.0f, 238.0f, 1044.0f, 604.0f), L"AUDIO", D2D1::ColorF(0xF6FF83));
    drawSlider(L"효과음 볼륨", OptionsSfxSliderRect(), m_sfxVolume, ToWideInt(static_cast<int>(std::round(m_sfxVolume * 100.0f))) + L"%", D2D1::ColorF(0x65B8FF));
    drawSlider(L"UI 볼륨", OptionsUiSliderRect(), m_uiVolume, ToWideInt(static_cast<int>(std::round(m_uiVolume * 100.0f))) + L"%", D2D1::ColorF(0xC8B7FF));
    drawSlider(L"브금 볼륨", OptionsBgmSliderRect(), m_bgmVolume, ToWideInt(static_cast<int>(std::round(m_bgmVolume * 100.0f))) + L"%", D2D1::ColorF(0xB8FF89));
    DrawButton(OptionsAudioResetButtonRect(), L"AUDIO RESET", true, D2D1::ColorF(0x283B27));

    drawSection(D2D1::RectF(236.0f, 612.0f, 1044.0f, 742.0f), L"DATA", D2D1::ColorF(0xC8B7FF));
    DrawButton(OptionsSaveProgressButtonRect(), L"SAVE S", true, D2D1::ColorF(0x283B27));
    DrawButton(OptionsLoadProgressButtonRect(), L"LOAD L", true, D2D1::ColorF(0x22323F));
    DrawButton(OptionsDeleteProgressButtonRect(), m_deleteConfirmTimer > 0.0f ? L"CONFIRM D" : L"DELETE D", true, m_deleteConfirmTimer > 0.0f ? D2D1::ColorF(0x4B232D) : D2D1::ColorF(0x302735));
    DrawButton(OptionsResetProgressButtonRect(), m_resetConfirmTimer > 0.0f ? L"CONFIRM X" : L"RESET X", true, m_resetConfirmTimer > 0.0f ? D2D1::ColorF(0x4B232D) : D2D1::ColorF(0x302735));
    DrawButton(OptionsBackButtonRect(), L"BACK", true, D2D1::ColorF(0x173C4B));
    DrawString(L"Esc / Backspace", D2D1::RectF(OptionsBackButtonRect().right + 14.0f, OptionsBackButtonRect().top + 8.0f, OptionsBackButtonRect().right + 168.0f, OptionsBackButtonRect().bottom), m_smallFormat, D2D1::ColorF(0x8EA9B8));
}

void PawlineGameImpl::DrawMenu()
{
    const StageDefinition menuStage = CurrentStage();
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), m_selectedStage, m_uiTime, 0.0f, true);
    DrawLine({86.0f, 638.0f}, {1162.0f, 698.0f}, D2D1::ColorF(menuStage.lineColor.r, menuStage.lineColor.g, menuStage.lineColor.b, 0.18f), 1.8f);
    for (float y = 30.0f; y < kHeight; y += 44.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.045f), 1.0f);
    }
    for (float x = 32.0f; x < kWidth; x += 48.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.032f), 1.0f);
    }
    DrawPixelText(L"PAWLINE DEFENSE", {48.0f, 42.0f}, 4.2f, D2D1::ColorF(0xF3FBFF));
    DrawOutlinedString(L"스테이지와 다섯 유닛을 골라 출격해.", D2D1::RectF(58.0f, 88.0f, 460.0f, 118.0f), m_smallFormat, D2D1::ColorF(0xBFD1DB), 0.58f);
    DrawPixelText(L"LUMEN " + ToWideInt(m_lumen), {1000.0f, 50.0f}, 3.0f, D2D1::ColorF(0xF6FF83));
    if (m_debugMode)
    {
        DrawPixelText(L"DEBUG ON", {1004.0f, 88.0f}, 2.0f, D2D1::ColorF(0xFFB6C2));
    }

    DrawString(L"스테이지", D2D1::RectF(48.0f, 112.0f, 260.0f, 140.0f), m_headerFormat, D2D1::ColorF(0xEAF7FF));
    for (int i = 0; i < kStageCount; ++i)
    {
        DrawStageCard(i);
    }

    DrawString(L"편성", D2D1::RectF(58.0f, 326.0f, 280.0f, 354.0f), m_headerFormat, D2D1::ColorF(0xEAF7FF));
    for (int i = 0; i < kLoadoutSize; ++i)
    {
        DrawLoadoutSlot(i);
    }

    DrawString(L"부대 명단", D2D1::RectF(650.0f, 326.0f, 850.0f, 354.0f), m_headerFormat, D2D1::ColorF(0xEAF7FF));
    for (int i = 0; i < kRosterCount; ++i)
    {
        DrawRosterCard(i);
    }

    FillRoundRect(D2D1::RectF(58.0f, 510.0f, 570.0f, 724.0f), 8.0f, D2D1::ColorF(0x0E1D26));
    StrokeRoundRect(D2D1::RectF(58.0f, 510.0f, 570.0f, 724.0f), 8.0f, D2D1::ColorF(0x2C4A5B), 1.0f);
    FillEllipse({520.0f, 550.0f}, 34.0f, 34.0f, D2D1::ColorF(menuStage.laneColor.r, menuStage.laneColor.g, menuStage.laneColor.b, 0.92f));
    StrokeEllipse({520.0f, 550.0f}, 34.0f, 34.0f, menuStage.lineColor, 2.2f);
    DrawString(menuStage.name, D2D1::RectF(82.0f, 532.0f, 500.0f, 562.0f), m_headerFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(menuStage.subtitle, D2D1::RectF(82.0f, 566.0f, 540.0f, 590.0f), m_bodyFormat, D2D1::ColorF(0xBFD1DB));
    DrawString(menuStage.gimmick, D2D1::RectF(82.0f, 594.0f, 540.0f, 618.0f), m_bodyFormat, D2D1::ColorF(0xF6FF83));
    DrawString(L"적 구성  " + StageEnemySummary(), D2D1::RectF(82.0f, 622.0f, 540.0f, 648.0f), m_bodyFormat, D2D1::ColorF(0xFFB6C2));
    DrawString(L"적 기지 체력  " + ToWideInt(static_cast<int>(menuStage.enemyHp)), D2D1::RectF(82.0f, 650.0f, 540.0f, 676.0f), m_bodyFormat, D2D1::ColorF(0xFFB6C2));
    DrawString(L"시작 에너지  " + ToWideInt(static_cast<int>(menuStage.startEnergy)), D2D1::RectF(82.0f, 678.0f, 540.0f, 704.0f), m_bodyFormat, D2D1::ColorF(0xB8FF89));

    const bool selectedUnlocked = IsStageUnlocked(m_selectedStage);
    DrawButton(MenuArchiveButtonRect(), L"ARCHIVE", true, D2D1::ColorF(0x22323F));
    DrawButton(MenuShopButtonRect(), L"SHOP", true, D2D1::ColorF(0x4B4321));
    DrawButton(StartGameButtonRect(), selectedUnlocked ? L"START STAGE" : L"LOCKED", selectedUnlocked, D2D1::ColorF(0x173C4B));
    DrawString(L"D / C", D2D1::RectF(MenuArchiveButtonRect().left, MenuArchiveButtonRect().bottom + 4.0f, MenuArchiveButtonRect().right, MenuArchiveButtonRect().bottom + 24.0f), m_centerFormat, D2D1::ColorF(0x8EA9B8));
    DrawString(L"S / B", D2D1::RectF(MenuShopButtonRect().left, MenuShopButtonRect().bottom + 4.0f, MenuShopButtonRect().right, MenuShopButtonRect().bottom + 24.0f), m_centerFormat, D2D1::ColorF(0x8EA9B8));
    DrawString(selectedUnlocked ? L"엔터 / 스페이스" : L"이전 행성 클리어 필요", D2D1::RectF(StartGameButtonRect().left, StartGameButtonRect().bottom + 4.0f, StartGameButtonRect().right, StartGameButtonRect().bottom + 28.0f), m_centerFormat, selectedUnlocked ? D2D1::ColorF(0x8EA9B8) : D2D1::ColorF(0xFFB6C2));
}

void PawlineGameImpl::DrawArchive()
{
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), m_selectedStage, m_uiTime, 0.0f, false);
    for (float y = 28.0f; y < kHeight; y += 42.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.038f), 1.0f);
    }
    for (float x = 34.0f; x < kWidth; x += 50.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.026f), 1.0f);
    }

    DrawPixelText(L"TACTICAL ARCHIVE", {52.0f, 40.0f}, 4.25f, D2D1::ColorF(0xF3FBFF));
    DrawString(L"유닛, 적, 행성 정보를 한곳에서 확인합니다.", D2D1::RectF(58.0f, 92.0f, 520.0f, 120.0f), m_bodyFormat, D2D1::ColorF(0xBFD1DB));

    const std::array<std::wstring, 3> tabs = {L"UNITS", L"ENEMY", L"STAGES"};
    for (int i = 0; i < 3; ++i)
    {
        const bool active = m_archiveTab == i;
        DrawButton(ArchiveTabRect(i), tabs[i], true, active ? D2D1::ColorF(0x173C4B) : D2D1::ColorF(0x202833));
    }

    const D2D1_RECT_F board = D2D1::RectF(52.0f, 186.0f, 1228.0f, 690.0f);
    DrawCartoonPanel(board, D2D1::ColorF(0x0D1821, 0.96f), D2D1::ColorF(0x65B8FF));

    if (m_archiveTab == 0)
    {
        for (int i = 0; i < kRosterCount; ++i)
        {
            const PlayerUnit unit = static_cast<PlayerUnit>(i);
            const UnitStats stats = PlayerStats(unit);
            const bool unlocked = IsUnitUnlocked(unit);
            const int col = i / 7;
            const int row = i % 7;
            const float x = 82.0f + static_cast<float>(col) * 570.0f;
            const float y = 212.0f + static_cast<float>(row) * 64.0f;
            const D2D1_RECT_F rect = D2D1::RectF(x, y, x + 516.0f, y + 52.0f);
            DrawCartoonPanel(rect, unlocked ? D2D1::ColorF(0x101D26, 0.96f) : D2D1::ColorF(0x101820, 0.78f), unlocked ? stats.accent : D2D1::ColorF(0x394955));
            DrawPlayerIcon(unit, {rect.left + 30.0f, rect.top + 26.0f}, 0.52f, unlocked);
            DrawString(unlocked ? stats.name : L"잠김 유닛", D2D1::RectF(rect.left + 64.0f, rect.top + 8.0f, rect.left + 238.0f, rect.top + 30.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x73818A));
            DrawString(unlocked ? L"Lv." + ToWideInt(UnitLevel(unit)) + (IsUnitEvolved(unit) ? L"  진화" : L"") : L"상점에서 구매", D2D1::RectF(rect.left + 64.0f, rect.top + 29.0f, rect.left + 238.0f, rect.bottom - 4.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2));
            DrawString(L"HP " + ToWideInt(static_cast<int>(stats.hp)) + L"  DMG " + ToWideInt(static_cast<int>(stats.damage)) + L"  COST " + ToWideInt(UnitEnergyCost(unit)), D2D1::RectF(rect.left + 248.0f, rect.top + 15.0f, rect.right - 12.0f, rect.bottom - 8.0f), m_smallFormat, D2D1::ColorF(0xC7D8FF));
        }
    }
    else if (m_archiveTab == 1)
    {
        for (int i = 0; i <= static_cast<int>(EnemyUnit::Boss); ++i)
        {
            const EnemyUnit enemy = static_cast<EnemyUnit>(i);
            const UnitStats stats = GetEnemyStats(enemy, DifficultyThreatMultiplier() * 2.0f + static_cast<float>(m_selectedStage) * 0.45f);
            const int col = i / 9;
            const int row = i % 9;
            const float x = 84.0f + static_cast<float>(col) * 568.0f;
            const float y = 210.0f + static_cast<float>(row) * 50.0f;
            const D2D1_RECT_F rect = D2D1::RectF(x, y, x + 512.0f, y + 40.0f);
            DrawCartoonPanel(rect, D2D1::ColorF(0x111923, 0.94f), stats.accent);
            FillEllipse({rect.left + 24.0f, rect.top + 20.0f}, stats.radius * 0.55f, stats.radius * 0.48f, stats.color);
            StrokeEllipse({rect.left + 24.0f, rect.top + 20.0f}, stats.radius * 0.55f, stats.radius * 0.48f, stats.accent, 1.6f);
            DrawString(stats.name, D2D1::RectF(rect.left + 54.0f, rect.top + 8.0f, rect.left + 236.0f, rect.bottom - 6.0f), m_smallFormat, D2D1::ColorF(0xF3FBFF));
            DrawString(L"HP " + ToWideInt(static_cast<int>(stats.hp)) + L"  DMG " + ToWideInt(static_cast<int>(stats.damage)) + L"  보상 " + ToWideInt(stats.reward), D2D1::RectF(rect.left + 244.0f, rect.top + 8.0f, rect.right - 10.0f, rect.bottom - 6.0f), m_smallFormat, D2D1::ColorF(0xFFCAD1));
        }
    }
    else
    {
        for (int i = 0; i < kStageCount; ++i)
        {
            const StageDefinition stage = GetStageDefinition(i);
            const bool unlocked = IsStageUnlocked(i);
            const int col = i / 5;
            const int row = i % 5;
            const float x = 86.0f + static_cast<float>(col) * 568.0f;
            const float y = 220.0f + static_cast<float>(row) * 84.0f;
            const D2D1_RECT_F rect = D2D1::RectF(x, y, x + 510.0f, y + 66.0f);
            DrawCartoonPanel(rect, unlocked ? D2D1::ColorF(0x101D26, 0.96f) : D2D1::ColorF(0x101820, 0.76f), unlocked ? stage.lineColor : D2D1::ColorF(0x394955));
            FillEllipse({rect.left + 34.0f, rect.top + 33.0f}, 20.0f, 20.0f, unlocked ? stage.laneColor : D2D1::ColorF(0x394955));
            StrokeEllipse({rect.left + 34.0f, rect.top + 33.0f}, 20.0f, 20.0f, stage.lineColor, 2.0f);
            DrawString(stage.name + (m_stageCleared[i] ? L"  클리어" : (unlocked ? L"  진행 가능" : L"  잠김")), D2D1::RectF(rect.left + 72.0f, rect.top + 8.0f, rect.left + 264.0f, rect.top + 32.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x73818A));
            DrawString(stage.gimmick, D2D1::RectF(rect.left + 72.0f, rect.top + 34.0f, rect.right - 12.0f, rect.bottom - 8.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0x7E919C));
        }
    }

    DrawButton(ArchiveBackButtonRect(), L"BACK", true, D2D1::ColorF(0x173C4B));
    DrawString(L"Esc / M", D2D1::RectF(ArchiveBackButtonRect().right + 12.0f, ArchiveBackButtonRect().top + 14.0f, ArchiveBackButtonRect().right + 120.0f, ArchiveBackButtonRect().bottom), m_bodyFormat, D2D1::ColorF(0x8EA9B8));
}

void PawlineGameImpl::DrawBriefing()
{
    const StageDefinition stage = CurrentStage();
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), m_selectedStage, m_uiTime, 0.0f, false);

    DrawPixelText(L"MISSION BRIEF", {74.0f, 48.0f}, 5.0f, D2D1::ColorF(0xF3FBFF));
    DrawPixelTextCentered(L"LUMEN " + ToWideInt(m_lumen), D2D1::RectF(972.0f, 50.0f, 1220.0f, 82.0f), 2.8f, D2D1::ColorF(0xF6FF83), 1.0f);

    const D2D1_RECT_F terrainPanel = D2D1::RectF(78.0f, 136.0f, 516.0f, 500.0f);
    DrawBriefingPanel(terrainPanel, D2D1::ColorF(0x0D1821, 0.96f), stage.lineColor);
    DrawOutlinedString(stage.name, D2D1::RectF(106.0f, 158.0f, 488.0f, 188.0f), m_centerFormat, D2D1::ColorF(0xF3FBFF), 0.70f);
    DrawOutlinedString(stage.subtitle, D2D1::RectF(106.0f, 188.0f, 488.0f, 212.0f), m_centerFormat, D2D1::ColorF(0xBFD1DB), 0.56f);
    DrawPixelTextCentered(L"SURFACE SAMPLE", D2D1::RectF(106.0f, 216.0f, 488.0f, 240.0f), 1.75f, D2D1::ColorF(0xCFE8F5), 1.0f);

    const D2D1_RECT_F terrain = D2D1::RectF(106.0f, 248.0f, 488.0f, 330.0f);
    FillRoundRect(terrain, 20.0f, D2D1::ColorF(stage.laneColor.r, stage.laneColor.g, stage.laneColor.b, 0.88f));
    FillRoundRect(D2D1::RectF(terrain.left + 10.0f, terrain.top + 18.0f, terrain.right - 10.0f, terrain.bottom - 18.0f),
                  16.0f,
                  D2D1::ColorF(stage.laneInnerColor.r, stage.laneInnerColor.g, stage.laneInnerColor.b, 0.94f));
    DrawLine({terrain.left + 24.0f, (terrain.top + terrain.bottom) * 0.5f},
             {terrain.right - 24.0f, (terrain.top + terrain.bottom) * 0.5f},
             D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.72f),
             3.0f);
    for (int i = 0; i < 7; ++i)
    {
        const float x = terrain.left + 42.0f + static_cast<float>(i) * 52.0f;
        const float y = terrain.top + 20.0f + std::sin(m_uiTime * 0.4f + static_cast<float>(i)) * 5.0f;
        DrawCrater({x, y + 26.0f}, 13.0f + static_cast<float>(i % 3) * 4.0f, 4.5f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.32f), D2D1::ColorF(0x000000, 0.18f));
    }
    FillRoundRect(D2D1::RectF(106.0f, 346.0f, 488.0f, 390.0f), 12.0f, D2D1::ColorF(0x0B1D28, 0.86f));
    DrawOutlinedString(L"이벤트",
                       D2D1::RectF(122.0f, 354.0f, 206.0f, 384.0f),
                       m_headerFormat,
                       D2D1::ColorF(0xF6FF83),
                       0.70f);
    DrawOutlinedString(stage.gimmick,
                       D2D1::RectF(224.0f, 354.0f, 476.0f, 384.0f),
                       m_bodyFormat,
                       D2D1::ColorF(0xF6FF83),
                       0.72f);
    FillRoundRect(D2D1::RectF(106.0f, 406.0f, 488.0f, 474.0f), 12.0f, D2D1::ColorF(0x0B1D28, 0.80f));
    DrawOutlinedString(L"적 구성",
                       D2D1::RectF(122.0f, 424.0f, 206.0f, 454.0f),
                       m_headerFormat,
                       D2D1::ColorF(0xFFB6C2),
                       0.70f);
    DrawOutlinedString(StageEnemySummary(),
                       D2D1::RectF(224.0f, 420.0f, 476.0f, 468.0f),
                       m_bodyFormat,
                       D2D1::ColorF(0xFFCAD1),
                       0.72f);
    DrawBalancePanel(D2D1::RectF(78.0f, 520.0f, 516.0f, 666.0f));

    const D2D1_RECT_F plan = D2D1::RectF(560.0f, 136.0f, 1202.0f, 474.0f);
    DrawBriefingPanel(plan, D2D1::ColorF(0x07131C, 0.98f), D2D1::ColorF(0x65B8FF));
    DrawPixelText(L"LOADOUT CHECK", {598.0f, 160.0f}, 3.15f, D2D1::ColorF(0xEAF7FF));
    DrawPixelText(L"CLICK SLOT TO EDIT", {600.0f, 198.0f}, 1.9f, D2D1::ColorF(0xC8D8E2));
    FillRoundRect(D2D1::RectF(plan.left + 34.0f, plan.top + 82.0f, plan.right - 34.0f, plan.top + 94.0f), 6.0f, D2D1::ColorF(0x0E2635, 0.92f));
    for (int i = 0; i < kLoadoutSize; ++i)
    {
        const D2D1_RECT_F rect = BriefingLoadoutSlotRect(i);
        const PlayerUnit unit = m_loadout[i];
        const UnitStats stats = PlayerStats(unit);
        const bool hover = Contains(rect, m_mouse);
        DrawCartoonPanel(rect, hover ? D2D1::ColorF(0x123044, 0.99f) : D2D1::ColorF(0x06131C, 0.99f), stats.accent, hover);
        DrawPlayerIcon(unit, {rect.left + 48.0f, rect.top + 42.0f}, 0.66f, true);
        FillRoundRect(D2D1::RectF(rect.right - 53.0f, rect.top + 8.0f, rect.right - 8.0f, rect.top + 29.0f),
                      5.0f,
                      D2D1::ColorF(UnitRoleColor(unit, stats).r, UnitRoleColor(unit, stats).g, UnitRoleColor(unit, stats).b, 0.22f));
        DrawPixelTextCentered(UnitRoleLabel(unit, stats),
                              D2D1::RectF(rect.right - 52.0f, rect.top + 8.0f, rect.right - 8.0f, rect.top + 29.0f),
                              0.72f,
                              UnitRoleColor(unit, stats),
                              0.96f);
        DrawPixelTextCentered(stats.name, D2D1::RectF(rect.left + 6.0f, rect.top + 72.0f, rect.right - 6.0f, rect.top + 94.0f), 1.18f, D2D1::ColorF(0xFFFFFF), 1.0f);
        FillRoundRect(D2D1::RectF(rect.left + 14.0f, rect.bottom - 26.0f, rect.right - 14.0f, rect.bottom - 5.0f), 6.0f, D2D1::ColorF(0x061019, hover ? 0.84f : 0.72f));
        DrawPixelTextCentered(L"KEY " + ToWideInt(i + 1), D2D1::RectF(rect.left + 12.0f, rect.bottom - 23.0f, rect.right - 12.0f, rect.bottom - 7.0f), 1.08f, D2D1::ColorF(0xF3FBFF), 1.0f);
    }
    FillRoundRect(D2D1::RectF(604.0f, 366.0f, 1160.0f, 424.0f), 6.0f, D2D1::ColorF(0x0B1D28, 0.90f));
    DrawString(L"적 기지 체력  " + ToWideInt(static_cast<int>(stage.enemyHp)), D2D1::RectF(622.0f, 374.0f, 836.0f, 400.0f), m_smallFormat, D2D1::ColorF(0xFFB6C2));
    DrawString(L"시작 에너지  " + ToWideInt(static_cast<int>(stage.startEnergy)), D2D1::RectF(862.0f, 374.0f, 1088.0f, 400.0f), m_smallFormat, D2D1::ColorF(0xB8FF89));
    DrawString(L"보스 첫 등장  " + ToWideInt(static_cast<int>(stage.bossFirstTime)) + L"초", D2D1::RectF(622.0f, 400.0f, 836.0f, 426.0f), m_smallFormat, D2D1::ColorF(0xFFB347));
    DrawString(L"이벤트 주기  " + ToWideInt(static_cast<int>(GimmickInterval())) + L"초", D2D1::RectF(862.0f, 400.0f, 1088.0f, 426.0f), m_smallFormat, D2D1::ColorF(0xF6FF83));
    DrawOutlinedString(CounterPlanSummary(),
                       D2D1::RectF(622.0f, 436.0f, 1156.0f, 464.0f),
                       m_smallFormat,
                       D2D1::ColorF(0xEAF7FF),
                       0.76f);

    const D2D1_RECT_F difficultyPanel = D2D1::RectF(604.0f, 616.0f, 1196.0f, 710.0f);
    DrawBriefingPanel(difficultyPanel, D2D1::ColorF(0x07131C, 0.90f), D2D1::ColorF(0x476779));
    DrawPixelText(L"DIFFICULTY", {622.0f, 628.0f}, 2.05f, D2D1::ColorF(0xEAF7FF));
    const std::array<std::wstring, 3> labels = {L"EASY", L"NORMAL", L"HARD"};
    for (int i = 0; i < 3; ++i)
    {
        const bool active = static_cast<int>(m_difficulty) == i;
        DrawBriefingButton(BriefingDifficultyRect(i), labels[i], true, active ? D2D1::ColorF(0x283B27) : D2D1::ColorF(0x202833), active);
    }
    DrawSynergyPanel(D2D1::RectF(604.0f, 536.0f, 1196.0f, 612.0f));

    DrawBriefingButton(BriefingBackButtonRect(), L"BACK", true, D2D1::ColorF(0x173C4B));
    DrawBriefingButton(BriefingShopButtonRect(), L"SHOP", true, D2D1::ColorF(0x4B4321));
    DrawBriefingButton(BriefingStartButtonRect(), L"LAUNCH", true, D2D1::ColorF(0x283B27));
    DrawPixelTextCentered(L"ENTER / SPACE", D2D1::RectF(BriefingStartButtonRect().left - 8.0f, 700.0f, BriefingStartButtonRect().right + 8.0f, 722.0f), 1.55f, D2D1::ColorF(0x8EA9B8), 1.0f);
}

void PawlineGameImpl::DrawShop()
{
    DrawDeepSpaceBackdrop(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), m_selectedStage, m_uiTime, 0.0f, false);
    for (float y = 30.0f; y < kHeight; y += 44.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.040f), 1.0f);
    }
    for (float x = 32.0f; x < kWidth; x += 48.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.026f), 1.0f);
    }

    DrawPixelText(L"UNIT SHOP", {48.0f, 42.0f}, 4.4f, D2D1::ColorF(0xF3FBFF));
    DrawPixelText(L"BUY LOCKED UNITS OR UPGRADE OWNED UNITS", {58.0f, 92.0f}, 2.1f, D2D1::ColorF(0x9AB2BF));
    DrawPixelText(L"LUMEN " + ToWideInt(m_lumen), {990.0f, 50.0f}, 3.0f, D2D1::ColorF(0xF6FF83));

    for (int i = 0; i < kRosterCount; ++i)
    {
        DrawShopUnitCard(i);
    }

    DrawShopUnitDetail();

    DrawButton(ShopBackButtonRect(), L"BACK", true, D2D1::ColorF(0x173C4B));
    DrawString(L"Esc / M", D2D1::RectF(ShopBackButtonRect().right + 12.0f, ShopBackButtonRect().top + 14.0f, ShopBackButtonRect().right + 120.0f, ShopBackButtonRect().bottom), m_bodyFormat, D2D1::ColorF(0x8EA9B8));
}

void PawlineGameImpl::DrawShopUnitCard(int index)
{
    const PlayerUnit unit = static_cast<PlayerUnit>(index);
    const UnitStats stats = PlayerStats(unit);
    const UnitStats base = GetPlayerStats(unit);
    const D2D1_RECT_F rect = ShopUnitRect(index);
    const bool unlocked = IsUnitUnlocked(unit);
    const bool hover = Contains(rect, m_mouse);
    const int level = UnitLevel(unit);
    const int cost = unlocked ? UnitUpgradeCost(unit) : UnitUnlockCost(unit);
    const bool maxed = unlocked && level >= kMaxUnitLevel;
    const bool affordable = cost == 0 || m_lumen >= cost;

    FillRoundRect(rect, 8.0f, hover ? D2D1::ColorF(0x142633) : D2D1::ColorF(0x0F1A22));
    StrokeRoundRect(rect, 8.0f, unlocked ? stats.accent : D2D1::ColorF(0x394955), hover ? 2.0f : 1.0f);
    DrawPlayerIcon(unit, {rect.left + 44.0f, rect.top + 43.0f}, 0.9f, unlocked);

    DrawString(base.name, D2D1::RectF(rect.left + 92.0f, rect.top + 14.0f, rect.right - 12.0f, rect.top + 40.0f), m_bodyFormat, unlocked ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x9AA8B0));
    DrawString(unlocked ? L"레벨 " + ToWideInt(level) + L" / " + ToWideInt(kMaxUnitLevel) : L"잠김", D2D1::RectF(rect.left + 92.0f, rect.top + 42.0f, rect.right - 12.0f, rect.top + 66.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2));
    DrawString(L"전투 비용 " + ToWideInt(stats.cost), D2D1::RectF(rect.left + 92.0f, rect.top + 66.0f, rect.right - 12.0f, rect.top + 90.0f), m_smallFormat, D2D1::ColorF(0xC7D8FF));

    std::wstring action = maxed ? L"최대" : (unlocked ? L"강화 " : L"구매 ");
    if (!maxed)
    {
        action += ToWideInt(cost);
    }
    DrawString(action, D2D1::RectF(rect.left + 92.0f, rect.bottom - 28.0f, rect.right - 12.0f, rect.bottom - 8.0f), m_smallFormat, maxed ? D2D1::ColorF(0xF6FF83) : (affordable ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0x7E919C)));
}

void PawlineGameImpl::DrawSynergyPanel(D2D1_RECT_F rect)
{
    // 편성 조합에 따라 활성화된 보너스를 한눈에 보여주는 패널이다.
    DrawBriefingPanel(rect, D2D1::ColorF(0x07131C, 0.98f), D2D1::ColorF(0xB8FF89));
    DrawPixelText(L"SYNERGY", {rect.left + 14.0f, rect.top + 12.0f}, 2.4f, D2D1::ColorF(0xB8FF89));
    DrawOutlinedString(SynergySummary(),
                       D2D1::RectF(rect.left + 188.0f, rect.top + 16.0f, rect.right - 18.0f, rect.bottom - 10.0f),
                       m_smallFormat,
                       D2D1::ColorF(0xF3FBFF),
                       0.72f);
}

void PawlineGameImpl::DrawBalancePanel(D2D1_RECT_F rect)
{
    // 스테이지 위협도와 현재 편성 전투력을 비교해 출격 전 판단을 돕는다.
    DrawBriefingPanel(rect, D2D1::ColorF(0x0F1A22, 0.96f), D2D1::ColorF(0xF6FF83));
    DrawPixelText(L"BALANCE", {rect.left + 14.0f, rect.top + 12.0f}, 2.35f, D2D1::ColorF(0xF6FF83));

    const float threat = StageThreatRating();
    const float power = LoadoutPowerRating();
    const float maxValue = std::max(1.0f, std::max(threat, power) * 1.18f);

    auto drawBar = [&](float y, const std::wstring& label, float value, D2D1_COLOR_F color) {
        DrawString(label, D2D1::RectF(rect.left + 18.0f, y - 2.0f, rect.left + 112.0f, y + 22.0f), m_smallFormat, D2D1::ColorF(0xD9E5F2));
        const D2D1_RECT_F back = D2D1::RectF(rect.left + 122.0f, y + 4.0f, rect.right - 52.0f, y + 18.0f);
        FillRoundRect(back, 5.0f, D2D1::ColorF(0x061019, 0.92f));
        FillRoundRect(D2D1::RectF(back.left, back.top, back.left + (back.right - back.left) * Clamp01(value / maxValue), back.bottom), 5.0f, color);
        DrawString(ToWideInt(static_cast<int>(std::round(value))), D2D1::RectF(rect.right - 48.0f, y - 2.0f, rect.right - 12.0f, y + 22.0f), m_smallFormat, D2D1::ColorF(0xF3FBFF));
    };

    drawBar(rect.top + 52.0f, L"위협", threat, D2D1::ColorF(0xFF9BA8));
    drawBar(rect.top + 82.0f, L"전력", power, D2D1::ColorF(0x65B8FF));
    DrawOutlinedString(BalanceAdvice(),
                       D2D1::RectF(rect.left + 18.0f, rect.bottom - 44.0f, rect.right - 18.0f, rect.bottom - 24.0f),
                       m_smallFormat,
                       D2D1::ColorF(0xFFFFFF),
                       0.74f);
    DrawOutlinedString(GrowthRecommendation(),
                       D2D1::RectF(rect.left + 18.0f, rect.bottom - 24.0f, rect.right - 18.0f, rect.bottom - 5.0f),
                       m_smallFormat,
                       D2D1::ColorF(0xF6FF83),
                       0.74f);
}

void PawlineGameImpl::DrawShopUnitDetail()
{
    const int index = std::max(0, std::min(kRosterCount - 1, m_shopSelectedUnit));
    const PlayerUnit unit = static_cast<PlayerUnit>(index);
    const UnitStats stats = PlayerStats(unit);
    const UnitStats base = GetPlayerStats(unit);
    const bool unlocked = IsUnitUnlocked(unit);
    const D2D1_RECT_F panel = D2D1::RectF(784.0f, 82.0f, 1226.0f, 144.0f);

    // 상점 카드만으로 부족한 수치를 보완하는 상세 정보 패널이다.
    DrawCartoonPanel(panel, D2D1::ColorF(0x0F1A22, 0.97f), unlocked ? stats.accent : D2D1::ColorF(0x394955));
    DrawPlayerIcon(unit, {panel.left + 34.0f, panel.top + 31.0f}, 0.50f, unlocked);
    DrawPixelTextCentered(base.name, D2D1::RectF(panel.left + 70.0f, panel.top + 8.0f, panel.left + 252.0f, panel.top + 32.0f), 1.75f, unlocked ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x9AA8B0), 1.0f);
    DrawPixelText(unlocked ? L"LV." + ToWideInt(UnitLevel(unit)) : L"LOCKED", {panel.left + 72.0f, panel.top + 36.0f}, 1.55f, unlocked ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2));
    DrawPixelText(L"HP " + ToWideInt(static_cast<int>(stats.hp)), {panel.left + 170.0f, panel.top + 36.0f}, 1.55f, D2D1::ColorF(0xC7D8FF));
    DrawPixelText(L"DMG " + ToWideInt(static_cast<int>(stats.damage)), {panel.left + 254.0f, panel.top + 36.0f}, 1.55f, D2D1::ColorF(0xFFB6C2));
    DrawPixelText(stats.ranged ? L"RANGED" : L"FRONT", {panel.left + 358.0f, panel.top + 36.0f}, 1.55f, D2D1::ColorF(0xEAF7FF));
}

void PawlineGameImpl::DrawStageCard(int index)
{
    const StageDefinition stage = GetStageDefinition(index);
    const D2D1_RECT_F rect = MenuStageRect(index);
    const bool active = index == m_selectedStage;
    const bool hover = Contains(rect, m_mouse);
    const bool unlocked = IsStageUnlocked(index);
    FillRoundRect(rect, 8.0f, active ? D2D1::ColorF(0x142A35) : D2D1::ColorF(0x0E1A22));
    StrokeRoundRect(rect, 8.0f, unlocked ? (active ? stage.lineColor : D2D1::ColorF(0x2C4A5B)) : D2D1::ColorF(0x33404A), hover || active ? 2.0f : 1.0f);
    FillRoundRect(D2D1::RectF(rect.left + 12.0f, rect.top + 15.0f, rect.left + 68.0f, rect.bottom - 15.0f), 8.0f, unlocked ? stage.laneColor : D2D1::ColorF(0x26313A));
    DrawLine({rect.left + 22.0f, rect.top + 42.0f}, {rect.left + 58.0f, rect.top + 42.0f}, stage.lineColor, 3.5f);
    DrawString(stage.name, D2D1::RectF(rect.left + 84.0f, rect.top + 12.0f, rect.right - 10.0f, rect.top + 38.0f), m_headerFormat, unlocked ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x73818A));
    DrawString(unlocked ? stage.subtitle : L"이전 행성 클리어 필요", D2D1::RectF(rect.left + 84.0f, rect.top + 38.0f, rect.right - 10.0f, rect.top + 60.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xAFC2CD) : D2D1::ColorF(0xFFB6C2));
    const std::wstring stageLabel = index == 9 ? L"Stage 0" : L"Stage " + ToWideInt(index + 1);
    if (unlocked)
    {
        DrawString(stageLabel, D2D1::RectF(rect.left + 84.0f, rect.bottom - 24.0f, rect.right - 10.0f, rect.bottom - 6.0f), m_smallFormat, active ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0x7E919C));
    }
    if (!unlocked)
    {
        FillRoundRect(rect, 8.0f, D2D1::ColorF(0x000000, 0.22f));
        FillRoundRect(D2D1::RectF(rect.right - 66.0f, rect.bottom - 27.0f, rect.right - 12.0f, rect.bottom - 8.0f), 5.0f, D2D1::ColorF(0x180D13, 0.78f));
        StrokeRoundRect(D2D1::RectF(rect.right - 66.0f, rect.bottom - 27.0f, rect.right - 12.0f, rect.bottom - 8.0f), 5.0f, D2D1::ColorF(0xFFB6C2, 0.48f), 1.0f);
        DrawPixelTextCentered(L"LOCK", D2D1::RectF(rect.right - 63.0f, rect.bottom - 25.0f, rect.right - 15.0f, rect.bottom - 10.0f), 1.25f, D2D1::ColorF(0xFFB6C2), 1.0f);
        DrawLine({rect.left + 32.0f, rect.top + 31.0f}, {rect.left + 48.0f, rect.top + 31.0f}, D2D1::ColorF(0xFFB6C2, 0.70f), 3.0f);
        StrokeRoundRect(D2D1::RectF(rect.left + 25.0f, rect.top + 32.0f, rect.left + 55.0f, rect.top + 54.0f), 5.0f, D2D1::ColorF(0xFFB6C2, 0.70f), 2.0f);
    }
}

void PawlineGameImpl::DrawLoadoutSlot(int index)
{
    const D2D1_RECT_F rect = MenuLoadoutSlotRect(index);
    const bool active = index == m_selectedLoadoutSlot;
    const UnitStats stats = PlayerStats(m_loadout[index]);
    FillRoundRect(rect, 8.0f, active ? D2D1::ColorF(0x152A35) : D2D1::ColorF(0x0E1A22));
    StrokeRoundRect(rect, 8.0f, active ? stats.accent : D2D1::ColorF(0x2C4A5B), active ? 2.2f : 1.0f);
    DrawPlayerIcon(m_loadout[index], {rect.left + 51.0f, rect.top + 34.0f}, 0.82f, true);
    DrawString(stats.name, D2D1::RectF(rect.left + 8.0f, rect.top + 66.0f, rect.right - 8.0f, rect.top + 92.0f), m_smallFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"레벨 " + ToWideInt(UnitLevel(m_loadout[index])) + L"  KEY " + ToWideInt(index + 1), D2D1::RectF(rect.left + 8.0f, rect.bottom - 25.0f, rect.right - 8.0f, rect.bottom - 4.0f), m_smallFormat, D2D1::ColorF(0xCFE8F5));
}

void PawlineGameImpl::DrawRosterCard(int index)
{
    const PlayerUnit unit = static_cast<PlayerUnit>(index);
    const UnitStats stats = PlayerStats(unit);
    const D2D1_RECT_F rect = RosterCardRect(index);
    const bool unlocked = IsUnitUnlocked(unit);
    const bool picked = IsUnitInLoadout(unit);
    const bool hover = Contains(rect, m_mouse);
    FillRoundRect(rect, 8.0f, hover ? D2D1::ColorF(0x142633) : D2D1::ColorF(0x0F1A22));
    StrokeRoundRect(rect, 8.0f, picked ? stats.accent : D2D1::ColorF(0x2C4A5B), picked || hover ? 2.0f : 1.0f);
    if (!unlocked)
    {
        FillRoundRect(rect, 8.0f, D2D1::ColorF(0x000000, 0.24f));
    }
    DrawPlayerIcon(unit, {rect.left + 34.0f, rect.top + 39.0f}, 0.62f, unlocked);
    DrawString(stats.name, D2D1::RectF(rect.left + 62.0f, rect.top + 16.0f, rect.right - 8.0f, rect.top + 44.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x73818A));
    DrawString(unlocked ? L"레벨 " + ToWideInt(UnitLevel(unit)) : L"잠김", D2D1::RectF(rect.left + 62.0f, rect.top + 48.0f, rect.right - 8.0f, rect.top + 72.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2));
    if (picked)
    {
        DrawString(L"편성", D2D1::RectF(rect.left + 8.0f, rect.bottom - 26.0f, rect.left + 54.0f, rect.bottom - 4.0f), m_smallFormat, stats.accent);
    }
}

void PawlineGameImpl::DrawArena()
{
    const StageDefinition stage = CurrentStage();
    FillRect(D2D1::RectF(0.0f, 0.0f, kWorldWidth, kHeight), D2D1::ColorF(0x071421));
    FillRect(D2D1::RectF(24.0f, kBattleTop, kWorldWidth - 24.0f, kBattleBottom), D2D1::ColorF(0x0D1D2B));
    DrawSpaceBackdrop();

    for (float y = kBattleTop + 22.0f; y < kBattleBottom; y += 38.0f)
    {
        DrawLine({34.0f, y}, {kWorldWidth - 34.0f, y}, D2D1::ColorF(0x27475B, 0.055f), 1.0f);
    }
    for (float x = 42.0f; x < kWorldWidth - 30.0f; x += 46.0f)
    {
        DrawLine({x, kBattleTop + 10.0f}, {x, kBattleBottom - 10.0f}, D2D1::ColorF(0x27475B, 0.032f), 1.0f);
    }

    FillRoundRect(D2D1::RectF(48.0f, kLaneY - kLaneHalfHeight, kWorldWidth - 48.0f, kLaneY + kLaneHalfHeight), 18.0f, D2D1::ColorF(stage.laneColor.r, stage.laneColor.g, stage.laneColor.b, 0.82f));
    FillRoundRect(D2D1::RectF(58.0f, kLaneY - kLaneHalfHeight + 14.0f, kWorldWidth - 58.0f, kLaneY + kLaneHalfHeight - 14.0f), 16.0f, D2D1::ColorF(stage.laneInnerColor.r + 0.02f, stage.laneInnerColor.g + 0.025f, stage.laneInnerColor.b + 0.035f, 0.90f));
    FillRoundRect(D2D1::RectF(58.0f, kLaneY - kLaneHalfHeight + 14.0f, kWorldWidth - 58.0f, kLaneY - 14.0f), 16.0f, D2D1::ColorF(0xFFFFFF, 0.035f));
    FillRoundRect(D2D1::RectF(58.0f, kLaneY + 18.0f, kWorldWidth - 58.0f, kLaneY + kLaneHalfHeight - 14.0f), 16.0f, D2D1::ColorF(0x000000, 0.12f));
    DrawLine({96.0f, kLaneY}, {kEnemyBaseX + 16.0f, kLaneY}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.62f), 3.0f);
    DrawStageLanePattern();

    if (m_cannonFlash > 0.0f)
    {
        const float alpha = Clamp01(m_cannonFlash / 0.42f);
        const float flashScale = m_reduceFlashes ? 0.46f : 1.0f;
        FillRoundRect(D2D1::RectF(108.0f, kLaneY - 40.0f, kEnemyBaseX + 2.0f, kLaneY + 40.0f), 32.0f, D2D1::ColorF(0xF6FF83, 0.24f * alpha * flashScale));
        DrawLine({116.0f, kLaneY}, {kEnemyBaseX - 2.0f, kLaneY}, D2D1::ColorF(0xF6FF83, 0.88f * alpha * (m_reduceFlashes ? 0.70f : 1.0f)), 11.0f);
    }

    StrokeRoundRect(D2D1::RectF(24.0f, kBattleTop, kWorldWidth - 24.0f, kBattleBottom), 8.0f, D2D1::ColorF(0x2B4A5B), 1.2f);
}

void PawlineGameImpl::DrawSpaceBackdrop()
{
    const D2D1_RECT_F arena = D2D1::RectF(24.0f, kBattleTop, kWorldWidth - 24.0f, kBattleBottom);

    DrawDeepSpaceBackdrop(arena, m_selectedStage, m_stageTime, m_cameraX, false);

    for (int i = 0; i < 8; ++i)
    {
        const float y = kBattleTop + 44.0f + static_cast<float>(i) * 44.0f;
        const float wave = std::sin(m_stageTime * 0.38f + static_cast<float>(i) * 0.77f) * 28.0f;
        DrawLine({60.0f + wave, y}, {kWorldWidth - 76.0f + wave * 0.35f, y + 26.0f}, D2D1::ColorF(0xCFE8F5, 0.010f), 3.0f);
    }

    FillRoundRect(D2D1::RectF(48.0f, kLaneY - kLaneHalfHeight - 20.0f, kWorldWidth - 48.0f, kLaneY + kLaneHalfHeight + 22.0f), 22.0f, D2D1::ColorF(0xFFFFFF, 0.018f));
    FillRect(D2D1::RectF(arena.left, arena.top, arena.right, arena.top + 76.0f), D2D1::ColorF(0xFFFFFF, 0.028f));
    FillRect(D2D1::RectF(arena.left, arena.bottom - 100.0f, arena.right, arena.bottom), D2D1::ColorF(0x000000, 0.13f));
}

void PawlineGameImpl::DrawCrater(Vec2 center, float rx, float ry, D2D1_COLOR_F rim, D2D1_COLOR_F shade)
{
    const float visualRy = std::max(ry, rx * 0.62f);
    FillEllipse(center, rx, visualRy, shade);
    StrokeEllipse(center, rx, visualRy, rim, 1.3f);
    FillEllipse({center.x - rx * 0.25f, center.y - visualRy * 0.22f}, rx * 0.34f, visualRy * 0.28f, D2D1::ColorF(0xFFFFFF, 0.08f));
}

void PawlineGameImpl::DrawCloudCluster(Vec2 center, float scale, D2D1_COLOR_F color)
{
    FillEllipse({center.x - 28.0f * scale, center.y + 2.0f * scale}, 30.0f * scale, 12.0f * scale, color);
    FillEllipse({center.x, center.y - 8.0f * scale}, 34.0f * scale, 15.0f * scale, color);
    FillEllipse({center.x + 32.0f * scale, center.y + 1.0f * scale}, 28.0f * scale, 11.0f * scale, color);
}

void PawlineGameImpl::DrawStageDecorations()
{
    const float drift = std::sin(m_stageTime * 0.45f) * 6.0f;
    const StageDefinition stage = CurrentStage();
    DrawString(stage.name, D2D1::RectF(1018.0f, 116.0f, 1226.0f, 158.0f), m_headerFormat, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.36f));

    switch (m_selectedStage)
    {
    case 0:
        DrawCrater({184.0f, 172.0f}, 52.0f, 22.0f, D2D1::ColorF(0xB6C0C6, 0.42f), D2D1::ColorF(0x030506, 0.22f));
        DrawCrater({430.0f, 526.0f}, 64.0f, 26.0f, D2D1::ColorF(0xAAB5BC, 0.34f), D2D1::ColorF(0x000000, 0.18f));
        DrawCrater({984.0f, 198.0f}, 44.0f, 18.0f, D2D1::ColorF(0xC3CDD2, 0.38f), D2D1::ColorF(0x000000, 0.20f));
        DrawCrater({1120.0f, 510.0f}, 78.0f, 28.0f, D2D1::ColorF(0xB6C0C6, 0.32f), D2D1::ColorF(0x030506, 0.18f));
        DrawLine({236.0f, 228.0f}, {338.0f, 278.0f}, D2D1::ColorF(0xC7D2D8, 0.28f), 2.0f);
        DrawLine({330.0f, 278.0f}, {382.0f, 254.0f}, D2D1::ColorF(0xC7D2D8, 0.22f), 1.5f);
        DrawLine({814.0f, 520.0f}, {932.0f, 476.0f}, D2D1::ColorF(0xC7D2D8, 0.24f), 1.8f);
        break;
    case 1:
        FillRoundRect(D2D1::RectF(54.0f, 142.0f + drift, 1230.0f, 184.0f + drift), 20.0f, D2D1::ColorF(0xC98653, 0.22f));
        FillRoundRect(D2D1::RectF(108.0f, 224.0f - drift, 1172.0f, 260.0f - drift), 18.0f, D2D1::ColorF(0xF0C06E, 0.18f));
        FillRoundRect(D2D1::RectF(42.0f, 496.0f + drift * 0.6f, 1228.0f, 542.0f + drift * 0.6f), 22.0f, D2D1::ColorF(0xE68A7A, 0.14f));
        DrawCloudCluster({260.0f + drift, 202.0f}, 1.0f, D2D1::ColorF(0xFFD27A, 0.18f));
        DrawCloudCluster({876.0f - drift, 520.0f}, 1.25f, D2D1::ColorF(0xFF9BA8, 0.13f));
        break;
    case 2:
        FillEllipse({220.0f, 186.0f}, 126.0f, 46.0f, D2D1::ColorF(0x2E7D5B, 0.30f));
        FillEllipse({1030.0f, 486.0f}, 150.0f, 58.0f, D2D1::ColorF(0x4BAA75, 0.26f));
        FillEllipse({852.0f, 172.0f}, 110.0f, 38.0f, D2D1::ColorF(0x2B9B8D, 0.24f));
        DrawCloudCluster({384.0f + drift, 146.0f}, 0.82f, D2D1::ColorF(0xFFFFFF, 0.18f));
        DrawCloudCluster({734.0f - drift, 548.0f}, 1.0f, D2D1::ColorF(0xEAF7FF, 0.14f));
        DrawLine({88.0f, 520.0f}, {310.0f, 452.0f}, D2D1::ColorF(0x65B8FF, 0.22f), 2.0f);
        DrawLine({930.0f, 168.0f}, {1198.0f, 232.0f}, D2D1::ColorF(0x65B8FF, 0.20f), 2.0f);
        break;
    case 3:
        for (int i = 0; i < 6; ++i)
        {
            const float y = 142.0f + static_cast<float>(i) * 74.0f;
            DrawLine({54.0f, y + std::sin(static_cast<float>(i)) * 9.0f}, {1228.0f, y + 36.0f}, D2D1::ColorF(0xE08A55, 0.18f), 5.0f);
        }
        FillEllipse({274.0f, 512.0f}, 78.0f, 22.0f, D2D1::ColorF(0x6D2D28, 0.30f));
        FillEllipse({998.0f, 172.0f}, 96.0f, 24.0f, D2D1::ColorF(0x8A3A2F, 0.24f));
        DrawLine({580.0f, 132.0f}, {628.0f, 208.0f}, D2D1::ColorF(0xFFB191, 0.28f), 2.0f);
        DrawLine({628.0f, 208.0f}, {590.0f, 266.0f}, D2D1::ColorF(0xFFB191, 0.20f), 1.6f);
        break;
    case 4:
        FillRoundRect(D2D1::RectF(34.0f, 124.0f, 1246.0f, 164.0f), 18.0f, D2D1::ColorF(0xD8A66A, 0.18f));
        FillRoundRect(D2D1::RectF(42.0f, 198.0f, 1238.0f, 240.0f), 20.0f, D2D1::ColorF(0xF1D09A, 0.14f));
        FillRoundRect(D2D1::RectF(36.0f, 486.0f, 1242.0f, 530.0f), 21.0f, D2D1::ColorF(0xA66445, 0.18f));
        FillEllipse({948.0f + drift, 214.0f}, 118.0f, 38.0f, D2D1::ColorF(0xB74B3D, 0.30f));
        StrokeEllipse({948.0f + drift, 214.0f}, 118.0f, 38.0f, D2D1::ColorF(0xFFD7A5, 0.34f), 2.4f);
        FillEllipse({950.0f + drift, 214.0f}, 52.0f, 16.0f, D2D1::ColorF(0xFFE2B5, 0.16f));
        break;
    case 5:
        for (int i = 0; i < 8; ++i)
        {
            const float offset = static_cast<float>(i) * 42.0f;
            DrawLine({40.0f, 186.0f + offset}, {1240.0f, 56.0f + offset}, D2D1::ColorF(0xD7C48A, 0.15f), 3.0f);
        }
        StrokeEllipse({650.0f, 352.0f}, 442.0f, 96.0f, D2D1::ColorF(0xE6D392, 0.24f), 5.0f);
        StrokeEllipse({650.0f, 352.0f}, 368.0f, 70.0f, D2D1::ColorF(0xF1E5B9, 0.14f), 3.0f);
        FillEllipse({1024.0f, 156.0f}, 72.0f, 24.0f, D2D1::ColorF(0xCDBB83, 0.18f));
        break;
    case 6:
        for (int i = 0; i < 7; ++i)
        {
            const float x = 112.0f + static_cast<float>(i) * 178.0f;
            DrawLine({x - 112.0f, 602.0f}, {x + 82.0f, 108.0f}, D2D1::ColorF(0x80E5D4, 0.17f), 4.0f);
        }
        FillEllipse({246.0f, 188.0f}, 64.0f, 18.0f, D2D1::ColorF(0xB9FFF5, 0.16f));
        FillEllipse({940.0f, 512.0f}, 112.0f, 25.0f, D2D1::ColorF(0x5EDDCB, 0.16f));
        DrawLine({868.0f, 150.0f}, {896.0f, 190.0f}, D2D1::ColorF(0xD9FFF8, 0.34f), 2.0f);
        DrawLine({896.0f, 190.0f}, {856.0f, 212.0f}, D2D1::ColorF(0xD9FFF8, 0.26f), 2.0f);
        break;
    case 7:
        for (int i = 0; i < 5; ++i)
        {
            const float y = 148.0f + static_cast<float>(i) * 88.0f + drift * 0.5f;
            DrawLine({54.0f, y}, {350.0f, y + 34.0f}, D2D1::ColorF(0x75A7FF, 0.18f), 4.0f);
            DrawLine({350.0f, y + 34.0f}, {726.0f, y - 22.0f}, D2D1::ColorF(0x75A7FF, 0.16f), 4.0f);
            DrawLine({726.0f, y - 22.0f}, {1228.0f, y + 28.0f}, D2D1::ColorF(0x75A7FF, 0.14f), 4.0f);
        }
        StrokeEllipse({930.0f, 198.0f}, 108.0f, 52.0f, D2D1::ColorF(0xBFD9FF, 0.28f), 3.0f);
        StrokeEllipse({930.0f, 198.0f}, 54.0f, 25.0f, D2D1::ColorF(0x75A7FF, 0.24f), 2.0f);
        FillEllipse({930.0f, 198.0f}, 18.0f, 9.0f, D2D1::ColorF(0xE7F0FF, 0.20f));
        break;
    case 8:
        FillEllipse({254.0f, 206.0f}, 42.0f, 42.0f, D2D1::ColorF(0xF4E0D5, 0.12f));
        FillEllipse({292.0f, 206.0f}, 42.0f, 42.0f, D2D1::ColorF(0xF4E0D5, 0.12f));
        FillEllipse({273.0f, 246.0f}, 54.0f, 34.0f, D2D1::ColorF(0xF4E0D5, 0.12f));
        for (int i = 0; i < 16; ++i)
        {
            const float x = 84.0f + static_cast<float>((i * 173) % 1110);
            const float y = 122.0f + static_cast<float>((i * 71) % 450);
            FillEllipse({x, y}, 2.3f + static_cast<float>(i % 3), 2.3f + static_cast<float>(i % 3), D2D1::ColorF(0xEAE6FF, 0.24f));
        }
        DrawCrater({1030.0f, 510.0f}, 84.0f, 28.0f, D2D1::ColorF(0xC8B7FF, 0.26f), D2D1::ColorF(0x000000, 0.20f));
        break;
    default:
        FillEllipse({640.0f, 352.0f}, 360.0f, 210.0f, D2D1::ColorF(0xFFB347, 0.08f));
        FillEllipse({640.0f, 352.0f}, 220.0f, 126.0f, D2D1::ColorF(0xFF7A39, 0.11f));
        for (int i = 0; i < 12; ++i)
        {
            const float angle = static_cast<float>(i) * kPi / 6.0f + m_stageTime * 0.12f;
            const Vec2 inner = {640.0f + std::cos(angle) * 150.0f, 352.0f + std::sin(angle) * 82.0f};
            const Vec2 outer = {640.0f + std::cos(angle) * 570.0f, 352.0f + std::sin(angle) * 310.0f};
            DrawLine(inner, outer, D2D1::ColorF(0xFFB347, 0.18f), 5.0f);
        }
        FillRoundRect(D2D1::RectF(84.0f, 146.0f, 330.0f, 184.0f), 18.0f, D2D1::ColorF(0xFFE66D, 0.15f));
        FillRoundRect(D2D1::RectF(920.0f, 506.0f, 1190.0f, 548.0f), 20.0f, D2D1::ColorF(0xFF6A3D, 0.18f));
        break;
    }
}

void PawlineGameImpl::DrawLongRangeDecorations()
{
    const float drift = std::sin(m_stageTime * 0.36f) * 7.0f;

    for (int i = 0; i < 5; ++i)
    {
        const float x = 340.0f + static_cast<float>(i) * 430.0f;
        const float upper = 142.0f + static_cast<float>((i * 53) % 84);
        const float lower = 492.0f - static_cast<float>((i * 37) % 74);
        const float pulse = 0.65f + 0.35f * std::sin(m_stageTime * 1.3f + static_cast<float>(i));

        switch (m_selectedStage)
        {
        case 0:
            DrawCrater({x + 130.0f, upper + 18.0f}, 42.0f + static_cast<float>(i % 3) * 10.0f, 16.0f, D2D1::ColorF(0xC3CDD2, 0.24f), D2D1::ColorF(0x000000, 0.16f));
            DrawLine({x + 18.0f, lower}, {x + 168.0f, lower - 34.0f}, D2D1::ColorF(0xC7D2D8, 0.18f), 1.7f);
            break;
        case 1:
            FillRoundRect(D2D1::RectF(x - 90.0f, upper + drift, x + 240.0f, upper + 33.0f + drift), 15.0f, D2D1::ColorF(0xFFD27A, 0.12f + 0.04f * pulse));
            FillRoundRect(D2D1::RectF(x + 48.0f, lower - drift, x + 350.0f, lower + 28.0f - drift), 14.0f, D2D1::ColorF(0xFF9BA8, 0.10f));
            break;
        case 2:
            FillEllipse({x + 120.0f, upper + 12.0f}, 92.0f, 28.0f, D2D1::ColorF(0x4BAA75, 0.16f));
            DrawCloudCluster({x + 20.0f + drift, lower}, 0.72f, D2D1::ColorF(0xFFFFFF, 0.12f));
            break;
        case 3:
            DrawLine({x - 130.0f, upper}, {x + 310.0f, upper + 44.0f}, D2D1::ColorF(0xFF8B60, 0.13f), 4.0f);
            DrawLine({x + 40.0f, lower}, {x + 210.0f, lower - 58.0f}, D2D1::ColorF(0xFFB191, 0.20f), 2.0f);
            break;
        case 4:
            FillRoundRect(D2D1::RectF(x - 140.0f, upper, x + 310.0f, upper + 26.0f), 13.0f, D2D1::ColorF(0xF1D09A, 0.11f));
            FillEllipse({x + 225.0f + drift, lower - 14.0f}, 80.0f, 24.0f, D2D1::ColorF(0xB74B3D, 0.19f));
            break;
        case 5:
            StrokeEllipse({x + 120.0f, kLaneY - 8.0f}, 210.0f, 38.0f, D2D1::ColorF(0xE6D392, 0.12f), 2.2f);
            DrawLine({x - 160.0f, upper + 70.0f}, {x + 360.0f, upper + 8.0f}, D2D1::ColorF(0xD7C48A, 0.12f), 2.6f);
            break;
        case 6:
            DrawLine({x - 110.0f, kBattleBottom - 18.0f}, {x + 90.0f, kBattleTop + 18.0f}, D2D1::ColorF(0x80E5D4, 0.13f), 3.2f);
            FillEllipse({x + 220.0f, lower}, 76.0f, 20.0f, D2D1::ColorF(0xB9FFF5, 0.13f));
            break;
        case 7:
            StrokeEllipse({x + 160.0f, upper + 28.0f}, 74.0f, 32.0f, D2D1::ColorF(0xBFD9FF, 0.18f), 2.2f);
            DrawLine({x - 100.0f, lower}, {x + 330.0f, lower + std::sin(static_cast<float>(i)) * 32.0f}, D2D1::ColorF(0x75A7FF, 0.12f), 4.0f);
            break;
        case 8:
            FillEllipse({x + 70.0f, upper}, 3.0f + static_cast<float>(i % 3), 3.0f + static_cast<float>(i % 3), D2D1::ColorF(0xEAE6FF, 0.26f));
            DrawCrater({x + 240.0f, lower}, 62.0f, 18.0f, D2D1::ColorF(0xC8B7FF, 0.17f), D2D1::ColorF(0x000000, 0.13f));
            break;
        default:
            FillEllipse({x + 80.0f, kLaneY}, 138.0f, 36.0f, D2D1::ColorF(0xFFB347, 0.08f + 0.03f * pulse));
            DrawLine({x - 100.0f, lower}, {x + 340.0f, upper}, D2D1::ColorF(0xFFE66D, 0.13f), 4.0f);
            break;
        }

        // 레인 위아래 보조선은 이동 경로처럼 보이기 쉬워 제거하고, 배경 장식만 남긴다.
    }
}

void PawlineGameImpl::DrawStageLanePattern()
{
    switch (m_selectedStage)
    {
    case 0:
        DrawCrater({332.0f, 320.0f}, 34.0f, 13.0f, D2D1::ColorF(0xBFC9CE, 0.22f), D2D1::ColorF(0x000000, 0.12f));
        DrawCrater({828.0f, 408.0f}, 46.0f, 15.0f, D2D1::ColorF(0xBFC9CE, 0.18f), D2D1::ColorF(0x000000, 0.10f));
        break;
    case 1:
        FillEllipse({310.0f, 318.0f}, 78.0f, 18.0f, D2D1::ColorF(0xFFD27A, 0.12f));
        FillEllipse({952.0f, 414.0f}, 112.0f, 22.0f, D2D1::ColorF(0xFF9BA8, 0.10f));
        break;
    case 2:
        FillEllipse({356.0f, 398.0f}, 88.0f, 24.0f, D2D1::ColorF(0x4BAA75, 0.18f));
        FillEllipse({906.0f, 312.0f}, 110.0f, 26.0f, D2D1::ColorF(0x2B9B8D, 0.15f));
        break;
    case 3:
        FillEllipse({284.0f, 306.0f}, 96.0f, 20.0f, D2D1::ColorF(0xFF8B60, 0.12f));
        FillEllipse({916.0f, 424.0f}, 118.0f, 24.0f, D2D1::ColorF(0x6D2D28, 0.13f));
        break;
    case 4:
        FillEllipse({390.0f, 292.0f}, 142.0f, 24.0f, D2D1::ColorF(0xEAC089, 0.12f));
        FillEllipse({898.0f, 426.0f}, 166.0f, 28.0f, D2D1::ColorF(0xA66445, 0.11f));
        break;
    case 5:
        StrokeEllipse({640.0f, kLaneY}, 470.0f, 74.0f, D2D1::ColorF(0xE6D392, 0.20f), 3.0f);
        StrokeEllipse({640.0f, kLaneY}, 336.0f, 52.0f, D2D1::ColorF(0xF1E5B9, 0.12f), 2.0f);
        break;
    case 6:
        FillEllipse({246.0f, 304.0f}, 72.0f, 20.0f, D2D1::ColorF(0xB9FFF5, 0.12f));
        FillEllipse({856.0f, 418.0f}, 92.0f, 22.0f, D2D1::ColorF(0x80E5D4, 0.12f));
        break;
    case 7:
        StrokeEllipse({464.0f, kLaneY}, 144.0f, 44.0f, D2D1::ColorF(0x75A7FF, 0.16f), 3.0f);
        StrokeEllipse({900.0f, kLaneY}, 174.0f, 48.0f, D2D1::ColorF(0xBFD9FF, 0.13f), 3.0f);
        break;
    case 8:
        FillEllipse({486.0f, 324.0f}, 62.0f, 18.0f, D2D1::ColorF(0xEAE6FF, 0.13f));
        FillEllipse({796.0f, 414.0f}, 92.0f, 24.0f, D2D1::ColorF(0xC8B7FF, 0.12f));
        break;
    default:
        FillEllipse({646.0f, kLaneY}, 154.0f, 42.0f, D2D1::ColorF(0xFFB347, 0.13f));
        break;
    }
}

void PawlineGameImpl::DrawBases()
{
    const StageDefinition stage = CurrentStage();
    const D2D1_COLOR_F enemyStageColor = stage.lineColor;
    const float playerShake = m_playerBaseShake > 0.0f ? std::sin(m_stageTime * 94.0f) * 4.0f : 0.0f;
    const float enemyShake = m_enemyBaseShake > 0.0f ? std::sin(m_stageTime * 94.0f) * 4.0f : 0.0f;
    Vec2 player = {kPlayerBaseX + playerShake, kLaneY};
    Vec2 enemy = {kEnemyBaseX + enemyShake, kLaneY};

    const float playerPulse = 0.55f + 0.45f * std::sin(m_stageTime * 2.2f);
    const float enemyPulse = 0.55f + 0.45f * std::sin(m_stageTime * 2.0f + 1.7f);
    FillEllipse({player.x + 48.0f, player.y + 72.0f}, 124.0f, 28.0f, D2D1::ColorF(0x000000, 0.23f));
    FillEllipse({player.x + 88.0f, player.y + 88.0f}, 96.0f, 18.0f, D2D1::ColorF(0x000000, 0.13f));
    FillEllipse({enemy.x - 52.0f, enemy.y + 74.0f}, 132.0f, 30.0f, D2D1::ColorF(0x000000, 0.26f));
    FillEllipse({enemy.x - 96.0f, enemy.y + 90.0f}, 102.0f, 18.0f, D2D1::ColorF(0x000000, 0.15f));
    FillEllipse({player.x, player.y - 4.0f}, 138.0f, 128.0f, D2D1::ColorF(0x65B8FF, 0.050f + 0.030f * playerPulse));
    FillEllipse({player.x + 16.0f, player.y - 10.0f}, 92.0f, 82.0f, D2D1::ColorF(0xF3FBFF, 0.032f));
    FillEllipse({enemy.x, enemy.y - 4.0f}, 146.0f, 132.0f, D2D1::ColorF(0xFF9BA8, 0.055f + 0.030f * enemyPulse));
    FillEllipse({enemy.x - 16.0f, enemy.y - 10.0f}, 96.0f, 86.0f, D2D1::ColorF(0xFF4A6E, 0.032f));

    FillEllipse({player.x, player.y + 60.0f}, 66.0f, 17.0f, D2D1::ColorF(0x000000, 0.28f));
    FillRoundRect(D2D1::RectF(player.x - 56.0f, player.y - 70.0f, player.x + 56.0f, player.y + 58.0f), 10.0f, D2D1::ColorF(0xF4FBFF));
    StrokeRoundRect(D2D1::RectF(player.x - 56.0f, player.y - 70.0f, player.x + 56.0f, player.y + 58.0f), 10.0f, D2D1::ColorF(0x65B8FF), 2.5f);
    FillEllipse({player.x - 23.0f, player.y - 16.0f}, 5.0f, 7.0f, D2D1::ColorF(0x071017));
    FillEllipse({player.x + 23.0f, player.y - 16.0f}, 5.0f, 7.0f, D2D1::ColorF(0x071017));
    DrawLine({player.x - 18.0f, player.y + 14.0f}, {player.x + 18.0f, player.y + 14.0f}, D2D1::ColorF(0x071017), 2.0f);
    DrawLine({player.x + 48.0f, player.y - 58.0f}, {player.x + 72.0f, player.y - 76.0f}, D2D1::ColorF(0x65B8FF), 7.0f);

    FillEllipse({enemy.x, enemy.y + 60.0f}, 70.0f, 18.0f, D2D1::ColorF(0x000000, 0.32f));
    FillRoundRect(D2D1::RectF(enemy.x - 64.0f, enemy.y - 74.0f, enemy.x + 64.0f, enemy.y + 62.0f), 10.0f, D2D1::ColorF(0x1B2028));
    StrokeRoundRect(D2D1::RectF(enemy.x - 64.0f, enemy.y - 74.0f, enemy.x + 64.0f, enemy.y + 62.0f), 10.0f, D2D1::ColorF(enemyStageColor.r, enemyStageColor.g, enemyStageColor.b, 0.78f), 2.7f);
    FillRoundRect(D2D1::RectF(enemy.x - 50.0f, enemy.y + 22.0f, enemy.x + 50.0f, enemy.y + 47.0f), 6.0f, D2D1::ColorF(0x071017, 0.80f));
    StrokeRoundRect(D2D1::RectF(enemy.x - 50.0f, enemy.y + 22.0f, enemy.x + 50.0f, enemy.y + 47.0f), 6.0f, D2D1::ColorF(enemyStageColor.r, enemyStageColor.g, enemyStageColor.b, 0.42f), 1.5f);

    // 적 포탑은 스테이지마다 다른 실루엣을 주어 행성별 방어선을 구분한다.
    const D2D1_COLOR_F towerGlow = D2D1::ColorF(enemyStageColor.r, enemyStageColor.g, enemyStageColor.b, 0.30f + enemyPulse * 0.16f);
    const D2D1_COLOR_F towerLine = D2D1::ColorF(enemyStageColor.r, enemyStageColor.g, enemyStageColor.b, 0.86f);
    const Vec2 turret = {enemy.x - 8.0f, enemy.y - 30.0f};
    FillEllipse(turret, 46.0f, 24.0f, towerGlow);
    switch (m_selectedStage)
    {
    case 0:
        FillRoundRect(D2D1::RectF(enemy.x - 38.0f, enemy.y - 45.0f, enemy.x + 28.0f, enemy.y - 2.0f), 8.0f, D2D1::ColorF(0x5B4B3A, 0.84f));
        DrawLine({enemy.x - 20.0f, enemy.y - 42.0f}, {enemy.x - 88.0f, enemy.y - 55.0f}, towerLine, 8.0f);
        DrawLine({enemy.x - 18.0f, enemy.y - 20.0f}, {enemy.x - 82.0f, enemy.y - 18.0f}, D2D1::ColorF(0xD9FFF8, 0.62f), 5.0f);
        StrokeEllipse({enemy.x - 6.0f, enemy.y - 20.0f}, 31.0f, 14.0f, D2D1::ColorF(0xD8A66A, 0.72f), 2.0f);
        break;
    case 1:
        FillEllipse({enemy.x - 4.0f, enemy.y - 35.0f}, 42.0f, 29.0f, D2D1::ColorF(0xFFB6E8, 0.34f));
        StrokeEllipse({enemy.x - 4.0f, enemy.y - 35.0f}, 42.0f, 29.0f, towerLine, 2.3f);
        DrawLine({enemy.x - 34.0f, enemy.y - 28.0f}, {enemy.x - 90.0f, enemy.y - 28.0f}, D2D1::ColorF(0xFFD27A, 0.74f), 7.0f);
        FillEllipse({enemy.x - 92.0f, enemy.y - 28.0f}, 9.0f, 9.0f, D2D1::ColorF(0xFFD27A, 0.72f));
        break;
    case 2:
        FillRoundRect(D2D1::RectF(enemy.x - 34.0f, enemy.y - 56.0f, enemy.x + 32.0f, enemy.y + 5.0f), 9.0f, D2D1::ColorF(0x204839, 0.84f));
        DrawLine({enemy.x - 10.0f, enemy.y - 40.0f}, {enemy.x - 82.0f, enemy.y - 60.0f}, D2D1::ColorF(0xB8FF89, 0.86f), 7.0f);
        FillEllipse({enemy.x + 18.0f, enemy.y - 54.0f}, 17.0f, 9.0f, D2D1::ColorF(0xB8FF89, 0.42f));
        DrawLine({enemy.x - 28.0f, enemy.y - 4.0f}, {enemy.x + 32.0f, enemy.y - 30.0f}, towerLine, 3.0f);
        break;
    case 3:
        FillRoundRect(D2D1::RectF(enemy.x - 42.0f, enemy.y - 50.0f, enemy.x + 32.0f, enemy.y + 8.0f), 6.0f, D2D1::ColorF(0x4B2725, 0.88f));
        DrawLine({enemy.x - 15.0f, enemy.y - 43.0f}, {enemy.x - 94.0f, enemy.y - 66.0f}, D2D1::ColorF(0xFF8B60, 0.86f), 9.0f);
        DrawLine({enemy.x - 13.0f, enemy.y - 19.0f}, {enemy.x - 88.0f, enemy.y - 20.0f}, D2D1::ColorF(0xFFDB7A, 0.66f), 6.0f);
        FillEllipse({enemy.x - 94.0f, enemy.y - 66.0f}, 8.0f, 8.0f, D2D1::ColorF(0xFFDB7A, 0.78f));
        break;
    case 4:
        FillEllipse({enemy.x - 2.0f, enemy.y - 34.0f}, 48.0f, 31.0f, D2D1::ColorF(0xD8A66A, 0.34f));
        StrokeEllipse({enemy.x - 2.0f, enemy.y - 34.0f}, 48.0f, 31.0f, towerLine, 2.3f);
        FillEllipse({enemy.x - 20.0f, enemy.y - 35.0f}, 16.0f, 9.0f, D2D1::ColorF(0xFF9BA8, 0.42f));
        DrawLine({enemy.x - 36.0f, enemy.y - 35.0f}, {enemy.x - 96.0f, enemy.y - 38.0f}, D2D1::ColorF(0xFFF0B5, 0.76f), 8.0f);
        break;
    case 5:
        FillEllipse({enemy.x - 3.0f, enemy.y - 31.0f}, 36.0f, 25.0f, D2D1::ColorF(0xE6D392, 0.28f));
        StrokeEllipse({enemy.x - 3.0f, enemy.y - 31.0f}, 56.0f, 17.0f, D2D1::ColorF(0xE6D392, 0.76f), 3.2f);
        DrawLine({enemy.x - 12.0f, enemy.y - 30.0f}, {enemy.x - 88.0f, enemy.y - 45.0f}, towerLine, 7.0f);
        FillEllipse({enemy.x + 18.0f, enemy.y - 18.0f}, 9.0f, 9.0f, D2D1::ColorF(0xFFF0B5, 0.72f));
        break;
    case 6:
        FillRoundRect(D2D1::RectF(enemy.x - 28.0f, enemy.y - 62.0f, enemy.x + 35.0f, enemy.y + 5.0f), 7.0f, D2D1::ColorF(0xD9FFF8, 0.22f));
        DrawLine({enemy.x + 16.0f, enemy.y - 56.0f}, {enemy.x - 74.0f, enemy.y - 36.0f}, D2D1::ColorF(0xD9FFF8, 0.88f), 8.0f);
        DrawLine({enemy.x + 28.0f, enemy.y - 7.0f}, {enemy.x - 62.0f, enemy.y - 28.0f}, D2D1::ColorF(0x75A7FF, 0.58f), 5.0f);
        StrokeEllipse({enemy.x + 2.0f, enemy.y - 30.0f}, 35.0f, 28.0f, towerLine, 2.2f);
        break;
    case 7:
        FillEllipse({enemy.x - 3.0f, enemy.y - 33.0f}, 40.0f, 27.0f, D2D1::ColorF(0x75A7FF, 0.25f));
        DrawLine({enemy.x - 22.0f, enemy.y - 42.0f}, {enemy.x - 95.0f, enemy.y - 60.0f}, D2D1::ColorF(0xBFD9FF, 0.80f), 7.0f);
        DrawLine({enemy.x - 18.0f, enemy.y - 18.0f}, {enemy.x - 91.0f, enemy.y - 14.0f}, D2D1::ColorF(0x65D8FF, 0.66f), 6.0f);
        StrokeEllipse({enemy.x - 31.0f, enemy.y - 30.0f}, 28.0f, 14.0f, towerLine, 2.0f);
        break;
    case 8:
        FillEllipse({enemy.x - 2.0f, enemy.y - 32.0f}, 42.0f, 32.0f, D2D1::ColorF(0xC8B7FF, 0.22f));
        StrokeEllipse({enemy.x - 2.0f, enemy.y - 32.0f}, 42.0f, 32.0f, towerLine, 2.4f);
        StrokeEllipse({enemy.x - 28.0f, enemy.y - 33.0f}, 42.0f, 14.0f, D2D1::ColorF(0xF7D6FF, 0.48f), 2.2f);
        DrawLine({enemy.x - 18.0f, enemy.y - 35.0f}, {enemy.x - 86.0f, enemy.y - 45.0f}, D2D1::ColorF(0xC8B7FF, 0.74f), 7.0f);
        break;
    default:
        FillEllipse({enemy.x - 4.0f, enemy.y - 35.0f}, 48.0f, 48.0f, D2D1::ColorF(0xFFB347, 0.24f));
        StrokeEllipse({enemy.x - 4.0f, enemy.y - 35.0f}, 48.0f, 48.0f, D2D1::ColorF(0xFFE66D, 0.88f), 2.7f);
        for (int i = 0; i < 8; ++i)
        {
            const float a = static_cast<float>(i) / 8.0f * (kPi * 2.0f) + m_stageTime * 0.4f;
            const Vec2 ray = {enemy.x - 4.0f + std::cos(a) * 42.0f, enemy.y - 35.0f + std::sin(a) * 42.0f};
            const Vec2 tip = {enemy.x - 4.0f + std::cos(a) * 66.0f, enemy.y - 35.0f + std::sin(a) * 66.0f};
            DrawLine(ray, tip, D2D1::ColorF(0xFFE66D, 0.42f), 3.0f);
        }
        DrawLine({enemy.x - 18.0f, enemy.y - 34.0f}, {enemy.x - 98.0f, enemy.y - 34.0f}, D2D1::ColorF(0xFFDB7A, 0.88f), 8.0f);
        break;
    }

    auto drawBaseDamage = [&](Vec2 base, float missing, D2D1_COLOR_F accent, bool enemySide) {
        // 체력바만 보면 전황이 늦게 읽히기 때문에, 기지 자체에 파손 단계를 올려준다.
        // missing 값은 0.0(멀쩡함)에서 1.0(파괴 직전)까지의 비율이다.
        if (missing <= 0.16f)
        {
            return;
        }

        const float side = enemySide ? -1.0f : 1.0f;
        const int crackCount = missing > 0.72f ? 5 : (missing > 0.42f ? 3 : 2);
        const D2D1_COLOR_F crackColor = D2D1::ColorF(0x02070A, 0.44f + missing * 0.28f);
        for (int i = 0; i < crackCount; ++i)
        {
            const float row = static_cast<float>(i);
            const float ox = -30.0f + std::fmod(row * 31.0f, 62.0f);
            const float oy = -55.0f + row * 23.0f;
            const Vec2 a = {base.x + ox, base.y + oy};
            const Vec2 b = {a.x + side * (16.0f + missing * 28.0f), a.y + 10.0f + std::sin(row * 1.7f) * 8.0f};
            const Vec2 c = {b.x - side * (9.0f + row * 2.0f), b.y + 12.0f};
            DrawLine(a, b, crackColor, 2.2f);
            DrawLine(b, c, crackColor, 1.45f);
        }

        if (missing > 0.44f)
        {
            for (int i = 0; i < 4; ++i)
            {
                const float seed = static_cast<float>(i);
                const float rise = std::fmod(m_stageTime * (18.0f + seed * 5.0f), 54.0f);
                const Vec2 smoke = {base.x - side * (38.0f + seed * 12.0f), base.y - 82.0f - rise};
                const float puff = 10.0f + seed * 2.0f + missing * 8.0f;
                FillEllipse(smoke, puff, puff * 0.64f, D2D1::ColorF(0xD9E5F2, (0.07f + missing * 0.05f) * (1.0f - rise / 64.0f)));
            }
        }

        if (missing > 0.70f)
        {
            const float pulse = 0.45f + 0.55f * std::abs(std::sin(m_stageTime * 9.0f));
            FillEllipse({base.x, base.y - 14.0f}, 108.0f, 112.0f, D2D1::ColorF(0xFF4A6E, 0.045f + pulse * 0.045f));
            StrokeEllipse({base.x, base.y - 14.0f}, 88.0f + pulse * 10.0f, 92.0f + pulse * 10.0f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.42f), 2.4f);
            DrawPixelTextCentered(L"CRITICAL",
                                  D2D1::RectF(base.x - 76.0f, base.y - 142.0f, base.x + 76.0f, base.y - 118.0f),
                                  1.65f,
                                  D2D1::ColorF(0xFFCAD1),
                                  0.84f + pulse * 0.16f);
        }
    };

    drawBaseDamage(player, MissingHpRatio(m_playerBaseHp, m_playerBaseMaxHp), D2D1::ColorF(0x65B8FF), false);
    drawBaseDamage(enemy, MissingHpRatio(m_enemyBaseHp, m_enemyBaseMaxHp), towerLine, true);

    DrawBaseHp(player, m_playerBaseHp, m_playerBaseMaxHp, D2D1::ColorF(0x65B8FF));
    DrawBaseHp(enemy, m_enemyBaseHp, m_enemyBaseMaxHp, towerLine);
}

void PawlineGameImpl::DrawBaseHp(Vec2 base, float hp, float maxHp, D2D1_COLOR_F color)
{
    const float pct = Clamp01(hp / maxHp);
    D2D1_RECT_F back = D2D1::RectF(base.x - 72.0f, base.y - 106.0f, base.x + 72.0f, base.y - 92.0f);
    FillRoundRect(back, 4.0f, D2D1::ColorF(0x071017));
    FillRoundRect(D2D1::RectF(back.left, back.top, back.left + (back.right - back.left) * pct, back.bottom), 4.0f, color);
    StrokeRoundRect(back, 4.0f, D2D1::ColorF(0xD9E5F2, 0.45f), 1.0f);
    DrawPixelTextCentered(ToWideInt(static_cast<int>(std::ceil(std::max(0.0f, hp)))) + L"/" + ToWideInt(static_cast<int>(maxHp)),
                          D2D1::RectF(back.left, back.top - 22.0f, back.right, back.top - 2.0f),
                          1.8f,
                          D2D1::ColorF(0xEAF7FF),
                          1.0f);
}

Vec2 PawlineGameImpl::StageLightDirection() const
{
    // 행성마다 태양빛이 들어오는 각도를 조금씩 바꿔 배경과 유닛 그림자가 같은 공간에 있는 것처럼 보이게 한다.
    const float drift = std::sin(m_stageTime * 0.10f + static_cast<float>(m_selectedStage) * 0.7f) * 0.08f;
    const float stageBias = -0.66f + static_cast<float>(m_selectedStage) * 0.105f;
    return Normalize({stageBias + drift, -0.86f});
}

float PawlineGameImpl::UnitShadowLift(const Unit& unit) const
{
    const float attackLift = AttackWindup(unit) * 0.18f + AttackStrike(unit) * 0.42f;
    const float knockLift = Clamp01(unit.knockbackTimer / 0.34f) * 0.40f;
    const float stunSink = unit.stunTimer > 0.0f ? 0.06f : 0.0f;
    return unit.radius * (0.20f + attackLift + knockLift + stunSink);
}

void PawlineGameImpl::DrawUnitLighting()
{
    const Vec2 lightDir = StageLightDirection();
    const Vec2 shadowDir = lightDir * -1.0f;

    for (const Unit& unit : m_units)
    {
        if (!unit.alive)
        {
            continue;
        }

        const Vec2 pos = UnitRenderPos(unit);
        const float windup = AttackWindup(unit);
        const float strike = AttackStrike(unit);
        const float attack = Clamp01(windup * 0.45f + strike);
        const float depth = Clamp01((pos.y - kBattleTop) / (kBattleBottom - kBattleTop));
        const float lift = UnitShadowLift(unit);
        const D2D1_COLOR_F accent = unit.team == Team::Player
                                        ? GetPlayerStats(static_cast<PlayerUnit>(unit.kind)).accent
                                        : GetEnemyStats(static_cast<EnemyUnit>(unit.kind), ThreatLevel()).accent;

        const Vec2 foot = {pos.x + unit.attackDir * attack * 6.0f, pos.y + unit.radius + 12.0f};
        FillEllipse(foot, unit.radius * (1.10f + depth * 0.20f), unit.radius * 0.24f, D2D1::ColorF(0x000000, 0.22f + depth * 0.08f));
        for (int i = 0; i < 3; ++i)
        {
            const float t = static_cast<float>(i);
            const Vec2 cast = foot + shadowDir * (lift * (1.15f + t * 0.72f));
            const float fade = 1.0f - t * 0.27f;
            FillEllipse(cast, unit.radius * (1.46f + lift * 0.024f + t * 0.16f + attack * 0.38f),
                        unit.radius * (0.32f + t * 0.035f + attack * 0.08f),
                        D2D1::ColorF(0x000000, (0.30f + depth * 0.10f) * fade));
        }
        FillEllipse({pos.x - lightDir.x * unit.radius * 0.42f, pos.y - unit.radius * 0.56f}, unit.radius * 0.60f, unit.radius * 0.25f, D2D1::ColorF(0xFFFFFF, 0.040f));
        FillEllipse({pos.x + unit.attackDir * unit.radius * 0.45f, pos.y + unit.radius * 0.16f}, unit.radius * 0.52f, unit.radius * 0.18f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.032f));

        if (attack > 0.0f)
        {
            const Vec2 front = {pos.x + unit.attackDir * (unit.radius + 28.0f + strike * 28.0f), pos.y - 6.0f};
            FillEllipse(pos, unit.radius * (2.55f + strike * 1.25f), unit.radius * (1.78f + strike * 0.70f), D2D1::ColorF(accent.r, accent.g, accent.b, 0.052f + strike * 0.092f));
            FillEllipse(front, unit.radius * (1.95f + strike * 1.55f), unit.radius * (1.10f + strike * 0.80f), D2D1::ColorF(accent.r, accent.g, accent.b, 0.066f + strike * 0.130f));
            FillEllipse({pos.x - lightDir.x * unit.radius * 0.38f, pos.y - unit.radius * 0.42f}, unit.radius * 0.72f, unit.radius * 0.34f, D2D1::ColorF(0xFFFFFF, 0.055f + strike * 0.060f));
            FillEllipse({pos.x + unit.attackDir * unit.radius * 0.10f, pos.y - unit.radius * 0.70f},
                        unit.radius * (0.68f + strike * 0.25f),
                        unit.radius * (0.11f + strike * 0.08f),
                        D2D1::ColorF(0xFFFFFF, 0.060f + strike * 0.075f));
        }
    }
}

void PawlineGameImpl::DrawUnits()
{
    std::vector<int> order;
    order.reserve(m_units.size());
    for (int i = 0; i < static_cast<int>(m_units.size()); ++i)
    {
        order.push_back(i);
    }
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_units[a].pos.y < m_units[b].pos.y;
    });

    for (int index : order)
    {
        const Unit& unit = m_units[index];
        if (unit.team == Team::Player)
        {
            DrawPlayerUnit(unit);
        }
        else
        {
            DrawEnemyUnit(unit);
        }
        DrawUnitStunEffect(unit);
        DrawUnitHp(unit);
    }
}

float PawlineGameImpl::AttackProgress(const Unit& unit) const
{
    if (unit.attackAnim <= 0.0f || unit.attackAnimMax <= 0.0f)
    {
        return 1.0f;
    }

    return 1.0f - Clamp01(unit.attackAnim / unit.attackAnimMax);
}

float PawlineGameImpl::AttackIntensity(const Unit& unit) const
{
    if (unit.attackAnim <= 0.0f || unit.attackAnimMax <= 0.0f)
    {
        return 0.0f;
    }
    return std::max(AttackStrike(unit), std::max(AttackWindup(unit) * 0.55f, AttackRecoil(unit) * 0.42f));
}

float PawlineGameImpl::AttackWindup(const Unit& unit) const
{
    const float p = AttackProgress(unit);
    if (p <= 0.0f || p >= 0.40f)
    {
        return 0.0f;
    }
    return std::sin((p / 0.40f) * kPi);
}

float PawlineGameImpl::AttackStrike(const Unit& unit) const
{
    const float p = AttackProgress(unit);
    float center = unit.ranged ? 0.46f : 0.56f;
    float width = unit.ranged ? 0.18f : 0.16f;
    if (unit.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(unit.kind))
        {
        case PlayerUnit::Dash:
        case PlayerUnit::Comet:
            center = 0.42f;
            width = 0.13f;
            break;
        case PlayerUnit::Box:
        case PlayerUnit::Titan:
        case PlayerUnit::Solar:
        case PlayerUnit::Drill:
            center = 0.63f;
            width = 0.19f;
            break;
        case PlayerUnit::Bell:
        case PlayerUnit::Mint:
            center = 0.50f;
            width = 0.23f;
            break;
        case PlayerUnit::Orbit:
        case PlayerUnit::Prism:
        case PlayerUnit::Nebula:
            center = 0.50f;
            width = 0.20f;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (static_cast<EnemyUnit>(unit.kind))
        {
        case EnemyUnit::Skitter:
        case EnemyUnit::Flare:
        case EnemyUnit::Comet:
        case EnemyUnit::Frost:
            center = 0.41f;
            width = 0.12f;
            break;
        case EnemyUnit::Brute:
        case EnemyUnit::Storm:
        case EnemyUnit::Quake:
        case EnemyUnit::Boss:
            center = 0.64f;
            width = 0.20f;
            break;
        case EnemyUnit::Sulfur:
        case EnemyUnit::Ring:
        case EnemyUnit::Tide:
        case EnemyUnit::Void:
        case EnemyUnit::Spore:
            center = 0.50f;
            width = 0.19f;
            break;
        default:
            break;
        }
    }
    return Clamp01(1.0f - std::abs(p - center) / width);
}

float PawlineGameImpl::AttackRecoil(const Unit& unit) const
{
    const float p = AttackProgress(unit);
    if (p <= 0.58f || p >= 1.0f)
    {
        return 0.0f;
    }
    return std::sin(((p - 0.58f) / 0.42f) * kPi);
}

float PawlineGameImpl::AttackLungeDistance(const Unit& unit) const
{
    if (unit.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(unit.kind))
        {
        case PlayerUnit::Dash:
        case PlayerUnit::Comet:
            return 24.0f;
        case PlayerUnit::Drill:
            return 22.0f;
        case PlayerUnit::Titan:
        case PlayerUnit::Solar:
            return 15.0f;
        case PlayerUnit::Spark:
        case PlayerUnit::Prism:
        case PlayerUnit::Nebula:
            return -7.0f;
        default:
            return unit.ranged ? -5.0f : 12.0f;
        }
    }

    switch (static_cast<EnemyUnit>(unit.kind))
    {
    case EnemyUnit::Comet:
    case EnemyUnit::Flare:
        return 21.0f;
    case EnemyUnit::Quake:
    case EnemyUnit::Boss:
        return 13.0f;
    default:
        return unit.ranged ? -5.0f : 10.0f;
    }
}

Vec2 PawlineGameImpl::UnitRenderPos(const Unit& unit) const
{
    Vec2 pos = unit.pos;
    if (unit.animState == UnitAnimState::Move)
    {
        const float step = std::sin(unit.walkCycle);
        pos.y += step * 1.8f;
        pos.x -= unit.attackDir * std::abs(step) * 1.2f;
    }
    else if (unit.animState == UnitAnimState::Idle)
    {
        pos.y += std::sin(unit.stateTime * 2.6f + unit.shakePhase) * 0.9f;
    }
    else if (unit.animState == UnitAnimState::Hit)
    {
        const float hit = Clamp01(unit.hitFlash / 0.12f);
        pos.x -= unit.attackDir * 4.5f * hit;
        pos.y += 1.8f * hit;
    }

    if (unit.shakeTimer > 0.0f)
    {
        const float amp = 7.0f * Clamp01(unit.shakeTimer / 0.18f);
        pos.x += std::sin(m_stageTime * 96.0f + unit.shakePhase) * amp;
        pos.y += std::cos(m_stageTime * 84.0f + unit.shakePhase) * amp * 0.45f;
    }

    const float attack = AttackIntensity(unit);
    if (attack > 0.0f)
    {
        const float windup = AttackWindup(unit);
        const float strike = AttackStrike(unit);
        const float recoil = AttackRecoil(unit);
        pos.x += unit.attackDir * AttackLungeDistance(unit) * (strike - windup * 0.34f - recoil * 0.14f);
        pos.y -= strike * 4.8f;
        pos.y += recoil * 1.8f;
        pos.y -= std::sin(AttackProgress(unit) * kPi * 2.0f) * 1.4f;
    }
    return pos;
}

void PawlineGameImpl::DrawUnitActionLines(const Unit& unit, Vec2 pos, D2D1_COLOR_F accent)
{
    if (unit.attackAnim <= 0.0f || unit.attackAnimMax <= 0.0f)
    {
        return;
    }

    const float windup = AttackWindup(unit);
    const float strike = AttackStrike(unit);
    const float recoil = AttackRecoil(unit);
    const float dir = unit.attackDir;

    if (windup > 0.0f)
    {
        const float alpha = windup * 0.28f;
        const Vec2 charge = {pos.x - dir * (unit.radius * 0.35f + 12.0f), pos.y - 2.0f};
        FillEllipse(charge, unit.radius * 1.28f, unit.radius * 0.68f, D2D1::ColorF(accent.r, accent.g, accent.b, alpha * 0.44f));
        StrokeEllipse(charge, unit.radius * (1.10f + windup * 0.22f), unit.radius * (0.55f + windup * 0.08f),
                      D2D1::ColorF(accent.r, accent.g, accent.b, alpha), 1.8f);
    }

    if (strike > 0.0f)
    {
        const Vec2 front = {pos.x + dir * (unit.radius + 34.0f + strike * 28.0f), pos.y - 2.0f};
        const ImageVfxKind kind = unit.team == Team::Player ? ImageVfxKind::Slash : ImageVfxKind::EnemySlash;
        const int frame = std::clamp(static_cast<int>(AttackProgress(unit) * 8.0f), 0, 7);
        FillEllipse(front, 40.0f + strike * 32.0f, 19.0f + strike * 15.0f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.22f * strike));
        DrawImageVfxFrame(kind, frame, front, 96.0f + strike * 72.0f, 0.72f * strike);
        DrawVfxAtlasTile(unit.team == Team::Player ? 0 : 1, 1, front, 104.0f + strike * 58.0f, 0.24f * strike);
        StrokeEllipse(front,
                      30.0f + strike * 34.0f,
                      12.0f + strike * 15.0f,
                      D2D1::ColorF(0xFFFFFF, 0.18f * strike),
                      1.5f + strike * 1.4f);
    }

    if (recoil > 0.0f)
    {
        FillEllipse({pos.x - dir * (unit.radius + 12.0f), pos.y + unit.radius + 10.0f},
                    unit.radius * (0.82f + recoil * 0.40f), 5.0f + recoil * 3.0f,
                    D2D1::ColorF(0xFFFFFF, recoil * 0.055f));
    }
}

void PawlineGameImpl::DrawPlayerWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil)
{
    const PlayerUnit type = static_cast<PlayerUnit>(unit.kind);
    const bool ranged = unit.ranged;
    const float dir = unit.attackDir;
    const float reach = ranged ? (strike * 7.0f - windup * 5.0f + recoil * 4.0f) : (strike * 34.0f - windup * 18.0f + recoil * 7.0f);
    const int weaponIndex = std::clamp(static_cast<int>(type), 0, kRosterCount - 1);

    // 스프라이트 팔 위치에 붙이는 수동 무기 포즈.
    // 각 유닛의 무기 길이와 전투 방식이 달라서 손 위치, 크기, 휘두름 각도를 따로 보정한다.
    float handLift = ranged ? (unit.radius * 0.06f + 2.0f) : (unit.radius * 0.15f + 4.0f);
    float handForward = unit.radius * (ranged ? 0.42f : 0.50f);
    float weaponForward = ranged ? 28.0f : 32.0f;
    float weaponLift = ranged ? 0.0f : 3.0f;
    float frontForward = ranged ? 42.0f : 38.0f;
    float frontLift = ranged ? handLift - 4.0f : 2.0f;
    float weaponWidth = (ranged ? 68.0f : 78.0f) + unit.radius * (ranged ? 0.20f : 0.26f);
    float weaponHeight = (ranged ? 46.0f : 50.0f) + unit.radius * (ranged ? 0.08f : 0.10f);
    float angleBase = ranged ? -2.0f : -18.0f;
    float strikeAngle = ranged ? 3.0f : 48.0f;
    float windupAngle = ranged ? 5.0f : 38.0f;
    float recoilAngle = ranged ? 2.0f : 17.0f;
    float reachWeight = ranged ? 0.45f : 0.68f;

    switch (type)
    {
    case PlayerUnit::Box:
        handLift = unit.radius * 0.30f + 8.0f;
        handForward = unit.radius * 0.54f;
        weaponForward = 38.0f;
        weaponLift = 9.0f;
        weaponWidth = 64.0f;
        weaponHeight = 56.0f;
        angleBase = -14.0f;
        strikeAngle = 16.0f;
        windupAngle = 8.0f;
        break;
    case PlayerUnit::Spark:
    case PlayerUnit::Bell:
    case PlayerUnit::Mint:
        handLift = unit.radius * 0.04f + 1.0f;
        handForward = unit.radius * 0.44f;
        weaponForward = 30.0f;
        weaponLift = 1.0f;
        weaponWidth = 66.0f;
        weaponHeight = 58.0f;
        angleBase = -10.0f;
        strikeAngle = 9.0f;
        windupAngle = 10.0f;
        break;
    case PlayerUnit::Titan:
        handLift = unit.radius * 0.16f + 6.0f;
        handForward = unit.radius * 0.52f;
        weaponForward = 39.0f;
        weaponLift = 3.0f;
        weaponWidth += 12.0f;
        weaponHeight += 8.0f;
        strikeAngle = 64.0f;
        windupAngle = 52.0f;
        recoilAngle = 24.0f;
        break;
    case PlayerUnit::Frost:
    case PlayerUnit::Comet:
        handLift = unit.radius * 0.10f + 4.0f;
        handForward = unit.radius * 0.50f;
        weaponForward = 42.0f;
        weaponLift = 3.0f;
        weaponWidth += 14.0f;
        weaponHeight -= 4.0f;
        angleBase = -8.0f;
        strikeAngle = 28.0f;
        windupAngle = 18.0f;
        reachWeight = 0.88f;
        break;
    case PlayerUnit::Orbit:
    case PlayerUnit::Prism:
    case PlayerUnit::Nebula:
        handLift = unit.radius * 0.04f + 1.0f;
        handForward = unit.radius * 0.46f;
        weaponForward = 32.0f;
        weaponLift = 2.0f;
        weaponWidth += type == PlayerUnit::Nebula ? 14.0f : 6.0f;
        weaponHeight += type == PlayerUnit::Nebula ? 8.0f : 2.0f;
        angleBase = -1.0f;
        strikeAngle = 5.0f;
        recoilAngle = 9.0f;
        break;
    case PlayerUnit::Solar:
        handLift = unit.radius * 0.12f + 5.0f;
        handForward = unit.radius * 0.52f;
        weaponForward = 40.0f;
        weaponLift = 3.0f;
        weaponWidth += 15.0f;
        weaponHeight += 8.0f;
        strikeAngle = 58.0f;
        windupAngle = 46.0f;
        recoilAngle = 22.0f;
        break;
    case PlayerUnit::Drill:
        handLift = unit.radius * 0.10f + 4.0f;
        handForward = unit.radius * 0.50f;
        weaponForward = 37.0f;
        weaponLift = 4.0f;
        weaponWidth += 10.0f;
        weaponHeight += 5.0f;
        angleBase = -6.0f;
        strikeAngle = 22.0f;
        windupAngle = 16.0f;
        break;
    default:
        break;
    }

    frontLift = ranged ? handLift - 4.0f : frontLift;
    const Vec2 hand = {pos.x + dir * handForward, pos.y + handLift};
    const Vec2 front = {pos.x + dir * (unit.radius + frontForward + reach), pos.y + frontLift};
    const float gripCenterOffset = weaponForward * (ranged ? 0.36f : 0.43f);
    const Vec2 weaponCenter = {hand.x + dir * (gripCenterOffset + reach * reachWeight * 0.64f),
                               hand.y + weaponLift * 0.54f + recoil * 2.0f};
    const float weaponAngle = (dir >= 0.0f ? angleBase : -angleBase) + dir * (strike * strikeAngle - windup * windupAngle + recoil * recoilAngle);
    const float action = Clamp01(windup + strike + recoil);
    const float glowAlpha = 0.10f + action * 0.18f;
    ImageVfxKind vfx = ImageVfxKind::HitFlash;
    float vfxSize = (ranged ? 42.0f : 52.0f) + strike * (ranged ? 14.0f : 24.0f);

    // 실제 무기는 PNG만 사용하고, 아래 switch는 유닛별 공격 빛과 충격 모양만 고른다.
    switch (type)
    {
    case PlayerUnit::Paw:
        vfx = ImageVfxKind::Slash;
        break;
    case PlayerUnit::Box:
        vfx = ImageVfxKind::Earth;
        vfxSize += 10.0f;
        break;
    case PlayerUnit::Spark:
        vfx = ImageVfxKind::ThunderSplash;
        vfxSize += 12.0f;
        break;
    case PlayerUnit::Dash:
        vfx = ImageVfxKind::WindHit;
        break;
    case PlayerUnit::Bell:
        vfx = ImageVfxKind::Holy;
        vfxSize += 8.0f;
        break;
    case PlayerUnit::Titan:
        vfx = ImageVfxKind::Explosion;
        vfxSize += 20.0f;
        break;
    case PlayerUnit::Frost:
        vfx = ImageVfxKind::Ice;
        vfxSize += 10.0f;
        break;
    case PlayerUnit::Comet:
        vfx = ImageVfxKind::Thrust;
        vfxSize += 14.0f;
        break;
    case PlayerUnit::Orbit:
        vfx = ImageVfxKind::EnergyImpact;
        vfxSize += 14.0f;
        break;
    case PlayerUnit::Solar:
        vfx = ImageVfxKind::FireBreath;
        vfxSize += 18.0f;
        break;
    case PlayerUnit::Mint:
        vfx = ImageVfxKind::HealSoft;
        break;
    case PlayerUnit::Drill:
        vfx = ImageVfxKind::Smear;
        vfxSize += 8.0f;
        break;
    case PlayerUnit::Prism:
        vfx = ImageVfxKind::MagicMirror;
        vfxSize += 10.0f;
        break;
    case PlayerUnit::Nebula:
        vfx = ImageVfxKind::Dark;
        vfxSize += 16.0f;
        break;
    }

    FillEllipse(weaponCenter,
                (ranged ? 36.0f : 46.0f) + action * (ranged ? 16.0f : 28.0f),
                (ranged ? 14.0f : 19.0f) + action * (ranged ? 7.0f : 10.0f),
                D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, glowAlpha));
    if (strike > 0.0f)
    {
        const int frame = std::clamp(static_cast<int>(AttackProgress(unit) * 8.0f), 0, 7);
        DrawImageVfxFrame(vfx, frame, front, vfxSize, (ranged ? 0.42f : 0.60f) * strike);
        StrokeEllipse(front,
                      (ranged ? 18.0f : 28.0f) + strike * (ranged ? 18.0f : 34.0f),
                      (ranged ? 8.0f : 12.0f) + strike * (ranged ? 8.0f : 14.0f),
                      D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.22f + strike * 0.26f),
                      2.0f + strike * 1.2f);
    }

    DrawWeaponBitmap(m_playerWeaponBitmaps[static_cast<size_t>(weaponIndex)].Get(),
                     weaponCenter,
                     weaponWidth + strike * (ranged ? 4.0f : 13.0f),
                     weaponHeight,
                     weaponAngle,
                     0.88f,
                     dir < 0.0f);

    if (recoil > 0.0f)
    {
        FillEllipse({pos.x - dir * (unit.radius + 12.0f), pos.y + unit.radius + 10.0f},
                    unit.radius * (0.82f + recoil * 0.40f), 5.0f + recoil * 3.0f,
                    D2D1::ColorF(0xFFFFFF, recoil * 0.055f));
    }
}

void PawlineGameImpl::DrawEnemyWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil)
{
    const EnemyUnit type = static_cast<EnemyUnit>(unit.kind);
    const bool ranged = unit.ranged;
    const float dir = unit.attackDir;
    const float reach = ranged ? (strike * 7.0f - windup * 4.0f + recoil * 4.0f) : (strike * 32.0f - windup * 16.0f + recoil * 7.0f);
    const int weaponIndex = std::clamp(static_cast<int>(type), 0, kEnemyCount - 1);

    // 적도 타워 쪽이 아니라 플레이어 쪽으로 무기가 향하도록 attackDir(-1)을 기준으로 좌우 반전한다.
    float handLift = ranged ? (unit.radius * 0.06f + 2.0f) : (unit.radius * 0.15f + 4.0f);
    float handForward = unit.radius * (ranged ? 0.42f : 0.50f);
    float weaponForward = ranged ? 28.0f : 32.0f;
    float weaponLift = ranged ? 0.0f : 3.0f;
    float frontForward = ranged ? 42.0f : 38.0f;
    float frontLift = ranged ? handLift - 4.0f : 2.0f;
    float weaponWidth = (ranged ? 68.0f : 78.0f) + unit.radius * (ranged ? 0.20f : 0.26f);
    float weaponHeight = (ranged ? 46.0f : 50.0f) + unit.radius * (ranged ? 0.08f : 0.10f);
    float angleBase = ranged ? -2.0f : -16.0f;
    float strikeAngle = ranged ? 3.0f : 46.0f;
    float windupAngle = ranged ? 4.0f : 36.0f;
    float recoilAngle = ranged ? 2.0f : 16.0f;
    float reachWeight = ranged ? 0.45f : 0.66f;

    switch (type)
    {
    case EnemyUnit::Brute:
    case EnemyUnit::Rust:
    case EnemyUnit::Quake:
    case EnemyUnit::Boss:
        handLift = unit.radius * 0.16f + 6.0f;
        handForward = unit.radius * 0.52f;
        weaponForward = type == EnemyUnit::Boss ? 43.0f : 38.0f;
        weaponLift = 3.0f;
        weaponWidth += type == EnemyUnit::Boss ? 20.0f : 10.0f;
        weaponHeight += type == EnemyUnit::Boss ? 12.0f : 7.0f;
        strikeAngle = type == EnemyUnit::Boss ? 66.0f : 58.0f;
        windupAngle = type == EnemyUnit::Boss ? 54.0f : 46.0f;
        recoilAngle = 23.0f;
        break;
    case EnemyUnit::Skitter:
    case EnemyUnit::Frost:
        handLift = unit.radius * 0.10f + 4.0f;
        handForward = unit.radius * 0.50f;
        weaponForward = 37.0f;
        weaponLift = 3.0f;
        weaponWidth += 4.0f;
        weaponHeight -= 2.0f;
        strikeAngle = 34.0f;
        windupAngle = 20.0f;
        reachWeight = 0.82f;
        break;
    case EnemyUnit::Sulfur:
    case EnemyUnit::Ring:
    case EnemyUnit::Tide:
    case EnemyUnit::Mirror:
        handLift = unit.radius * 0.04f + 1.0f;
        handForward = unit.radius * 0.46f;
        weaponForward = 32.0f;
        weaponLift = 2.0f;
        weaponWidth += 6.0f;
        weaponHeight += 2.0f;
        angleBase = -1.0f;
        strikeAngle = 5.0f;
        recoilAngle = 9.0f;
        break;
    case EnemyUnit::Flare:
    case EnemyUnit::Comet:
        handLift = unit.radius * 0.10f + 4.0f;
        handForward = unit.radius * 0.50f;
        weaponForward = 42.0f;
        weaponLift = 3.0f;
        weaponWidth += 14.0f;
        weaponHeight -= 4.0f;
        angleBase = -8.0f;
        strikeAngle = 28.0f;
        windupAngle = 18.0f;
        reachWeight = 0.86f;
        break;
    case EnemyUnit::Spore:
        handLift = unit.radius * 0.04f + 1.0f;
        handForward = unit.radius * 0.44f;
        weaponForward = 30.0f;
        weaponLift = 1.0f;
        weaponWidth = 66.0f;
        weaponHeight = 58.0f;
        angleBase = -10.0f;
        strikeAngle = 9.0f;
        windupAngle = 10.0f;
        break;
    default:
        break;
    }

    frontLift = ranged ? handLift - 4.0f : frontLift;
    const Vec2 hand = {pos.x + dir * handForward, pos.y + handLift};
    const Vec2 front = {pos.x + dir * (unit.radius + frontForward + reach), pos.y + frontLift};
    const float gripCenterOffset = weaponForward * (ranged ? 0.36f : 0.43f);
    const Vec2 weaponCenter = {hand.x + dir * (gripCenterOffset + reach * reachWeight * 0.64f),
                               hand.y + weaponLift * 0.54f + recoil * 2.0f};
    const float weaponAngle = (dir >= 0.0f ? angleBase : -angleBase) + dir * (strike * strikeAngle - windup * windupAngle + recoil * recoilAngle);
    const float action = Clamp01(windup + strike + recoil);
    ImageVfxKind vfx = ImageVfxKind::EnemySlash;
    float vfxSize = (ranged ? 42.0f : 50.0f) + strike * (ranged ? 14.0f : 24.0f);

    // 적도 PNG 무기를 주역으로 쓰고, 타입별 차이는 색과 VFX로만 더한다.
    switch (type)
    {
    case EnemyUnit::Dust:
        vfx = ImageVfxKind::SmokeDust;
        break;
    case EnemyUnit::Brute:
        vfx = ImageVfxKind::Earth;
        vfxSize += 10.0f;
        break;
    case EnemyUnit::Skitter:
        vfx = ImageVfxKind::Smear;
        break;
    case EnemyUnit::Sulfur:
        vfx = ImageVfxKind::Acid;
        break;
    case EnemyUnit::Moss:
        vfx = ImageVfxKind::Wood;
        break;
    case EnemyUnit::Rust:
        vfx = ImageVfxKind::EnemySlash;
        break;
    case EnemyUnit::Storm:
        vfx = ImageVfxKind::EnergyImpact;
        vfxSize += 10.0f;
        break;
    case EnemyUnit::Ring:
        vfx = ImageVfxKind::Crystal;
        break;
    case EnemyUnit::Frost:
        vfx = ImageVfxKind::Ice;
        vfxSize += 8.0f;
        break;
    case EnemyUnit::Tide:
        vfx = ImageVfxKind::WaterBallImpact;
        break;
    case EnemyUnit::Void:
        vfx = ImageVfxKind::Dark;
        vfxSize += 12.0f;
        break;
    case EnemyUnit::Flare:
        vfx = ImageVfxKind::FireBreath;
        vfxSize += 10.0f;
        break;
    case EnemyUnit::Spore:
        vfx = ImageVfxKind::Acid;
        break;
    case EnemyUnit::Quake:
        vfx = ImageVfxKind::Earth;
        vfxSize += 16.0f;
        break;
    case EnemyUnit::Mirror:
        vfx = ImageVfxKind::MagicMirror;
        vfxSize += 8.0f;
        break;
    case EnemyUnit::Comet:
        vfx = ImageVfxKind::Thrust;
        vfxSize += 12.0f;
        break;
    case EnemyUnit::Boss:
        vfx = ImageVfxKind::Explosion;
        vfxSize += 24.0f;
        break;
    }

    FillEllipse(weaponCenter,
                (ranged ? 34.0f : 44.0f) + action * (ranged ? 15.0f : 26.0f),
                (ranged ? 13.0f : 18.0f) + action * (ranged ? 7.0f : 10.0f),
                D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.10f + action * 0.18f));
    if (strike > 0.0f)
    {
        const int frame = std::clamp(static_cast<int>(AttackProgress(unit) * 8.0f), 0, 7);
        DrawImageVfxFrame(vfx, frame, front, vfxSize, (ranged ? 0.40f : 0.56f) * strike);
        StrokeEllipse(front,
                      (ranged ? 18.0f : 26.0f) + strike * (ranged ? 18.0f : 32.0f),
                      (ranged ? 8.0f : 12.0f) + strike * (ranged ? 8.0f : 13.0f),
                      D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.22f + strike * 0.25f),
                      2.0f + strike * 1.2f);
    }

    DrawWeaponBitmap(m_enemyWeaponBitmaps[static_cast<size_t>(weaponIndex)].Get(),
                     weaponCenter,
                     weaponWidth + strike * (ranged ? 4.0f : 13.0f),
                     weaponHeight,
                     weaponAngle,
                     0.88f,
                     dir < 0.0f);

    if (recoil > 0.0f)
    {
        FillEllipse({pos.x - dir * (unit.radius + 12.0f), pos.y + unit.radius + 10.0f},
                    unit.radius * (0.80f + recoil * 0.38f), 5.0f + recoil * 3.0f,
                    D2D1::ColorF(0xFFFFFF, recoil * 0.050f));
    }
}

void PawlineGameImpl::DrawUnitIdentityMark(const Unit& unit, Vec2 pos, D2D1_COLOR_F accent)
{
    const float attack = AttackIntensity(unit);
    const float pulse = 0.65f + std::sin(m_uiTime * 4.2f + static_cast<float>(unit.id) * 0.37f) * 0.18f;
    const float dir = unit.team == Team::Player ? 1.0f : -1.0f;
    const D2D1_COLOR_F soft = D2D1::ColorF(accent.r, accent.g, accent.b, 0.18f + attack * 0.16f);
    const D2D1_COLOR_F bright = D2D1::ColorF(accent.r, accent.g, accent.b, 0.62f + attack * 0.24f);

    if (unit.team == Team::Player)
    {
        switch (static_cast<PlayerUnit>(unit.kind))
        {
        case PlayerUnit::Paw:
            DrawLine({pos.x + dir * 18.0f, pos.y - 24.0f}, {pos.x + dir * (42.0f + attack * 12.0f), pos.y - 18.0f}, bright, 2.4f);
            FillEllipse({pos.x + dir * 29.0f, pos.y - 16.0f}, 17.0f + attack * 9.0f, 8.0f + attack * 4.0f, soft);
            break;
        case PlayerUnit::Box:
            FillRoundRect(D2D1::RectF(pos.x + dir * 18.0f - 5.0f, pos.y - 28.0f, pos.x + dir * 48.0f + 5.0f, pos.y + 24.0f), 7.0f, D2D1::ColorF(0xFFF0B5, 0.13f + attack * 0.16f));
            StrokeRoundRect(D2D1::RectF(pos.x + dir * 20.0f - 5.0f, pos.y - 25.0f, pos.x + dir * 45.0f + 5.0f, pos.y + 21.0f), 6.0f, bright, 2.3f);
            break;
        case PlayerUnit::Spark:
            DrawLine({pos.x - 10.0f, pos.y - 42.0f}, {pos.x + 4.0f, pos.y - 56.0f}, bright, 2.4f);
            DrawLine({pos.x + 4.0f, pos.y - 56.0f}, {pos.x - 2.0f, pos.y - 39.0f}, D2D1::ColorF(0xF6FF83, 0.78f), 2.2f);
            StrokeEllipse({pos.x, pos.y - 42.0f}, 24.0f + attack * 12.0f, 11.0f + attack * 5.0f, D2D1::ColorF(0xF6FF83, 0.26f + attack * 0.18f), 1.8f);
            break;
        case PlayerUnit::Dash:
            for (int i = 0; i < 3; ++i)
            {
                const float y = pos.y - 14.0f + static_cast<float>(i) * 12.0f;
                DrawLine({pos.x - dir * (38.0f + static_cast<float>(i) * 8.0f), y},
                         {pos.x - dir * (14.0f + attack * 16.0f), y - 2.0f},
                         D2D1::ColorF(accent.r, accent.g, accent.b, 0.36f + attack * 0.26f), 2.2f);
            }
            break;
        case PlayerUnit::Bell:
            StrokeEllipse(pos, 36.0f + pulse * 4.0f + attack * 18.0f, 18.0f + pulse * 2.0f + attack * 9.0f, D2D1::ColorF(0xF6FF83, 0.24f + attack * 0.16f), 1.7f);
            StrokeEllipse(pos, 50.0f + attack * 22.0f, 25.0f + attack * 10.0f, D2D1::ColorF(0xF6FF83, 0.14f + attack * 0.12f), 1.4f);
            break;
        case PlayerUnit::Titan:
            DrawLine({pos.x - 35.0f, pos.y + 34.0f}, {pos.x - 8.0f, pos.y + 24.0f}, soft, 3.2f);
            DrawLine({pos.x + 6.0f, pos.y + 26.0f}, {pos.x + 38.0f, pos.y + 35.0f}, soft, 3.2f);
            FillEllipse({pos.x, pos.y + 36.0f}, 46.0f + attack * 16.0f, 6.0f + attack * 3.0f, D2D1::ColorF(0xFFF0B5, 0.10f + attack * 0.12f));
            break;
        case PlayerUnit::Frost:
            DrawLine({pos.x, pos.y - 52.0f}, {pos.x, pos.y - 30.0f}, bright, 1.8f);
            DrawLine({pos.x - 12.0f, pos.y - 47.0f}, {pos.x + 12.0f, pos.y - 35.0f}, bright, 1.6f);
            DrawLine({pos.x + 12.0f, pos.y - 47.0f}, {pos.x - 12.0f, pos.y - 35.0f}, bright, 1.6f);
            StrokeEllipse(pos, 40.0f + attack * 14.0f, 23.0f + attack * 6.0f, D2D1::ColorF(0xD9FFF8, 0.12f + attack * 0.16f), 1.5f);
            break;
        case PlayerUnit::Comet:
            FillEllipse({pos.x - dir * 42.0f, pos.y + 11.0f}, 34.0f + attack * 16.0f, 7.0f + attack * 4.0f, D2D1::ColorF(0xFFB347, 0.16f + attack * 0.18f));
            DrawLine({pos.x - dir * 55.0f, pos.y + 2.0f}, {pos.x - dir * 16.0f, pos.y - 4.0f}, D2D1::ColorF(0xFFCA7A, 0.34f + attack * 0.24f), 2.4f);
            break;
        case PlayerUnit::Orbit:
        {
            StrokeEllipse(pos, unit.radius + 20.0f, unit.radius * 0.62f + 3.0f, bright, 1.8f);
            DrawLine({pos.x - dir * 30.0f, pos.y - 4.0f}, {pos.x + dir * 32.0f, pos.y - 4.0f}, D2D1::ColorF(0xDCE6FF, 0.46f + attack * 0.20f), 1.8f);
            break;
        }
        case PlayerUnit::Solar:
            for (int i = 0; i < 8; ++i)
            {
                const float a = static_cast<float>(i) / 8.0f * kPi * 2.0f + m_uiTime * 0.8f;
                DrawLine({pos.x + std::cos(a) * 31.0f, pos.y + std::sin(a) * 31.0f},
                         {pos.x + std::cos(a) * (39.0f + attack * 12.0f), pos.y + std::sin(a) * (39.0f + attack * 12.0f)},
                         D2D1::ColorF(0xFFB347, 0.28f + attack * 0.22f), 2.0f);
            }
            break;
        case PlayerUnit::Mint:
            StrokeEllipse(pos, 42.0f + attack * 18.0f, 42.0f + attack * 18.0f, D2D1::ColorF(0xD8FFF3, 0.13f + attack * 0.16f), 1.6f);
            DrawLine({pos.x - 12.0f, pos.y - 45.0f}, {pos.x + 12.0f, pos.y - 45.0f}, bright, 2.4f);
            DrawLine({pos.x, pos.y - 57.0f}, {pos.x, pos.y - 33.0f}, bright, 2.4f);
            break;
        case PlayerUnit::Drill:
            StrokeEllipse({pos.x + dir * 35.0f, pos.y + 2.0f}, 20.0f + attack * 9.0f, 8.0f + attack * 3.0f, bright, 2.0f);
            DrawLine({pos.x + dir * 21.0f, pos.y - 8.0f}, {pos.x + dir * 50.0f, pos.y + 12.0f}, D2D1::ColorF(0xFFF0C8, 0.68f), 1.6f);
            break;
        case PlayerUnit::Prism:
            DrawLine({pos.x, pos.y - 55.0f}, {pos.x + 16.0f, pos.y - 38.0f}, bright, 2.0f);
            DrawLine({pos.x + 16.0f, pos.y - 38.0f}, {pos.x, pos.y - 21.0f}, D2D1::ColorF(0x65D8FF, 0.64f), 2.0f);
            DrawLine({pos.x, pos.y - 21.0f}, {pos.x - 16.0f, pos.y - 38.0f}, D2D1::ColorF(0xF7D6FF, 0.64f), 2.0f);
            DrawLine({pos.x - 16.0f, pos.y - 38.0f}, {pos.x, pos.y - 55.0f}, bright, 2.0f);
            break;
        case PlayerUnit::Nebula:
        {
            StrokeEllipse(pos, unit.radius + 23.0f, unit.radius + 9.0f, D2D1::ColorF(0xC8B7FF, 0.22f + attack * 0.14f), 1.8f);
            StrokeEllipse(pos, unit.radius + 11.0f + attack * 10.0f, unit.radius * 0.55f + 5.0f, D2D1::ColorF(0xE5D9FF, 0.20f + attack * 0.18f), 1.5f);
            break;
        }
        }
        return;
    }

    if (unit.boss)
    {
        StrokeEllipse(pos, unit.radius + 24.0f + attack * 20.0f, unit.radius + 18.0f + attack * 12.0f, D2D1::ColorF(0xFFB347, 0.30f + attack * 0.16f), 2.6f);
        for (int i = 0; i < 6; ++i)
        {
            const float a = static_cast<float>(i) / 6.0f * kPi * 2.0f - m_uiTime * 0.9f;
            DrawLine({pos.x + std::cos(a) * 38.0f, pos.y + std::sin(a) * 34.0f},
                     {pos.x + std::cos(a) * (54.0f + attack * 18.0f), pos.y + std::sin(a) * (48.0f + attack * 14.0f)},
                     D2D1::ColorF(0xFF9BA8, 0.28f + attack * 0.18f), 2.4f);
        }
        return;
    }

    if (unit.elite)
    {
        StrokeEllipse({pos.x, pos.y - unit.radius - 12.0f}, unit.radius * 0.92f, 5.0f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.66f), 1.8f);
    }

    const EnemyUnit enemyType = static_cast<EnemyUnit>(unit.kind);
    if (unit.ranged || enemyType == EnemyUnit::Ring || enemyType == EnemyUnit::Tide || enemyType == EnemyUnit::Mirror)
    {
        StrokeEllipse({pos.x + dir * 28.0f, pos.y - 12.0f}, 18.0f + attack * 9.0f, 10.0f + attack * 5.0f, bright, 1.8f);
    }
    else if (enemyType == EnemyUnit::Brute || enemyType == EnemyUnit::Rust || enemyType == EnemyUnit::Quake || enemyType == EnemyUnit::Storm)
    {
        FillEllipse({pos.x, pos.y + unit.radius + 16.0f}, unit.radius * 1.35f + attack * 18.0f, 5.0f + attack * 3.0f, soft);
    }
    else
    {
        DrawLine({pos.x - dir * 28.0f, pos.y - 20.0f}, {pos.x - dir * 6.0f, pos.y - 7.0f}, soft, 2.0f);
        DrawLine({pos.x - dir * 30.0f, pos.y + 2.0f}, {pos.x - dir * 8.0f, pos.y + 6.0f}, soft, 1.8f);
    }
}

bool PawlineGameImpl::DrawUnitCharacterSprite(const Unit& unit, Vec2 pos, D2D1_COLOR_F accent)
{
    // Kenney Toon Characters CC0 포즈를 유닛별 아틀라스로 묶어 사용한다.
    // 구조: 유닛 종류마다 7행 모션을 가지고, 각 모션은 10열 프레임으로 재생된다.
    ID2D1Bitmap* sheet = unit.team == Team::Player ? m_playerUnitAtlas.Get() : m_enemyUnitAtlas.Get();
    if (!sheet)
    {
        return false;
    }

    int row = 0;
    if (!unit.alive || unit.animState == UnitAnimState::Death)
    {
        row = 4;
    }
    else if (unit.hitFlash > 0.0f || unit.animState == UnitAnimState::Hit)
    {
        row = 3;
    }
    else if (unit.animState == UnitAnimState::Windup)
    {
        row = 5;
    }
    else if (unit.animState == UnitAnimState::Attack || unit.animState == UnitAnimState::Recover)
    {
        row = 6;
    }
    else if (unit.animState == UnitAnimState::Move)
    {
        row = unit.speed > 86.0f ? 2 : 1;
    }

    const std::array<int, 7> frameCounts = {10, 10, 8, 10, 10, 8, 10};
    const int frameCount = frameCounts[static_cast<size_t>(row)];
    int frame = 0;
    if (row == 5 || row == 6)
    {
        frame = std::clamp(static_cast<int>(AttackProgress(unit) * static_cast<float>(frameCount)), 0, frameCount - 1);
    }
    else
    {
        const float frameRate = row == 2 ? 13.0f : (row == 1 ? 9.0f : 6.5f);
        frame = static_cast<int>(std::floor((unit.stateTime + unit.shakePhase * 0.01f) * frameRate)) % frameCount;
    }

    constexpr float kCellW = 128.0f;
    constexpr float kCellH = 128.0f;
    const int unitRow = std::max(0, unit.kind) * 7 + row;
    const D2D1_RECT_F source = D2D1::RectF(static_cast<float>(frame) * kCellW,
                                           static_cast<float>(unitRow) * kCellH,
                                           static_cast<float>(frame + 1) * kCellW,
                                           static_cast<float>(unitRow + 1) * kCellH);

    const float roleScale = unit.team == Team::Player ? 5.10f : 5.00f;
    const float eliteScale = unit.boss ? 1.28f : (unit.elite ? 1.14f : 1.0f);
    const float height = std::clamp(unit.radius * roleScale * eliteScale, 62.0f, unit.boss ? 190.0f : 148.0f);
    const float width = height * (kCellW / kCellH);
    const float bottom = pos.y + unit.radius + 21.0f;
    const D2D1_RECT_F destination = D2D1::RectF(pos.x - width * 0.5f,
                                                bottom - height,
                                                pos.x + width * 0.5f,
                                                bottom);

    FillEllipse({pos.x, pos.y + unit.radius + 10.0f}, width * 0.30f, 7.0f,
                D2D1::ColorF(0x000000, unit.team == Team::Player ? 0.26f : 0.32f));
    FillEllipse({pos.x, pos.y - unit.radius * 0.18f}, width * 0.32f, height * 0.33f,
                D2D1::ColorF(accent.r, accent.g, accent.b, 0.055f + AttackIntensity(unit) * 0.045f));
    DrawBitmap(sheet, destination, unit.team == Team::Player ? 0.98f : 0.96f, &source);

    if (unit.hitFlash > 0.0f)
    {
        FillEllipse({pos.x, pos.y - unit.radius * 0.14f}, width * 0.34f, height * 0.32f,
                    D2D1::ColorF(0xFFFFFF, 0.12f));
    }
    if (unit.elite || unit.boss)
    {
        StrokeEllipse({pos.x, pos.y - unit.radius * 0.06f}, width * 0.31f, height * 0.28f,
                      unit.boss ? D2D1::ColorF(0xFF9BA8, 0.58f) : D2D1::ColorF(accent.r, accent.g, accent.b, 0.46f),
                      unit.boss ? 3.0f : 2.0f);
    }
    return true;
}

void PawlineGameImpl::DrawPlayerUnit(const Unit& unit)
{
    const PlayerUnit playerType = static_cast<PlayerUnit>(unit.kind);
    const UnitStats stats = PlayerStats(playerType);
    Vec2 pos = UnitRenderPos(unit);
    const float attack = AttackIntensity(unit);
    const float windup = AttackWindup(unit);
    const float strike = AttackStrike(unit);
    const float recoil = AttackRecoil(unit);
    const float dir = unit.attackDir;
    const float flash = unit.hitFlash > 0.0f ? 1.0f : 0.0f;
    const float stride = UnitMoveStride(unit);
    const float step = UnitMoveStep(unit);
    const float hitPose = UnitHitPose(unit);
    const float deathPose = UnitDeathPose(unit);
    const float idleBreath = unit.animState == UnitAnimState::Idle ? std::sin(unit.stateTime * 3.0f + unit.shakePhase) : 0.0f;
    const float manualLean = strike * 8.0f - windup * 6.5f - recoil * 3.0f - hitPose * 5.0f;
    pos.x += dir * manualLean;
    pos.y += deathPose * 15.0f - step * 2.2f + idleBreath * 0.7f;
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, 0.96f);
    D2D1_COLOR_F body = stats.color;
    body.r = std::min(1.0f, body.r + flash * 0.25f);
    body.g = std::min(1.0f, body.g + flash * 0.25f);
    body.b = std::min(1.0f, body.b + flash * 0.25f);
    const float bodyRx = unit.radius * (1.0f + strike * 0.30f - windup * 0.10f + recoil * 0.07f + step * 0.05f + deathPose * 0.34f);
    const float bodyRy = unit.radius * (1.0f - strike * 0.18f + windup * 0.10f + recoil * 0.04f - step * 0.04f - deathPose * 0.42f);
    const float trailAlpha = Clamp01(strike * 0.55f + windup * 0.18f + step * 0.12f + hitPose * 0.24f);

    const bool spriteDrawn = DrawUnitCharacterSprite(unit, pos, stats.accent);
    if (!spriteDrawn)
    {
    if (trailAlpha > 0.02f)
    {
        // 시트 프레임 대신 잔상으로 걷기/공격 방향을 직접 보여준다.
        for (int i = 1; i <= 3; ++i)
        {
            const float t = static_cast<float>(i);
            const Vec2 ghost = {pos.x - dir * (t * (8.0f + strike * 9.0f)), pos.y + t * 1.6f + stride * t * 0.8f};
            FillEllipse(ghost, bodyRx * (1.0f - t * 0.13f), bodyRy * (1.0f - t * 0.10f),
                        D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, trailAlpha * (0.13f - t * 0.025f)));
        }
    }

    FillEllipse({pos.x, pos.y + unit.radius + 9.0f + deathPose * 3.0f}, unit.radius * (1.25f + deathPose * 0.35f), 7.0f + deathPose * 1.5f, D2D1::ColorF(0x000000, 0.28f));
    FillEllipse({pos.x - unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.46f, unit.radius * 0.46f, ink);
    FillEllipse({pos.x + unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.46f, unit.radius * 0.46f, ink);
    FillEllipse(pos, bodyRx + 3.4f, bodyRy + 3.4f, ink);
    FillEllipse({pos.x - unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.42f, unit.radius * 0.42f, body);
    FillEllipse({pos.x + unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.42f, unit.radius * 0.42f, body);
    FillEllipse(pos, bodyRx, bodyRy, body);
    FillEllipse({pos.x - unit.radius * 0.24f, pos.y - unit.radius * 0.46f}, unit.radius * 0.30f, unit.radius * 0.18f, D2D1::ColorF(0xFFFFFF, 0.18f));
    StrokeEllipse(pos, bodyRx, bodyRy, stats.accent, 2.4f);
    DrawUnitIdentityMark(unit, pos, stats.accent);
    if (strike > 0.0f || windup > 0.0f)
    {
        // 손처럼 보이던 공통 원형 파츠를 없애고, 공격 방향만 읽히는 짧은 로컬 빛으로 대체한다.
        const Vec2 handGlow = {pos.x + dir * (unit.radius * 0.72f + strike * 12.0f - windup * 5.0f),
                               pos.y + unit.radius * 0.16f - strike * 5.0f};
        FillEllipse(handGlow, unit.radius * (0.62f + strike * 0.35f), unit.radius * (0.22f + strike * 0.18f),
                    D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.06f + strike * 0.12f + windup * 0.03f));
    }
    if (IsUnitEvolved(playerType))
    {
        // 진화한 유닛은 머리 위의 작은 빛 장식과 공격 잔광으로 일반 유닛과 구분한다.
        const float crownPulse = 0.72f + std::sin(m_uiTime * 5.0f + static_cast<float>(unit.id)) * 0.16f;
        FillEllipse({pos.x, pos.y - unit.radius - 17.0f}, 18.0f, 5.0f, D2D1::ColorF(0xF6FF83, 0.18f * crownPulse));
        StrokeEllipse({pos.x, pos.y - unit.radius - 17.0f}, 18.0f, 5.0f, D2D1::ColorF(0xF6FF83, 0.72f * crownPulse), 2.0f);
        FillEllipse({pos.x - 12.0f, pos.y - unit.radius - 22.0f}, 3.5f, 3.5f, D2D1::ColorF(0xF6FF83, 0.92f));
        FillEllipse({pos.x, pos.y - unit.radius - 25.0f}, 4.5f, 4.5f, D2D1::ColorF(0xFFF4B8, 0.96f));
        FillEllipse({pos.x + 12.0f, pos.y - unit.radius - 22.0f}, 3.5f, 3.5f, D2D1::ColorF(0xF6FF83, 0.92f));
        if (attack > 0.0f)
        {
            FillEllipse(pos, unit.radius * (1.95f + attack * 0.75f), unit.radius * (1.15f + attack * 0.45f), D2D1::ColorF(0xF6FF83, 0.045f + attack * 0.055f));
        }
    }

    }
    DrawPlayerWeapon(unit, pos, stats, windup, strike, recoil);
    DrawUnitActionLines(unit, pos, stats.accent);

    if (!spriteDrawn)
    {
        FillEllipse({pos.x - unit.radius * 0.34f, pos.y - unit.radius * 0.12f}, 2.6f, 4.2f, D2D1::ColorF(0x071017));
        FillEllipse({pos.x + unit.radius * 0.34f, pos.y - unit.radius * 0.12f}, 2.6f, 4.2f, D2D1::ColorF(0x071017));
        DrawLine({pos.x - unit.radius * 0.25f, pos.y + unit.radius * 0.32f},
                 {pos.x + unit.radius * 0.25f, pos.y + unit.radius * 0.32f},
                 D2D1::ColorF(0x071017), 1.8f);
    }

    if (!spriteDrawn && attack > 0.0f)
    {
        const Vec2 front = {pos.x + dir * (unit.radius + 22.0f + attack * 14.0f), pos.y};
        switch (playerType)
        {
        case PlayerUnit::Paw:
            DrawLine({front.x - dir * 7.0f, front.y - 24.0f}, {front.x + dir * 12.0f, front.y + 13.0f}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.78f * attack), 3.2f);
            DrawLine({front.x - dir * 16.0f, front.y - 10.0f}, {front.x + dir * 14.0f, front.y + 22.0f}, D2D1::ColorF(0xFFFFFF, 0.32f * attack), 1.6f);
            break;
        case PlayerUnit::Box:
            FillRoundRect(D2D1::RectF(std::min(front.x, front.x + dir * 32.0f) - 8.0f, front.y - 24.0f, std::max(front.x, front.x + dir * 32.0f) + 8.0f, front.y + 22.0f), 5.0f, D2D1::ColorF(0xFFF0B5, 0.20f * attack));
            StrokeRoundRect(D2D1::RectF(std::min(front.x, front.x + dir * 32.0f) - 8.0f, front.y - 24.0f, std::max(front.x, front.x + dir * 32.0f) + 8.0f, front.y + 22.0f), 5.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.76f * attack), 2.5f);
            break;
        case PlayerUnit::Spark:
            StrokeEllipse({pos.x + dir * 25.0f, pos.y - 42.0f}, 18.0f + attack * 9.0f, 18.0f + attack * 9.0f, D2D1::ColorF(0xF6FF83, 0.62f * attack), 2.2f);
            DrawLine({pos.x + dir * 15.0f, pos.y - 37.0f}, front, D2D1::ColorF(0xF6FF83, 0.72f * attack), 2.4f);
            break;
        case PlayerUnit::Dash:
            DrawLine({pos.x - dir * 56.0f, pos.y + 17.0f}, {pos.x - dir * 10.0f, pos.y + 10.0f}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.72f * attack), 4.0f);
            DrawLine({pos.x - dir * 70.0f, pos.y + 1.0f}, {pos.x - dir * 18.0f, pos.y - 2.0f}, D2D1::ColorF(0xFFFFFF, 0.22f * attack), 2.0f);
            break;
        case PlayerUnit::Bell:
            StrokeEllipse(pos, 42.0f + attack * 22.0f, 22.0f + attack * 10.0f, D2D1::ColorF(0xF2C94C, 0.48f * attack), 2.4f);
            StrokeEllipse(front, 26.0f + attack * 18.0f, 15.0f + attack * 8.0f, D2D1::ColorF(0xF6FF83, 0.35f * attack), 1.8f);
            break;
        case PlayerUnit::Titan:
            DrawLine({front.x - dir * 28.0f, front.y + 30.0f}, {front.x + dir * 36.0f, front.y + 30.0f}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.76f * attack), 6.0f);
            StrokeEllipse({front.x, front.y + 22.0f}, 44.0f + attack * 28.0f, 14.0f + attack * 5.0f, D2D1::ColorF(0xFFF0B5, 0.38f * attack), 3.0f);
            break;
        case PlayerUnit::Frost:
            DrawLine({pos.x + dir * 10.0f, pos.y - 22.0f}, {front.x + dir * 24.0f, front.y + 4.0f}, D2D1::ColorF(0xD9FFF8, 0.86f * attack), 4.0f);
            DrawLine({front.x + dir * 10.0f, front.y - 18.0f}, {front.x + dir * 28.0f, front.y + 4.0f}, D2D1::ColorF(0xB9FFF5, 0.56f * attack), 2.2f);
            break;
        case PlayerUnit::Comet:
            DrawLine({pos.x - dir * 74.0f, pos.y + 18.0f}, {front.x, front.y + 2.0f}, D2D1::ColorF(0xFFCA7A, 0.72f * attack), 5.0f);
            FillEllipse({pos.x - dir * 62.0f, pos.y + 12.0f}, 9.0f + attack * 6.0f, 5.0f + attack * 4.0f, D2D1::ColorF(0xFFB347, 0.36f * attack));
            break;
        case PlayerUnit::Orbit:
            StrokeEllipse(pos, unit.radius + 24.0f + attack * 24.0f, unit.radius * 0.7f + attack * 12.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.60f * attack), 2.5f);
            FillEllipse({front.x, front.y - 18.0f}, 6.0f + attack * 5.0f, 6.0f + attack * 5.0f, stats.accent);
            break;
        case PlayerUnit::Solar:
            for (int i = -1; i <= 1; ++i)
            {
                DrawLine({pos.x, pos.y - 6.0f}, {front.x + dir * 28.0f, front.y + static_cast<float>(i) * 24.0f}, D2D1::ColorF(0xFFE66D, 0.52f * attack), 3.2f);
            }
            break;
        case PlayerUnit::Mint:
            StrokeEllipse(pos, 38.0f + attack * 18.0f, 38.0f + attack * 18.0f, D2D1::ColorF(0xD8FFF3, 0.36f * attack), 2.0f);
            DrawLine({front.x - 12.0f, front.y}, {front.x + 12.0f, front.y}, D2D1::ColorF(0xD8FFF3, 0.74f * attack), 2.8f);
            DrawLine({front.x, front.y - 12.0f}, {front.x, front.y + 12.0f}, D2D1::ColorF(0xD8FFF3, 0.74f * attack), 2.8f);
            break;
        case PlayerUnit::Drill:
            DrawLine({pos.x + dir * 16.0f, pos.y + 2.0f}, {front.x + dir * 44.0f, front.y + 2.0f}, D2D1::ColorF(0xFFF0C8, 0.86f * attack), 5.0f);
            StrokeEllipse({front.x + dir * 28.0f, front.y + 2.0f}, 22.0f, 8.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.66f * attack), 2.2f);
            break;
        case PlayerUnit::Prism:
            DrawLine({pos.x + dir * 20.0f, pos.y - 42.0f}, {front.x + dir * 50.0f, front.y - 12.0f}, D2D1::ColorF(0xF7D6FF, 0.86f * attack), 4.0f);
            DrawLine({pos.x + dir * 18.0f, pos.y - 24.0f}, {front.x + dir * 43.0f, front.y + 10.0f}, D2D1::ColorF(0x65D8FF, 0.35f * attack), 2.0f);
            break;
        case PlayerUnit::Nebula:
            StrokeEllipse(pos, unit.radius + 30.0f + attack * 20.0f, unit.radius + 12.0f + attack * 12.0f, D2D1::ColorF(0xC8B7FF, 0.52f * attack), 2.5f);
            StrokeEllipse(pos, unit.radius + 10.0f + attack * 16.0f, unit.radius + 34.0f + attack * 20.0f, D2D1::ColorF(0xF7D6FF, 0.42f * attack), 2.0f);
            break;
        }
    }

    if (!spriteDrawn)
    {
    if (playerType == PlayerUnit::Box)
    {
        FillRoundRect(D2D1::RectF(pos.x - 15.0f, pos.y - 4.0f, pos.x + 17.0f, pos.y + 21.0f), 5.0f, D2D1::ColorF(0xDCA85B, 0.72f));
        StrokeRoundRect(D2D1::RectF(pos.x - 15.0f, pos.y - 4.0f, pos.x + 17.0f, pos.y + 21.0f), 5.0f, D2D1::ColorF(0xFFF0B5), 1.3f);
    }
    else if (playerType == PlayerUnit::Spark)
    {
        DrawLine({pos.x + 8.0f, pos.y - 23.0f}, {pos.x + 20.0f, pos.y - 42.0f}, stats.accent, 3.0f);
        FillEllipse({pos.x + 22.0f, pos.y - 45.0f}, 5.5f, 5.5f, D2D1::ColorF(0xF6FF83));
    }
    else if (playerType == PlayerUnit::Dash)
    {
        DrawLine({pos.x - 30.0f, pos.y + 11.0f}, {pos.x - 16.0f, pos.y + 11.0f}, stats.accent, 2.6f);
        DrawLine({pos.x - 33.0f, pos.y + 1.0f}, {pos.x - 19.0f, pos.y + 1.0f}, stats.accent, 2.6f);
    }
    else if (playerType == PlayerUnit::Bell)
    {
        FillEllipse({pos.x, pos.y - 25.0f}, 7.0f, 5.5f, D2D1::ColorF(0xF2C94C));
        DrawLine({pos.x, pos.y - 19.0f}, {pos.x, pos.y - 10.0f}, stats.accent, 2.1f);
    }
    else if (playerType == PlayerUnit::Titan)
    {
        FillEllipse({pos.x - 26.0f, pos.y + 4.0f}, 8.0f, 11.0f, body);
        FillEllipse({pos.x + 26.0f, pos.y + 4.0f}, 8.0f, 11.0f, body);
    }
    else if (playerType == PlayerUnit::Frost)
    {
        StrokeRoundRect(D2D1::RectF(pos.x - 19.0f, pos.y + 5.0f, pos.x + 19.0f, pos.y + 27.0f), 6.0f, stats.accent, 2.3f);
        DrawLine({pos.x - 12.0f, pos.y + 14.0f}, {pos.x + 12.0f, pos.y + 14.0f}, stats.accent, 2.0f);
        FillEllipse({pos.x, pos.y - 28.0f}, 8.0f, 3.0f, D2D1::ColorF(0xE9FFFF, 0.72f));
    }
    else if (playerType == PlayerUnit::Comet)
    {
        DrawLine({pos.x - 34.0f, pos.y + 13.0f}, {pos.x - 17.0f, pos.y + 7.0f}, stats.accent, 3.0f);
        DrawLine({pos.x - 38.0f, pos.y + 2.0f}, {pos.x - 19.0f, pos.y - 3.0f}, stats.accent, 2.4f);
        FillEllipse({pos.x + 19.0f, pos.y - 22.0f}, 5.0f, 5.0f, D2D1::ColorF(0xFFCA7A));
    }
    else if (playerType == PlayerUnit::Orbit)
    {
        StrokeEllipse(pos, unit.radius + 14.0f, unit.radius * 0.58f, stats.accent, 2.2f);
        FillEllipse({pos.x + unit.radius + 13.0f, pos.y - 5.0f}, 4.0f, 4.0f, stats.accent);
    }
    else if (playerType == PlayerUnit::Solar)
    {
        DrawLine({pos.x, pos.y - unit.radius - 7.0f}, {pos.x, pos.y - unit.radius - 24.0f}, stats.accent, 3.0f);
        DrawLine({pos.x - 22.0f, pos.y - 17.0f}, {pos.x - 36.0f, pos.y - 31.0f}, stats.accent, 2.4f);
        DrawLine({pos.x + 22.0f, pos.y - 17.0f}, {pos.x + 36.0f, pos.y - 31.0f}, stats.accent, 2.4f);
        StrokeEllipse(pos, unit.radius + 5.0f, unit.radius + 5.0f, D2D1::ColorF(0xFFB347, 0.34f), 2.0f);
    }
    else if (playerType == PlayerUnit::Mint)
    {
        DrawLine({pos.x - 16.0f, pos.y - 27.0f}, {pos.x + 16.0f, pos.y - 27.0f}, stats.accent, 2.6f);
        DrawLine({pos.x, pos.y - 42.0f}, {pos.x, pos.y - 14.0f}, stats.accent, 2.6f);
        FillEllipse({pos.x, pos.y - 42.0f}, 5.0f, 5.0f, D2D1::ColorF(0xD8FFF3));
    }
    else if (playerType == PlayerUnit::Drill)
    {
        FillEllipse({pos.x + 24.0f, pos.y + 2.0f}, 14.0f, 7.0f, stats.accent);
        DrawLine({pos.x + 12.0f, pos.y + 2.0f}, {pos.x + 42.0f, pos.y + 2.0f}, D2D1::ColorF(0xFFF0C8), 3.0f);
        DrawLine({pos.x + 22.0f, pos.y - 5.0f}, {pos.x + 38.0f, pos.y + 9.0f}, D2D1::ColorF(0xFFF0C8), 1.6f);
    }
    else if (playerType == PlayerUnit::Prism)
    {
        StrokeEllipse(pos, 9.0f, unit.radius + 17.0f, stats.accent, 2.0f);
        DrawLine({pos.x + 11.0f, pos.y - 22.0f}, {pos.x + 32.0f, pos.y - 42.0f}, stats.accent, 2.6f);
        FillEllipse({pos.x + 34.0f, pos.y - 44.0f}, 4.0f, 4.0f, D2D1::ColorF(0xF7D6FF));
    }
    else if (playerType == PlayerUnit::Nebula)
    {
        StrokeEllipse(pos, unit.radius + 12.0f, unit.radius * 0.58f, stats.accent, 2.3f);
        StrokeEllipse(pos, unit.radius * 0.78f, unit.radius + 11.0f, D2D1::ColorF(0xC8B7FF, 0.55f), 1.9f);
        FillEllipse({pos.x + 30.0f, pos.y - 8.0f}, 5.0f, 5.0f, D2D1::ColorF(0xE5D9FF));
    }
    }
}

void PawlineGameImpl::DrawEnemyUnit(const Unit& unit)
{
    const EnemyUnit type = static_cast<EnemyUnit>(unit.kind);
    const UnitStats stats = GetEnemyStats(type, ThreatLevel());
    Vec2 pos = UnitRenderPos(unit);
    const float attack = AttackIntensity(unit);
    const float windup = AttackWindup(unit);
    const float strike = AttackStrike(unit);
    const float recoil = AttackRecoil(unit);
    const float dir = unit.attackDir;
    const float stride = UnitMoveStride(unit);
    const float step = UnitMoveStep(unit);
    const float hitPose = UnitHitPose(unit);
    const float deathPose = UnitDeathPose(unit);
    const float idleBreath = unit.animState == UnitAnimState::Idle ? std::sin(unit.stateTime * 2.7f + unit.shakePhase) : 0.0f;
    pos.x += dir * (strike * 8.5f - windup * 6.0f - recoil * 3.0f - hitPose * 5.5f);
    pos.y += deathPose * 16.0f - step * 1.8f + idleBreath * 0.6f;
    const D2D1_COLOR_F ink = D2D1::ColorF(0x08080F, 0.96f);
    D2D1_COLOR_F body = stats.color;
    if (unit.hitFlash > 0.0f)
    {
        body.r = std::min(1.0f, body.r + 0.28f);
        body.g = std::min(1.0f, body.g + 0.18f);
        body.b = std::min(1.0f, body.b + 0.18f);
    }

    const float bodyRx = unit.radius * (1.08f + strike * 0.30f - windup * 0.09f + recoil * 0.05f + step * 0.04f + deathPose * 0.30f);
    const float bodyRy = unit.radius * (1.0f - strike * 0.17f + windup * 0.09f - step * 0.03f - deathPose * 0.40f);
    const float trailAlpha = Clamp01(strike * 0.52f + windup * 0.16f + step * 0.10f + hitPose * 0.24f);

    const bool spriteDrawn = DrawUnitCharacterSprite(unit, pos, stats.accent);
    if (!spriteDrawn)
    {
    if (trailAlpha > 0.02f)
    {
        // 적도 같은 수동 포즈 규칙을 쓰지만, 더 거칠고 어두운 잔상으로 위협감을 준다.
        for (int i = 1; i <= 3; ++i)
        {
            const float t = static_cast<float>(i);
            const Vec2 ghost = {pos.x - dir * (t * (7.0f + strike * 10.0f)), pos.y + t * 1.8f - stride * t * 0.6f};
            FillEllipse(ghost, bodyRx * (1.0f - t * 0.12f), bodyRy * (1.0f - t * 0.11f),
                        D2D1::ColorF(stats.accent.r, stats.accent.g * 0.75f, stats.accent.b * 0.75f, trailAlpha * (0.12f - t * 0.024f)));
        }
    }

    FillEllipse({pos.x, pos.y + unit.radius + 9.0f + deathPose * 3.0f}, unit.radius * (1.25f + deathPose * 0.38f), 7.0f + deathPose * 1.5f, D2D1::ColorF(0x000000, 0.30f));
    FillEllipse(pos, bodyRx + 3.2f, bodyRy + 3.2f, ink);
    FillEllipse(pos, bodyRx, bodyRy, body);
    FillEllipse({pos.x - unit.radius * 0.18f, pos.y - unit.radius * 0.42f}, unit.radius * 0.28f, unit.radius * 0.15f, D2D1::ColorF(0xFFFFFF, 0.12f));
    StrokeEllipse(pos, bodyRx, bodyRy, stats.accent, 2.2f);
    DrawUnitIdentityMark(unit, pos, stats.accent);
    if (strike > 0.0f || windup > 0.0f)
    {
        // 적도 공통 손/발 원형을 쓰지 않고, 공격 순간의 색광으로만 타격 방향을 보여준다.
        const Vec2 clawGlow = {pos.x + dir * (unit.radius * 0.76f + strike * 12.0f - windup * 5.0f),
                               pos.y + unit.radius * 0.16f - strike * 4.0f};
        FillEllipse(clawGlow, unit.radius * (0.66f + strike * 0.32f), unit.radius * (0.22f + strike * 0.16f),
                    D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.06f + strike * 0.12f + windup * 0.03f));
    }
    }
    DrawEnemyWeapon(unit, pos, stats, windup, strike, recoil);
    DrawUnitActionLines(unit, pos, stats.accent);
    if (!spriteDrawn)
    {
        FillEllipse({pos.x - unit.radius * 0.32f, pos.y - unit.radius * 0.12f}, 3.0f, 4.6f, stats.accent);
        FillEllipse({pos.x + unit.radius * 0.32f, pos.y - unit.radius * 0.12f}, 3.0f, 4.6f, stats.accent);
        DrawLine({pos.x - unit.radius * 0.30f, pos.y + unit.radius * 0.34f},
                 {pos.x + unit.radius * 0.30f, pos.y + unit.radius * 0.25f},
                 stats.accent, 2.0f);
    }

    if (!spriteDrawn && attack > 0.0f)
    {
        const Vec2 front = {pos.x + dir * (unit.radius + 20.0f + attack * 12.0f), pos.y};
        if (type == EnemyUnit::Dust || type == EnemyUnit::Skitter || type == EnemyUnit::Rust)
        {
            DrawLine({front.x - dir * 16.0f, front.y - 14.0f}, {front.x + dir * 18.0f, front.y + 18.0f}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.70f * attack), 2.6f);
            DrawLine({front.x - dir * 12.0f, front.y + 18.0f}, {front.x + dir * 22.0f, front.y - 12.0f}, D2D1::ColorF(0xFFFFFF, 0.20f * attack), 1.4f);
        }
        else if (type == EnemyUnit::Brute || type == EnemyUnit::Storm || type == EnemyUnit::Quake)
        {
            StrokeEllipse({front.x, front.y + 24.0f}, 42.0f + attack * 26.0f, 12.0f + attack * 7.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.42f * attack), 3.2f);
            DrawLine({front.x - dir * 30.0f, front.y + 28.0f}, {front.x + dir * 34.0f, front.y + 28.0f}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.62f * attack), 4.0f);
        }
        else if (type == EnemyUnit::Sulfur || type == EnemyUnit::Spore || type == EnemyUnit::Tide)
        {
            StrokeEllipse(front, 30.0f + attack * 18.0f, 18.0f + attack * 9.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.44f * attack), 2.0f);
            DrawLine({pos.x + dir * 8.0f, pos.y - 18.0f}, {front.x + dir * 18.0f, front.y}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.68f * attack), 2.4f);
        }
        else if (type == EnemyUnit::Ring || type == EnemyUnit::Frost || type == EnemyUnit::Mirror)
        {
            DrawLine({pos.x + dir * 8.0f, pos.y - 22.0f}, {front.x + dir * 36.0f, front.y + 10.0f}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.78f * attack), 3.2f);
            DrawLine({front.x + dir * 10.0f, front.y - 16.0f}, {front.x + dir * 34.0f, front.y + 12.0f}, D2D1::ColorF(0xFFFFFF, 0.22f * attack), 1.6f);
        }
        else if (type == EnemyUnit::Void || type == EnemyUnit::Boss)
        {
            StrokeEllipse(pos, unit.radius + 26.0f + attack * 26.0f, unit.radius + 12.0f + attack * 14.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.50f * attack), 3.0f);
            StrokeEllipse(front, 30.0f + attack * 28.0f, 30.0f + attack * 28.0f, D2D1::ColorF(0xFF9BA8, 0.30f * attack), 2.0f);
        }
        else if (type == EnemyUnit::Flare || type == EnemyUnit::Comet)
        {
            DrawLine({pos.x - dir * 60.0f, pos.y + 14.0f}, {front.x + dir * 16.0f, front.y}, D2D1::ColorF(0xFFDB7A, 0.68f * attack), 4.6f);
            DrawLine({pos.x - dir * 72.0f, pos.y - 2.0f}, {front.x, front.y - 10.0f}, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.46f * attack), 2.6f);
        }
    }

    if (!spriteDrawn)
    {
    if (type == EnemyUnit::Sulfur)
    {
        FillEllipse({pos.x - 18.0f, pos.y - 22.0f}, 12.0f, 7.0f, D2D1::ColorF(0xFFD27A, 0.38f));
        FillEllipse({pos.x + 16.0f, pos.y - 25.0f}, 10.0f, 6.0f, D2D1::ColorF(0xFFD27A, 0.30f));
    }
    else if (type == EnemyUnit::Moss)
    {
        FillEllipse({pos.x - 13.0f, pos.y - 23.0f}, 10.0f, 6.0f, stats.accent);
        FillEllipse({pos.x + 10.0f, pos.y - 24.0f}, 9.0f, 5.0f, D2D1::ColorF(0xB8FF89, 0.82f));
    }
    else if (type == EnemyUnit::Rust)
    {
        DrawLine({pos.x - 18.0f, pos.y - 22.0f}, {pos.x - 31.0f, pos.y - 38.0f}, stats.accent, 3.0f);
        DrawLine({pos.x + 18.0f, pos.y - 22.0f}, {pos.x + 31.0f, pos.y - 38.0f}, stats.accent, 3.0f);
    }
    else if (type == EnemyUnit::Storm)
    {
        FillRoundRect(D2D1::RectF(pos.x - unit.radius - 5.0f, pos.y - 6.0f, pos.x + unit.radius + 5.0f, pos.y + 7.0f), 6.0f, D2D1::ColorF(0xF1D09A, 0.32f));
        StrokeEllipse(pos, unit.radius + 6.0f, unit.radius * 0.72f, stats.accent, 2.2f);
    }
    else if (type == EnemyUnit::Ring)
    {
        DrawLine({pos.x - 24.0f, pos.y - 25.0f}, {pos.x + 28.0f, pos.y + 22.0f}, stats.accent, 3.0f);
        FillEllipse({pos.x - 27.0f, pos.y - 28.0f}, 5.0f, 5.0f, stats.accent);
    }
    else if (type == EnemyUnit::Frost)
    {
        DrawLine({pos.x, pos.y - 23.0f}, {pos.x, pos.y - 38.0f}, stats.accent, 2.2f);
        DrawLine({pos.x - 10.0f, pos.y - 29.0f}, {pos.x + 10.0f, pos.y - 29.0f}, stats.accent, 2.2f);
        DrawLine({pos.x - 8.0f, pos.y + unit.radius + 8.0f}, {pos.x + 8.0f, pos.y + unit.radius + 8.0f}, D2D1::ColorF(0xE9FFFF, 0.62f), 2.0f);
    }
    else if (type == EnemyUnit::Tide)
    {
        DrawLine({pos.x - 23.0f, pos.y + 16.0f}, {pos.x - 36.0f, pos.y + 4.0f}, stats.accent, 3.0f);
        DrawLine({pos.x + 23.0f, pos.y + 16.0f}, {pos.x + 36.0f, pos.y + 4.0f}, stats.accent, 3.0f);
        StrokeEllipse(pos, unit.radius + 7.0f, unit.radius * 0.48f, D2D1::ColorF(0xBFD9FF, 0.30f), 2.0f);
    }
    else if (type == EnemyUnit::Void)
    {
        FillEllipse(pos, unit.radius * 0.44f, unit.radius * 0.44f, D2D1::ColorF(0x08080F, 0.65f));
        StrokeEllipse(pos, unit.radius + 9.0f, unit.radius + 4.0f, stats.accent, 2.2f);
    }
    else if (type == EnemyUnit::Flare)
    {
        DrawLine({pos.x, pos.y - unit.radius - 4.0f}, {pos.x, pos.y - unit.radius - 20.0f}, stats.accent, 2.6f);
        DrawLine({pos.x - 18.0f, pos.y - 12.0f}, {pos.x - 31.0f, pos.y - 24.0f}, stats.accent, 2.2f);
        DrawLine({pos.x + 18.0f, pos.y - 12.0f}, {pos.x + 31.0f, pos.y - 24.0f}, stats.accent, 2.2f);
    }
    else if (type == EnemyUnit::Spore)
    {
        FillEllipse({pos.x, pos.y - unit.radius - 6.0f}, unit.radius * 0.72f, unit.radius * 0.42f, stats.accent);
        FillEllipse({pos.x - 10.0f, pos.y - unit.radius - 4.0f}, 3.0f, 3.0f, D2D1::ColorF(0xFFF0FF));
        FillEllipse({pos.x + 8.0f, pos.y - unit.radius - 8.0f}, 3.5f, 3.5f, D2D1::ColorF(0xFFF0FF));
    }
    else if (type == EnemyUnit::Quake)
    {
        FillRoundRect(D2D1::RectF(pos.x - unit.radius - 8.0f, pos.y + 3.0f, pos.x + unit.radius + 8.0f, pos.y + 22.0f), 8.0f, D2D1::ColorF(0x2A211D, 0.42f));
        DrawLine({pos.x - 24.0f, pos.y - 22.0f}, {pos.x - 42.0f, pos.y - 37.0f}, stats.accent, 4.0f);
        DrawLine({pos.x + 24.0f, pos.y - 22.0f}, {pos.x + 42.0f, pos.y - 37.0f}, stats.accent, 4.0f);
    }
    else if (type == EnemyUnit::Mirror)
    {
        FillRoundRect(D2D1::RectF(pos.x - 12.0f, pos.y - 28.0f, pos.x + 12.0f, pos.y + 4.0f), 3.0f, D2D1::ColorF(0xEAF7FF, 0.58f));
        StrokeRoundRect(D2D1::RectF(pos.x - 12.0f, pos.y - 28.0f, pos.x + 12.0f, pos.y + 4.0f), 3.0f, stats.accent, 1.8f);
        DrawLine({pos.x - 7.0f, pos.y - 21.0f}, {pos.x + 7.0f, pos.y - 9.0f}, stats.accent, 1.8f);
    }
    else if (type == EnemyUnit::Comet)
    {
        DrawLine({pos.x + 19.0f, pos.y + 12.0f}, {pos.x + 38.0f, pos.y + 16.0f}, stats.accent, 3.4f);
        DrawLine({pos.x + 17.0f, pos.y - 2.0f}, {pos.x + 38.0f, pos.y - 5.0f}, D2D1::ColorF(0xFFDB7A, 0.82f), 2.8f);
        FillEllipse({pos.x - 18.0f, pos.y - 20.0f}, 5.0f, 5.0f, D2D1::ColorF(0xFFDB7A));
    }
    }

    if (unit.boss || unit.elite || type == EnemyUnit::Boss)
    {
        StrokeEllipse(pos, unit.radius + 8.0f, unit.radius + 8.0f, D2D1::ColorF(0xFF9BA8, 0.62f), 3.0f);
        DrawPixelTextCentered(unit.boss || type == EnemyUnit::Boss ? L"BOSS" : L"ELITE", D2D1::RectF(pos.x - 38.0f, pos.y - 59.0f, pos.x + 38.0f, pos.y - 35.0f), 1.8f, D2D1::ColorF(0xFFE3E8), 1.0f);
    }
}

void PawlineGameImpl::DrawUnitHp(const Unit& unit)
{
    const Vec2 pos = UnitRenderPos(unit);
    const float pct = Clamp01(unit.hp / unit.maxHp);
    const float width = std::max(28.0f, unit.radius * 2.45f);
    D2D1_RECT_F back = D2D1::RectF(pos.x - width * 0.5f, pos.y - unit.radius - 14.0f, pos.x + width * 0.5f, pos.y - unit.radius - 8.0f);
    FillRoundRect(back, 2.0f, D2D1::ColorF(0x071017, 0.88f));
    FillRoundRect(D2D1::RectF(back.left, back.top, back.left + width * pct, back.bottom), 2.0f,
                  unit.team == Team::Player ? D2D1::ColorF(0x65B8FF) : D2D1::ColorF(0xFF9BA8));
}

void PawlineGameImpl::DrawUnitStunEffect(const Unit& unit)
{
    if (unit.stunTimer <= 0.0f && unit.knockbackTimer <= 0.0f)
    {
        return;
    }

    const Vec2 pos = UnitRenderPos(unit);
    const float stunAlpha = Clamp01(unit.stunTimer / 0.80f);
    const float knockAlpha = Clamp01(unit.knockbackTimer / 0.34f);
    const float dir = unit.knockbackVelocity >= 0.0f ? 1.0f : -1.0f;

    if (knockAlpha > 0.0f)
    {
        for (int i = 0; i < 4; ++i)
        {
            const float t = static_cast<float>(i);
            const float y = pos.y - unit.radius * 0.20f + (t - 1.5f) * 8.0f;
            const float len = 20.0f + t * 7.0f + unit.radius * 0.35f;
            DrawLine({pos.x - dir * (unit.radius + 10.0f + t * 5.0f), y},
                     {pos.x - dir * (unit.radius + len), y - 4.0f},
                     D2D1::ColorF(0xEAF7FF, 0.18f * knockAlpha), 2.0f + knockAlpha * 1.6f);
        }
        StrokeEllipse({pos.x, pos.y + unit.radius * 0.58f}, unit.radius * (1.45f + knockAlpha * 0.42f), unit.radius * 0.38f,
                      D2D1::ColorF(0xFFFFFF, 0.14f * knockAlpha), 1.8f);
    }

    if (stunAlpha <= 0.0f)
    {
        return;
    }

    const float spin = m_uiTime * 5.0f + unit.shakePhase;
    const Vec2 halo = {pos.x, pos.y - unit.radius - 25.0f};
    StrokeEllipse(halo, unit.radius * 0.84f, 8.0f, D2D1::ColorF(0xF6FF83, 0.35f * stunAlpha), 1.6f);
    for (int i = 0; i < 4; ++i)
    {
        const float a = spin + static_cast<float>(i) * 1.5708f;
        const Vec2 star = {halo.x + std::cos(a) * unit.radius * 0.76f, halo.y + std::sin(a) * 6.5f};
        DrawLine({star.x - 5.0f, star.y}, {star.x + 5.0f, star.y}, D2D1::ColorF(0xFFF4B8, 0.72f * stunAlpha), 1.8f);
        DrawLine({star.x, star.y - 5.0f}, {star.x, star.y + 5.0f}, D2D1::ColorF(0xFFF4B8, 0.58f * stunAlpha), 1.8f);
        FillEllipse(star, 2.4f, 2.4f, D2D1::ColorF(0xF6FF83, 0.78f * stunAlpha));
    }
    DrawPixelTextCentered(L"STUN", D2D1::RectF(halo.x - 34.0f, halo.y - 26.0f, halo.x + 34.0f, halo.y - 10.0f), 1.35f, D2D1::ColorF(0xFFF4B8), 0.86f * stunAlpha);
}

void PawlineGameImpl::DrawProjectiles()
{
    for (const Projectile& projectile : m_projectiles)
    {
        if (projectile.life <= 0.0f)
        {
            continue;
        }

        Vec2 dir = Normalize(projectile.pos - projectile.lastPos);
        if (Length(dir) <= 0.0001f)
        {
            dir = projectile.team == Team::Player ? Vec2{1.0f, 0.0f} : Vec2{-1.0f, 0.0f};
        }
        const Vec2 normal = {-dir.y, dir.x};
        const float pulse = 0.62f + 0.38f * std::sin(projectile.age * 17.0f + projectile.spin);
        const float wave = std::sin(projectile.wobble + projectile.age * 6.0f) * projectile.radius * 0.44f;
        const Vec2 drawPos = projectile.pos + normal * wave;
        const Vec2 tail = projectile.lastPos - dir * (projectile.radius * 2.8f);

        FillEllipse({drawPos.x - dir.x * 8.0f, drawPos.y + projectile.radius + 13.0f}, projectile.radius * 2.1f, projectile.radius * 0.44f, D2D1::ColorF(0x000000, 0.20f));
        DrawLine(tail, drawPos, D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.12f), projectile.radius * 3.9f);
        DrawLine(tail + normal * wave * 0.35f, drawPos, D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.42f), std::max(2.0f, projectile.radius * 1.35f));
        DrawLine(tail, drawPos, D2D1::ColorF(0xFFFFFF, 0.28f + pulse * 0.20f), std::max(1.0f, projectile.radius * 0.34f));
        FillEllipse(drawPos, projectile.radius * (3.1f + pulse * 0.65f), projectile.radius * (2.3f + pulse * 0.40f), D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.080f + pulse * 0.035f));
        DrawProjectileShape(projectile, drawPos, dir, normal, pulse);
    }
}

void PawlineGameImpl::DrawProjectileShape(const Projectile& projectile, Vec2 drawPos, Vec2 dir, Vec2 normal, float pulse)
{
    const D2D1_COLOR_F glow = D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.34f + pulse * 0.18f);
    const D2D1_COLOR_F core = projectile.color;
    const D2D1_COLOR_F white = D2D1::ColorF(0xFFFFFF, 0.72f + pulse * 0.20f);
    const float r = projectile.radius;

    switch (projectile.visual)
    {
    case ProjectileVisual::Bolt:
    {
        const Vec2 p0 = drawPos - dir * (r * 2.8f);
        const Vec2 p1 = drawPos - dir * (r * 1.2f) + normal * (r * 0.85f);
        const Vec2 p2 = drawPos + dir * (r * 0.25f) - normal * (r * 0.75f);
        const Vec2 p3 = drawPos + dir * (r * 2.4f);
        DrawLine(p0, p1, glow, r * 1.5f);
        DrawLine(p1, p2, glow, r * 1.5f);
        DrawLine(p2, p3, glow, r * 1.5f);
        DrawLine(p0, p1, white, std::max(1.2f, r * 0.45f));
        DrawLine(p1, p2, white, std::max(1.2f, r * 0.45f));
        DrawLine(p2, p3, white, std::max(1.2f, r * 0.45f));
        FillEllipse(drawPos, r * 0.62f, r * 0.62f, core);
        break;
    }
    case ProjectileVisual::BellWave:
        StrokeEllipse(drawPos, r * (2.4f + pulse * 1.2f), r * (1.0f + pulse * 0.48f), glow, 2.2f);
        StrokeEllipse(drawPos - dir * r * 1.7f, r * 2.0f, r * 0.78f, D2D1::ColorF(0xFFF4B8, 0.34f), 1.8f);
        FillEllipse(drawPos, r * 0.86f, r * 0.72f, D2D1::ColorF(0xFFF4B8));
        DrawLine(drawPos - normal * r * 0.9f, drawPos + normal * r * 0.9f, D2D1::ColorF(0x071017, 0.42f), 1.2f);
        break;
    case ProjectileVisual::OrbitStar:
    {
        StrokeEllipse(drawPos, r * 2.2f, r * 0.82f, glow, 2.0f);
        StrokeEllipse(drawPos, r * 0.82f, r * 2.1f, D2D1::ColorF(0xFFFFFF, 0.18f + pulse * 0.10f), 1.5f);
        for (int i = 0; i < 4; ++i)
        {
            const float a = projectile.spin + static_cast<float>(i) * kPi * 0.5f;
            Vec2 star = {drawPos.x + std::cos(a) * r * 1.55f, drawPos.y + std::sin(a) * r * 0.78f};
            FillEllipse(star, r * 0.38f, r * 0.38f, i == 0 ? white : core);
        }
        FillEllipse(drawPos, r * 0.74f, r * 0.74f, core);
        break;
    }
    case ProjectileVisual::PrismShard:
    case ProjectileVisual::MirrorShard:
    {
        const Vec2 front = drawPos + dir * r * 2.3f;
        const Vec2 back = drawPos - dir * r * 1.6f;
        const Vec2 top = drawPos - normal * r * 1.1f;
        const Vec2 bottom = drawPos + normal * r * 1.1f;
        DrawLine(front, top, glow, 3.0f);
        DrawLine(top, back, glow, 3.0f);
        DrawLine(back, bottom, glow, 3.0f);
        DrawLine(bottom, front, glow, 3.0f);
        DrawLine(front, back, D2D1::ColorF(0xFFFFFF, 0.40f), 1.6f);
        DrawLine(top, bottom, projectile.visual == ProjectileVisual::MirrorShard ? D2D1::ColorF(0x65D8FF, 0.55f) : D2D1::ColorF(0x65D8FF, 0.34f), 1.4f);
        FillEllipse(drawPos, r * 0.72f, r * 0.48f, D2D1::ColorF(core.r, core.g, core.b, 0.78f));
        break;
    }
    case ProjectileVisual::NebulaOrb:
    case ProjectileVisual::VoidOrb:
        FillEllipse(drawPos, r * 1.85f, r * 1.45f, D2D1::ColorF(core.r, core.g, core.b, 0.26f));
        FillEllipse(drawPos, r * 1.04f, r * 1.04f, projectile.visual == ProjectileVisual::VoidOrb ? D2D1::ColorF(0x061019, 0.92f) : D2D1::ColorF(0x2B1948, 0.86f));
        FillEllipse({drawPos.x - r * 0.20f, drawPos.y - r * 0.24f}, r * 0.58f, r * 0.58f, core);
        StrokeEllipse(drawPos, r * (2.25f + pulse * 0.9f), r * (1.20f + pulse * 0.45f), glow, 2.3f);
        DrawLine(drawPos - normal * r * 1.8f, drawPos + normal * r * 1.8f, D2D1::ColorF(0xFFFFFF, 0.22f), 1.2f);
        break;
    case ProjectileVisual::MintPulse:
        FillEllipse(drawPos, r * 1.35f, r * 1.35f, D2D1::ColorF(0xD8FFF3, 0.78f));
        DrawLine(drawPos - Vec2{r, 0.0f}, drawPos + Vec2{r, 0.0f}, D2D1::ColorF(0x061019, 0.30f), 1.4f);
        DrawLine(drawPos - Vec2{0.0f, r}, drawPos + Vec2{0.0f, r}, D2D1::ColorF(0x061019, 0.30f), 1.4f);
        StrokeEllipse(drawPos, r * (1.9f + pulse * 0.6f), r * (1.9f + pulse * 0.6f), D2D1::ColorF(0x61E6B0, 0.36f), 2.0f);
        break;
    case ProjectileVisual::FrostShard:
        DrawLine(drawPos - dir * r * 2.2f, drawPos + dir * r * 2.6f, D2D1::ColorF(0xD9FFF8, 0.82f), 3.0f);
        DrawLine(drawPos - normal * r * 1.3f, drawPos + normal * r * 1.3f, D2D1::ColorF(0xB9FFF5, 0.72f), 2.2f);
        DrawLine(drawPos - dir * r * 0.7f - normal * r * 0.8f, drawPos + dir * r * 0.9f + normal * r * 0.8f, D2D1::ColorF(0xFFFFFF, 0.46f), 1.5f);
        FillEllipse(drawPos, r * 0.62f, r * 0.62f, core);
        break;
    case ProjectileVisual::AcidGlob:
        FillEllipse(drawPos, r * 1.4f, r * 1.05f, D2D1::ColorF(0xB8FF89, 0.58f));
        FillEllipse(drawPos - dir * r * 0.42f + normal * r * 0.20f, r * 0.84f, r * 0.72f, core);
        FillEllipse(drawPos + dir * r * 1.2f - normal * r * 0.55f, r * 0.34f, r * 0.34f, D2D1::ColorF(0xFFF4B8, 0.82f));
        FillEllipse(drawPos - dir * r * 1.4f + normal * r * 0.72f, r * 0.28f, r * 0.28f, D2D1::ColorF(0xB8FF89, 0.70f));
        break;
    case ProjectileVisual::TideWave:
        for (int i = -2; i <= 2; ++i)
        {
            const float offset = static_cast<float>(i) * r * 0.62f;
            const Vec2 a = drawPos - dir * r * 2.0f + normal * offset;
            const Vec2 b = drawPos + dir * r * 2.1f - normal * (offset * 0.42f);
            DrawLine(a, b, D2D1::ColorF(0xBFD9FF, 0.28f + pulse * 0.16f), 1.6f + static_cast<float>(2 - std::abs(i)) * 0.8f);
        }
        FillEllipse(drawPos, r * 1.08f, r * 0.72f, D2D1::ColorF(0x75A7FF, 0.72f));
        FillEllipse(drawPos + normal * r * 1.2f, r * 0.32f, r * 0.32f, D2D1::ColorF(0xE7F0FF, 0.72f));
        break;
    case ProjectileVisual::SolarSpark:
        FillEllipse(drawPos - dir * r * 1.4f, r * 2.6f, r * 0.62f, D2D1::ColorF(0xFF6A3D, 0.32f));
        DrawLine(drawPos - dir * r * 3.0f, drawPos + dir * r * 2.5f, D2D1::ColorF(0xFFDB7A, 0.84f), r * 0.72f);
        FillEllipse(drawPos + dir * r * 1.1f, r * 0.90f, r * 0.90f, D2D1::ColorF(0xFFF4B8));
        for (int i = -1; i <= 1; ++i)
        {
            DrawLine(drawPos, drawPos - dir * r * 1.4f + normal * static_cast<float>(i) * r * 1.4f, D2D1::ColorF(0xFFE66D, 0.36f), 1.3f);
        }
        break;
    case ProjectileVisual::SporeSeed:
        FillEllipse(drawPos, r * 1.16f, r * 0.92f, D2D1::ColorF(0xFFB6E8, 0.76f));
        StrokeEllipse(drawPos, r * 1.5f, r * 1.05f, D2D1::ColorF(0xFFCADF, 0.46f), 1.8f);
        for (int i = 0; i < 3; ++i)
        {
            const float a = projectile.spin + static_cast<float>(i) * 2.1f;
            FillEllipse({drawPos.x + std::cos(a) * r * 0.62f, drawPos.y + std::sin(a) * r * 0.42f}, r * 0.22f, r * 0.22f, D2D1::ColorF(0x061019, 0.44f));
        }
        break;
    }
}

void PawlineGameImpl::DrawBeams()
{
    for (const BeamEffect& beam : m_beams)
    {
        const float alpha = Clamp01(beam.life / beam.maxLife);
        const Vec2 dir = Normalize(beam.end - beam.start);
        const Vec2 normal = {-dir.y, dir.x};
        const float shimmer = std::sin((m_stageTime + m_uiTime) * 68.0f + beam.start.x * 0.05f) * 1.8f * alpha;
        const Vec2 start = beam.start + normal * shimmer;
        const Vec2 end = beam.end + normal * shimmer;

        DrawLine(start, end, FadeColor(beam.color, 0.11f * alpha), beam.width * 4.8f);
        DrawLine(start, end, FadeColor(beam.color, 0.34f * alpha), beam.width * 2.2f);
        DrawLine(start, end, FadeColor(D2D1::ColorF(0xFFFFFF), 0.80f * alpha), std::max(1.0f, beam.width * 0.42f));
    }
}

void PawlineGameImpl::DrawSparkLines()
{
    for (const SparkLine& line : m_sparkLines)
    {
        const float alpha = Clamp01(line.life / line.maxLife);
        const Vec2 mid = (line.start + line.end) * 0.5f;
        // 충돌 파편은 선 대신 빛점과 작은 블룸으로 표현해, 외부 이펙트 시트가 주인공처럼 보이게 한다.
        FillEllipse(mid, line.width * (5.6f + alpha * 2.4f), line.width * (2.8f + alpha * 1.2f), FadeColor(line.color, 0.13f * alpha));
        FillEllipse(line.end, line.width * (2.6f + alpha * 2.6f), line.width * (1.8f + alpha * 1.4f), FadeColor(line.color, 0.36f * alpha));
        FillEllipse(line.end, line.width * (1.0f + alpha), line.width * (1.0f + alpha), FadeColor(D2D1::ColorF(0xFFFFFF), 0.30f * alpha));
    }
}

void PawlineGameImpl::DrawRings()
{
    for (const RingEffect& ring : m_rings)
    {
        const float alpha = Clamp01(ring.life / ring.maxLife);
        D2D1_COLOR_F color = ring.color;
        color.a *= alpha;
        StrokeEllipse(ring.pos, ring.radius, ring.radius * 0.55f, color, ring.width * (0.5f + alpha));
    }
}

void PawlineGameImpl::DrawParticles()
{
    for (const Particle& particle : m_particles)
    {
        const float alpha = Clamp01(particle.life / particle.maxLife);
        D2D1_COLOR_F color = particle.color;
        color.a *= alpha;
        switch (particle.kind)
        {
        case ParticleKind::Glow:
            FillEllipse(particle.pos, particle.radius * (1.8f + alpha), particle.radius * (1.8f + alpha), D2D1::ColorF(color.r, color.g, color.b, color.a * 0.16f));
            FillEllipse(particle.pos, particle.radius * (0.55f + alpha * 0.35f), particle.radius * (0.55f + alpha * 0.35f), D2D1::ColorF(color.r, color.g, color.b, color.a * 0.40f));
            break;
        case ParticleKind::Smoke:
        case ParticleKind::Dust:
            FillEllipse(particle.pos, particle.radius * (1.25f + (1.0f - alpha) * 0.75f), particle.radius * (0.62f + (1.0f - alpha) * 0.34f), D2D1::ColorF(color.r, color.g, color.b, color.a * (particle.kind == ParticleKind::Dust ? 0.34f : 0.24f)));
            break;
        case ParticleKind::Shard:
        {
            const Vec2 dir = Normalize(particle.vel);
            const Vec2 a = particle.pos - dir * (particle.radius * 1.8f);
            const Vec2 b = particle.pos + dir * (particle.radius * 1.9f);
            DrawLine(a, b, D2D1::ColorF(0x061019, 0.55f * alpha), particle.radius + 1.8f);
            DrawLine(a, b, color, std::max(1.0f, particle.radius * 0.55f));
            FillEllipse(particle.pos, particle.radius * 0.42f, particle.radius * 0.42f, D2D1::ColorF(0xFFFFFF, 0.28f * alpha));
            break;
        }
        case ParticleKind::Ember:
            DrawLine(particle.pos - Normalize(particle.vel) * (particle.radius * 3.0f), particle.pos, D2D1::ColorF(color.r, color.g, color.b, color.a * 0.34f), particle.radius * 1.3f);
            FillEllipse(particle.pos, particle.radius * (0.8f + alpha), particle.radius * (0.8f + alpha), D2D1::ColorF(color.r, color.g, color.b, color.a * 0.18f));
            FillEllipse(particle.pos, particle.radius * 0.72f, particle.radius * 0.72f, color);
            break;
        case ParticleKind::Snow:
            DrawLine({particle.pos.x - particle.radius, particle.pos.y}, {particle.pos.x + particle.radius, particle.pos.y}, color, 1.2f);
            DrawLine({particle.pos.x, particle.pos.y - particle.radius}, {particle.pos.x, particle.pos.y + particle.radius}, color, 1.2f);
            break;
        case ParticleKind::Bubble:
            FillEllipse(particle.pos, particle.radius * 1.4f, particle.radius * 1.0f, D2D1::ColorF(color.r, color.g, color.b, color.a * 0.08f));
            StrokeEllipse(particle.pos, particle.radius * 1.1f, particle.radius * 0.86f, color, 1.4f);
            break;
        case ParticleKind::Dot:
        default:
            FillEllipse(particle.pos, particle.radius * (1.4f + alpha), particle.radius * (1.4f + alpha), D2D1::ColorF(color.r, color.g, color.b, color.a * 0.18f));
            FillEllipse(particle.pos, particle.radius * (0.6f + alpha), particle.radius * (0.6f + alpha), color);
            break;
        }
    }
}

void PawlineGameImpl::DrawShaderPostProcess()
{
    // This mirrors shaders/pawline_vfx.hlsl using Direct2D primitives. Combat
    // attacks use local light in DrawUnitLighting; this pass keeps scene-level
    // space texture, vignette, camera hints, and cannon-scale feedback.
    const StageDefinition stage = CurrentStage();
    const D2D1_RECT_F arena = D2D1::RectF(24.0f, kBattleTop, 1256.0f, kBattleBottom);
    const float action = Clamp01(m_cannonFlash / 0.42f);
    const float feedback = PostFxFeedbackIntensity();
    const float chroma = feedback * (m_reduceFlashes ? 0.38f : 1.0f);
    const float scanOffset = std::fmod(m_stageTime * 22.0f, 14.0f);

    for (float y = kBattleTop + 4.0f + scanOffset; y < kBattleBottom; y += 14.0f)
    {
        DrawLine({arena.left + 6.0f, y}, {arena.right - 6.0f, y}, FadeColor(stage.lineColor, 0.010f + action * 0.016f + feedback * 0.006f), 1.0f);
    }

    for (int i = 0; i < 26; ++i)
    {
        const float x = 62.0f + static_cast<float>((i * 211 + m_selectedStage * 37) % 1150);
        const float y = 118.0f + static_cast<float>((i * 89 + m_selectedStage * 53) % 462);
        const float twinkle = 0.25f + 0.75f * Hash01(static_cast<float>(i), static_cast<float>(m_selectedStage), std::floor(m_stageTime * 3.0f));
        const float radius = 1.0f + static_cast<float>(i % 3) * 0.7f;
        FillEllipse({x, y}, radius + twinkle * (1.5f + feedback * 1.2f), radius + twinkle * (1.5f + feedback * 1.2f), D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.035f + twinkle * 0.055f + feedback * 0.018f));
    }

    // 투사체 주변에만 얇은 블룸을 더해 화면 전체가 번쩍이지 않게 한다.
    for (const Projectile& projectile : m_projectiles)
    {
        if (projectile.life <= 0.0f)
        {
            continue;
        }
        const Vec2 screen = WorldToScreen(projectile.pos);
        if (screen.x < arena.left - 80.0f || screen.x > arena.right + 80.0f || screen.y < arena.top - 60.0f || screen.y > arena.bottom + 60.0f)
        {
            continue;
        }
        const float pulse = 0.58f + 0.42f * std::sin(projectile.age * 15.0f + projectile.spin);
        FillEllipse(screen, projectile.radius * (5.8f + pulse * 1.6f + feedback * 2.2f), projectile.radius * (3.8f + pulse * 1.0f + feedback * 1.4f), D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.035f + pulse * 0.026f + feedback * 0.020f));
        FillEllipse({screen.x, screen.y + projectile.radius + 14.0f}, projectile.radius * 2.4f, projectile.radius * 0.48f, D2D1::ColorF(0x000000, 0.12f));
        StrokeEllipse(screen, projectile.radius * (2.6f + pulse * 0.8f), projectile.radius * (1.6f + pulse * 0.5f), D2D1::ColorF(0xFFFFFF, 0.055f + pulse * 0.045f), 1.2f);
    }

    // 외부 이미지 이펙트 주변에 후처리용 로컬 라이트를 한 번 더 올린다.
    // 화면 전체를 밝히지 않고, 타격 지점 근처만 빛나게 해서 그림자가 살아 보이게 한다.
    for (const ImageVfx& effect : m_imageVfx)
    {
        const float alpha = Clamp01(effect.life / effect.maxLife);
        if (alpha <= 0.0f)
        {
            continue;
        }
        const Vec2 screen = WorldToScreen(effect.pos);
        if (screen.x < arena.left - 120.0f || screen.x > arena.right + 120.0f ||
            screen.y < arena.top - 100.0f || screen.y > arena.bottom + 100.0f)
        {
            continue;
        }
        const float pulse = std::sin((1.0f - alpha) * kPi);
        const float strongKind = effect.kind == ImageVfxKind::Fire || effect.kind == ImageVfxKind::Thunder ||
                                 effect.kind == ImageVfxKind::Holy || effect.kind == ImageVfxKind::Wind ||
                                 effect.kind == ImageVfxKind::Thrust || effect.kind == ImageVfxKind::Explosion ||
                                 effect.kind == ImageVfxKind::FireBreath || effect.kind == ImageVfxKind::EnergyImpact ||
                                 effect.kind == ImageVfxKind::MagicMirror || effect.kind == ImageVfxKind::ThunderSplash ||
                                 effect.kind == ImageVfxKind::WaterBallImpact ? 1.0f : 0.72f;
        FillEllipse(screen, effect.size * (0.82f + pulse * 0.28f), effect.size * (0.38f + pulse * 0.16f),
                    D2D1::ColorF(effect.color.r, effect.color.g, effect.color.b, effect.color.a * alpha * (0.022f + feedback * 0.014f) * strongKind));
        FillEllipse({screen.x, screen.y + effect.size * 0.22f}, effect.size * 0.42f, effect.size * 0.08f,
                    D2D1::ColorF(0x000000, 0.045f * alpha));
    }

    for (const BeamEffect& beam : m_beams)
    {
        const float alpha = Clamp01(beam.life / beam.maxLife);
        if (alpha <= 0.0f)
        {
            continue;
        }
        const Vec2 start = WorldToScreen(beam.start);
        const Vec2 end = WorldToScreen(beam.end);
        DrawLine(start, end, D2D1::ColorF(beam.color.r, beam.color.g, beam.color.b, (0.018f + feedback * 0.018f) * alpha), beam.width * (9.0f + feedback * 3.0f));
        if (feedback > 0.05f)
        {
            DrawLine({start.x, start.y - 2.0f}, {end.x, end.y - 2.0f}, D2D1::ColorF(0x65D8FF, 0.075f * chroma * alpha), std::max(1.0f, beam.width * 1.2f));
            DrawLine({start.x, start.y + 2.0f}, {end.x, end.y + 2.0f}, D2D1::ColorF(0xFF6A8A, 0.075f * chroma * alpha), std::max(1.0f, beam.width * 1.2f));
        }
    }

    if (m_selectedStage >= 8)
    {
        const float heat = m_selectedStage == 9 ? 1.0f : 0.55f;
        for (int i = 0; i < 9; ++i)
        {
            const float y = 132.0f + static_cast<float>(i) * 48.0f;
            const float wave = std::sin(m_stageTime * 2.6f + static_cast<float>(i) * 0.9f) * 18.0f;
            DrawLine({54.0f + wave, y}, {1226.0f + wave * 0.35f, y + 16.0f}, D2D1::ColorF(0xFFB347, 0.030f * heat), 7.0f);
        }
    }

    float actionLevel = 0.0f;
    for (const Unit& unit : m_units)
    {
        if (!unit.alive)
        {
            continue;
        }
        actionLevel = std::max(actionLevel, AttackStrike(unit) + AttackWindup(unit) * 0.25f);
    }
    actionLevel = Clamp01(actionLevel);

    if (actionLevel > 0.0f)
    {
        for (const Unit& unit : m_units)
        {
            const float strike = AttackStrike(unit);
            if (!unit.alive || strike <= 0.0f)
            {
                continue;
            }
            const Vec2 screen = WorldToScreen(UnitRenderPos(unit));
            if (screen.x < arena.left - 80.0f || screen.x > arena.right + 80.0f)
            {
                continue;
            }
            const D2D1_COLOR_F accent = unit.team == Team::Player
                                            ? PlayerStats(static_cast<PlayerUnit>(unit.kind)).accent
                                            : GetEnemyStats(static_cast<EnemyUnit>(unit.kind), ThreatLevel()).accent;
            FillEllipse({screen.x + unit.attackDir * 20.0f, screen.y - 4.0f}, unit.radius * (3.1f + strike * 2.2f + feedback * 0.65f), unit.radius * (1.62f + strike * 1.02f + feedback * 0.30f), D2D1::ColorF(accent.r, accent.g, accent.b, 0.034f + strike * 0.070f + feedback * 0.018f));
            FillEllipse({screen.x - 18.0f, screen.y + unit.radius + 14.0f}, unit.radius * 2.0f, unit.radius * 0.50f, D2D1::ColorF(0x000000, 0.18f + strike * 0.09f));
            StrokeEllipse({screen.x, screen.y + 4.0f}, unit.radius + 30.0f * strike, unit.radius * 0.60f + 16.0f * strike, D2D1::ColorF(accent.r, accent.g, accent.b, 0.23f * strike), 2.0f + strike * 1.9f);
            if (feedback > 0.04f)
            {
                DrawLine({screen.x - unit.attackDir * (unit.radius + 42.0f), screen.y - 10.0f},
                         {screen.x + unit.attackDir * (unit.radius + 72.0f), screen.y - 10.0f},
                         D2D1::ColorF(accent.r, accent.g, accent.b, 0.11f * feedback), 2.4f + strike * 1.8f);
            }
            DrawLine({screen.x - unit.radius * 0.55f, screen.y - unit.radius * 0.72f},
                     {screen.x + unit.radius * 0.55f, screen.y - unit.radius * 0.58f},
                     D2D1::ColorF(0xFFFFFF, 0.055f + strike * 0.11f), 1.6f);
        }
    }

    for (int yIndex = 0; yIndex < 12; ++yIndex)
    {
        const float y = kBattleTop + 22.0f + static_cast<float>(yIndex) * 42.0f;
        const float verticalEdge = 1.0f - std::min(Clamp01((y - kBattleTop) / 128.0f), Clamp01((kBattleBottom - y) / 128.0f));
        for (int xIndex = 0; xIndex < 18; ++xIndex)
        {
            const float x = arena.left + 34.0f + static_cast<float>(xIndex) * 68.0f + static_cast<float>(yIndex % 2) * 20.0f;
            const float horizontalEdge = 1.0f - std::min(Clamp01((x - arena.left) / 180.0f), Clamp01((arena.right - x) / 180.0f));
            const float mask = std::max(horizontalEdge, verticalEdge);
            if (mask <= 0.12f)
            {
                continue;
            }
            const float pulse = 0.72f + 0.28f * Hash01(static_cast<float>(xIndex), static_cast<float>(yIndex), std::floor(m_stageTime * 2.0f));
            FillEllipse({x, y}, 2.0f + mask * 3.0f * pulse, 2.0f + mask * 3.0f * pulse, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.020f + mask * 0.034f));
        }
    }

    FillRect(D2D1::RectF(arena.left, arena.top, arena.right, arena.top + 44.0f), D2D1::ColorF(0x000000, 0.095f));
    FillRect(D2D1::RectF(arena.left, arena.bottom - 48.0f, arena.right, arena.bottom), D2D1::ColorF(0x000000, 0.115f));
    FillRect(D2D1::RectF(arena.left, arena.top, arena.left + 52.0f, arena.bottom), D2D1::ColorF(0x000000, 0.085f));
    FillRect(D2D1::RectF(arena.right - 52.0f, arena.top, arena.right, arena.bottom), D2D1::ColorF(0x000000, 0.085f));

    if (feedback > 0.0f)
    {
        const float offset = 2.0f + chroma * 5.0f;
        FillRect(D2D1::RectF(arena.left, arena.top, arena.right, arena.top + 62.0f), D2D1::ColorF(0x000000, 0.060f * feedback));
        FillRect(D2D1::RectF(arena.left, arena.bottom - 70.0f, arena.right, arena.bottom), D2D1::ColorF(0x000000, 0.075f * feedback));
        StrokeRoundRect(D2D1::RectF(arena.left - offset, arena.top - 1.0f, arena.right - offset, arena.bottom + 1.0f), 8.0f, D2D1::ColorF(0x65D8FF, 0.18f * chroma), 1.6f);
        StrokeRoundRect(D2D1::RectF(arena.left + offset, arena.top + 1.0f, arena.right + offset, arena.bottom - 1.0f), 8.0f, D2D1::ColorF(0xFF6A8A, 0.16f * chroma), 1.6f);
        for (int i = 0; i < 3; ++i)
        {
            const float y = kBattleTop + 90.0f + static_cast<float>(i) * 132.0f + std::sin(m_uiTime * 4.0f + static_cast<float>(i)) * 18.0f;
            DrawLine({arena.left + 70.0f, y}, {arena.right - 70.0f, y + 10.0f}, D2D1::ColorF(0xFFFFFF, 0.030f * feedback), 3.0f);
        }
    }

    if (action > 0.0f)
    {
        FillRoundRect(D2D1::RectF(arena.left + 2.0f, arena.top + 2.0f, arena.right - 2.0f, arena.bottom - 2.0f), 8.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, action * 0.045f));
        StrokeRoundRect(D2D1::RectF(arena.left + 6.0f, arena.top + 4.0f, arena.right + 2.0f, arena.bottom + 3.0f), 8.0f, D2D1::ColorF(0xFF6A8A, action * 0.26f), 1.4f);
        StrokeRoundRect(D2D1::RectF(arena.left - 2.0f, arena.top - 3.0f, arena.right - 6.0f, arena.bottom - 4.0f), 8.0f, D2D1::ColorF(0x65D8FF, action * 0.24f), 1.4f);
    }

    StrokeRoundRect(InflateRectF(arena, 1.0f, 1.0f), 9.0f, D2D1::ColorF(0x061019, 0.78f), 4.0f);
    StrokeRoundRect(arena, 8.0f, FadeColor(stage.lineColor, 0.22f + action * 0.16f + actionLevel * 0.10f + feedback * 0.12f), 1.8f + feedback * 1.2f);

    const float leftHint = m_mouse.y >= kBattleTop && m_mouse.y <= kBattleBottom ? Clamp01((118.0f - m_mouse.x) / 118.0f) : 0.0f;
    const float rightHint = m_mouse.y >= kBattleTop && m_mouse.y <= kBattleBottom ? Clamp01((m_mouse.x - (kWidth - 118.0f)) / 118.0f) : 0.0f;
    if (leftHint > 0.0f && m_cameraTargetX > 1.0f)
    {
        FillRect(D2D1::RectF(0.0f, kBattleTop, 74.0f, kBattleBottom), D2D1::ColorF(0x65B8FF, 0.10f * leftHint));
        DrawLine({34.0f, kBattleTop + 28.0f}, {18.0f, (kBattleTop + kBattleBottom) * 0.5f}, D2D1::ColorF(0xF3FBFF, 0.34f * leftHint), 2.4f);
        DrawLine({18.0f, (kBattleTop + kBattleBottom) * 0.5f}, {34.0f, kBattleBottom - 28.0f}, D2D1::ColorF(0xF3FBFF, 0.34f * leftHint), 2.4f);
    }
    if (rightHint > 0.0f && m_cameraTargetX < kCameraMaxX - 1.0f)
    {
        FillRect(D2D1::RectF(kWidth - 74.0f, kBattleTop, kWidth, kBattleBottom), D2D1::ColorF(0xF6FF83, 0.10f * rightHint));
        DrawLine({kWidth - 34.0f, kBattleTop + 28.0f}, {kWidth - 18.0f, (kBattleTop + kBattleBottom) * 0.5f}, D2D1::ColorF(0xF3FBFF, 0.34f * rightHint), 2.4f);
        DrawLine({kWidth - 18.0f, (kBattleTop + kBattleBottom) * 0.5f}, {kWidth - 34.0f, kBattleBottom - 28.0f}, D2D1::ColorF(0xF3FBFF, 0.34f * rightHint), 2.4f);
    }
}

void PawlineGameImpl::DrawScreenFlash()
{
    if (m_screenFlash <= 0.0f)
    {
        return;
    }

    const float alpha = Clamp01(m_screenFlash / 0.26f);
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0xF6FF83, 0.055f * alpha * (m_reduceFlashes ? 0.28f : 1.0f)));
}

void PawlineGameImpl::DrawFloatTexts()
{
    for (const FloatText& text : m_floatTexts)
    {
        const float alpha = Clamp01(text.life / text.maxLife);
        D2D1_COLOR_F color = text.color;
        color.a *= alpha;
        DrawPixelTextCentered(text.text, D2D1::RectF(text.pos.x - 76.0f, text.pos.y - 17.0f, text.pos.x + 76.0f, text.pos.y + 19.0f), 2.2f, color, alpha);
    }
}

void PawlineGameImpl::DrawUiPulses()
{
    for (const UiPulse& pulse : m_uiPulses)
    {
        const float alpha = Clamp01(pulse.life / pulse.maxLife);
        FillEllipse(pulse.pos, pulse.radius * 0.82f, pulse.radius * 0.82f, D2D1::ColorF(pulse.color.r, pulse.color.g, pulse.color.b, 0.055f * alpha));
        StrokeEllipse(pulse.pos, pulse.radius, pulse.radius, D2D1::ColorF(pulse.color.r, pulse.color.g, pulse.color.b, 0.42f * alpha), 2.0f);
        StrokeEllipse(pulse.pos, pulse.radius * 0.58f, pulse.radius * 0.58f, D2D1::ColorF(0xFFFFFF, 0.20f * alpha), 1.0f);
    }
}

void PawlineGameImpl::DrawTelegraphs()
{
    // 위험 예고는 전장 좌표에서 먼저 그린다. 남은 시간이 짧을수록
    // 더 밝게 깜빡여서 플레이어가 대응 타이밍을 읽을 수 있게 한다.
    for (const Telegraph& telegraph : m_telegraphs)
    {
        const float pct = 1.0f - Clamp01(telegraph.life / std::max(0.01f, telegraph.maxLife));
        const float blink = 0.45f + 0.55f * std::abs(std::sin((m_stageTime + m_uiTime) * (8.0f + pct * 20.0f)));
        const float alpha = 0.12f + pct * 0.22f + blink * 0.08f;
        const D2D1_COLOR_F fill = D2D1::ColorF(telegraph.color.r, telegraph.color.g, telegraph.color.b, alpha);
        const D2D1_COLOR_F stroke = D2D1::ColorF(telegraph.color.r, telegraph.color.g, telegraph.color.b, 0.48f + pct * 0.32f);

        if (telegraph.shape == TelegraphShape::Circle)
        {
            FillEllipse(telegraph.start, telegraph.radius, telegraph.radius * 0.58f, fill);
            StrokeEllipse(telegraph.start, telegraph.radius, telegraph.radius * 0.58f, stroke, 2.4f + pct * 3.0f);
            StrokeEllipse(telegraph.start, telegraph.radius * (0.42f + pct * 0.20f), telegraph.radius * (0.25f + pct * 0.10f), D2D1::ColorF(0xFFFFFF, 0.16f + pct * 0.22f), 1.4f);
            for (int i = 0; i < 8; ++i)
            {
                const float angle = static_cast<float>(i) / 8.0f * kPi * 2.0f + m_stageTime * 1.5f;
                const Vec2 outer = {telegraph.start.x + std::cos(angle) * telegraph.radius,
                                    telegraph.start.y + std::sin(angle) * telegraph.radius * 0.58f};
                const Vec2 inner = {telegraph.start.x + std::cos(angle) * telegraph.radius * (0.84f - pct * 0.14f),
                                    telegraph.start.y + std::sin(angle) * telegraph.radius * (0.49f - pct * 0.08f)};
                DrawLine(inner, outer, D2D1::ColorF(0xFFFFFF, 0.20f + pct * 0.24f), 1.6f);
            }
        }
        else if (telegraph.shape == TelegraphShape::Line)
        {
            DrawLine(telegraph.start, telegraph.end, D2D1::ColorF(telegraph.color.r, telegraph.color.g, telegraph.color.b, alpha * 0.46f), telegraph.width);
            DrawLine(telegraph.start, telegraph.end, stroke, 3.0f + pct * 4.0f);
            DrawLine(telegraph.start, telegraph.end, D2D1::ColorF(0xFFFFFF, 0.20f + pct * 0.22f), 1.2f + pct * 1.4f);
            for (int i = 0; i < 5; ++i)
            {
                const float t = (static_cast<float>(i) + 0.5f) / 5.0f;
                const Vec2 mark = telegraph.start + (telegraph.end - telegraph.start) * t;
                StrokeEllipse(mark, 12.0f + pct * 7.0f, 5.0f + pct * 3.0f, D2D1::ColorF(0xFFFFFF, 0.16f + pct * 0.24f), 1.2f);
            }
        }
        else
        {
            FillRoundRect(D2D1::RectF(kPlayerBaseX + 28.0f, kLaneY - kLaneHalfHeight - 18.0f, kEnemyBaseX - 28.0f, kLaneY + kLaneHalfHeight + 18.0f), 18.0f, fill);
            StrokeRoundRect(D2D1::RectF(kPlayerBaseX + 28.0f, kLaneY - kLaneHalfHeight - 18.0f, kEnemyBaseX - 28.0f, kLaneY + kLaneHalfHeight + 18.0f), 18.0f, stroke, 2.0f + pct * 2.6f);
            FillRoundRect(D2D1::RectF(kPlayerBaseX + 70.0f, kLaneY - 6.0f, kEnemyBaseX - 70.0f, kLaneY + 6.0f),
                          6.0f,
                          D2D1::ColorF(0xFFFFFF, 0.045f + pct * 0.065f));
        }

        const float labelLift = telegraph.shape == TelegraphShape::Circle ? telegraph.radius * 0.62f + 42.0f : 52.0f;
        const Vec2 labelCenter = telegraph.shape == TelegraphShape::Line ? (telegraph.start + telegraph.end) * 0.5f : telegraph.start;
        DrawPixelTextCentered(L"WARNING",
                              D2D1::RectF(labelCenter.x - 78.0f, labelCenter.y - labelLift, labelCenter.x + 78.0f, labelCenter.y - labelLift + 24.0f),
                              1.8f,
                              D2D1::ColorF(0xF3FBFF),
                              0.70f + pct * 0.30f);
        DrawPixelTextCentered(ToWideFloat(std::max(0.0f, telegraph.life), 1) + L"S",
                              D2D1::RectF(labelCenter.x - 46.0f, labelCenter.y - labelLift + 25.0f, labelCenter.x + 46.0f, labelCenter.y - labelLift + 45.0f),
                              1.45f,
                              D2D1::ColorF(0xF6FF83),
                              0.74f + pct * 0.26f);
    }
}

void PawlineGameImpl::DrawStageGimmickOverlay()
{
    if (m_screen != GameScreen::Playing && m_screen != GameScreen::Result)
    {
        return;
    }

    const StageDefinition stage = CurrentStage();
    const float interval = std::max(0.1f, GimmickInterval());
    const float ready = 1.0f - Clamp01(m_stageGimmickTimer / interval);
    const D2D1_RECT_F meter = D2D1::RectF(470.0f, 84.0f, 810.0f, 93.0f);
    DrawPixelTextCentered(L"EVENT",
                          D2D1::RectF(meter.left - 118.0f, meter.top - 9.0f, meter.left - 14.0f, meter.bottom + 10.0f),
                          1.55f,
                          D2D1::ColorF(0xEAF7FF),
                          0.96f);
    DrawPixelTextCentered(ToWideInt(static_cast<int>(std::ceil(std::max(0.0f, m_stageGimmickTimer)))) + L"S",
                          D2D1::RectF(meter.right + 14.0f, meter.top - 9.0f, meter.right + 86.0f, meter.bottom + 10.0f),
                          1.45f,
                          D2D1::ColorF(0xF6FF83),
                          0.96f);
    FillRoundRect(meter, 4.0f, D2D1::ColorF(0x061019, 0.70f));
    FillRoundRect(D2D1::RectF(meter.left, meter.top, meter.left + (meter.right - meter.left) * ready, meter.bottom), 4.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.62f));
    StrokeRoundRect(meter, 4.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.56f), 1.0f);

    if (m_stageGimmickPulse > 0.0f)
    {
        const float pulse = Clamp01(m_stageGimmickPulse / 1.35f);
        const D2D1_RECT_F arena = D2D1::RectF(24.0f, kBattleTop, 1256.0f, kBattleBottom);
        FillRoundRect(arena, 8.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.055f * pulse));
        StrokeRoundRect(InflateRectF(arena, -8.0f, -8.0f), 8.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.28f * pulse), 2.8f);
        DrawPixelTextCentered(L"PLANET EVENT", D2D1::RectF(520.0f, 116.0f, 760.0f, 146.0f), 2.8f, D2D1::ColorF(0xF3FBFF), pulse);
    }
}

void PawlineGameImpl::DrawBossPresentation()
{
    if (m_screen != GameScreen::Playing && m_screen != GameScreen::Result)
    {
        return;
    }

    const StageDefinition stage = CurrentStage();
    const auto bossRef = FindBossUnit();

    if (m_bossBannerTimer > 0.0f)
    {
        const float alpha = Clamp01(m_bossBannerTimer / 0.55f);
        const float slide = (1.0f - Clamp01(m_bossBannerTimer / 3.15f)) * 18.0f;
        const D2D1_RECT_F panel = D2D1::RectF(286.0f, 198.0f + slide, 994.0f, 314.0f + slide);
        FillRect(D2D1::RectF(0.0f, kBattleTop, kWidth, kBattleBottom), D2D1::ColorF(0x000000, 0.24f * alpha));
        DrawCartoonPanel(panel, D2D1::ColorF(0x190D0F, 0.96f * alpha), D2D1::ColorF(0xFF9BA8, alpha), true);
        const D2D1_RECT_F inner = InflateRectF(panel, -16.0f, -14.0f);
        FillRoundRect(inner, 10.0f, D2D1::ColorF(0x071017, 0.62f * alpha));
        FillEllipse({inner.left + 72.0f, inner.top + 44.0f}, 32.0f, 32.0f, D2D1::ColorF(0xFFB347, 0.11f * alpha));
        FillEllipse({inner.left + 72.0f, inner.top + 44.0f}, 15.0f, 15.0f, D2D1::ColorF(0xFFB347, 0.42f * alpha));
        StrokeEllipse({inner.left + 72.0f, inner.top + 44.0f}, 28.0f, 28.0f, D2D1::ColorF(0xFFB347, 0.40f * alpha), 2.0f);
        DrawLine({inner.left + 116.0f, inner.top + 26.0f}, {inner.right - 24.0f, inner.top + 26.0f}, D2D1::ColorF(0xFFB347, 0.42f * alpha), 2.0f);
        DrawLine({inner.left + 116.0f, inner.bottom - 24.0f}, {inner.right - 24.0f, inner.bottom - 24.0f}, D2D1::ColorF(0xFF9BA8, 0.32f * alpha), 2.0f);
        for (int i = 0; i < 11; ++i)
        {
            const float x = panel.left + 18.0f + static_cast<float>(i) * 64.0f;
            DrawLine({x, panel.top + 10.0f}, {x + 38.0f, panel.bottom - 10.0f}, D2D1::ColorF(0xFFB347, 0.075f * alpha), 4.0f);
        }
        DrawPixelTextCentered(L"WARNING", D2D1::RectF(panel.left + 30.0f, panel.top + 22.0f, panel.right - 30.0f, panel.top + 62.0f), 4.4f, D2D1::ColorF(0xFFB347), alpha);
        DrawPixelTextCentered(L"BOSS INCOMING", D2D1::RectF(panel.left + 30.0f, panel.top + 70.0f, panel.right - 30.0f, panel.bottom - 18.0f), 3.2f, D2D1::ColorF(0xF3FBFF), alpha);
    }

    if (m_bossPhaseBannerTimer > 0.0f)
    {
        const float alpha = m_bossPhaseBannerMax > 0.0f ? Clamp01(m_bossPhaseBannerTimer / m_bossPhaseBannerMax) : 0.0f;
        const float pop = 1.0f + std::sin((1.0f - alpha) * kPi) * 0.06f;
        const D2D1_RECT_F panel = D2D1::RectF(344.0f, 212.0f, 936.0f, 296.0f);
        const D2D1_COLOR_F accent = m_bossPhaseBannerLevel >= 3 ? D2D1::ColorF(0xFFB347, alpha) : D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, alpha);
        FillRect(D2D1::RectF(0.0f, kBattleTop, kWidth, kBattleBottom), D2D1::ColorF(0x000000, 0.14f * alpha));
        DrawCartoonPanel(panel, D2D1::ColorF(0x130B10, 0.92f * alpha), accent, true);
        DrawPixelTextCentered(m_bossPhaseBannerLevel >= 3 ? L"FINAL PHASE" : L"PHASE 2", D2D1::RectF(panel.left + 20.0f, panel.top + 14.0f, panel.right - 20.0f, panel.top + 50.0f), 3.8f * pop, D2D1::ColorF(0xF3FBFF), alpha);
        DrawPixelTextCentered(m_bossPhaseBannerLevel >= 3 ? L"PATTERN OVERDRIVE" : L"PATTERN SHIFT", D2D1::RectF(panel.left + 26.0f, panel.top + 50.0f, panel.right - 26.0f, panel.bottom - 10.0f), 2.3f, D2D1::ColorF(0xF6FF83), alpha);
        DrawLine({panel.left + 18.0f, panel.bottom - 9.0f}, {panel.right - 18.0f, panel.bottom - 9.0f}, D2D1::ColorF(0xFFFFFF, 0.22f * alpha), 2.0f);
    }

    if (!bossRef)
    {
        return;
    }

    const Unit& boss = bossRef->get();
    const float pct = Clamp01(boss.hp / boss.maxHp);
    const UnitStats stats = GetEnemyStats(static_cast<EnemyUnit>(boss.kind), ThreatLevel());
    const D2D1_RECT_F panel = D2D1::RectF(378.0f, 104.0f, 902.0f, 142.0f);
    DrawCartoonPanel(panel, D2D1::ColorF(0x0D1118, 0.94f), D2D1::ColorF(0xFF9BA8));
    DrawPixelText(L"BOSS", {panel.left + 18.0f, panel.top + 10.0f}, 2.4f, D2D1::ColorF(0xFFB347), 1.0f);
    DrawPixelTextCentered(stats.name, D2D1::RectF(panel.left + 82.0f, panel.top + 7.0f, panel.left + 254.0f, panel.bottom - 7.0f), 2.2f, D2D1::ColorF(0xF3FBFF), 1.0f);
    const D2D1_RECT_F bar = D2D1::RectF(panel.left + 270.0f, panel.top + 13.0f, panel.right - 18.0f, panel.bottom - 13.0f);
    FillRoundRect(bar, 6.0f, D2D1::ColorF(0x061019, 0.94f));
    FillRoundRect(D2D1::RectF(bar.left, bar.top, bar.left + (bar.right - bar.left) * pct, bar.bottom), 6.0f, D2D1::ColorF(0xFF9BA8));
    FillRoundRect(D2D1::RectF(bar.left, bar.top, bar.left + (bar.right - bar.left) * pct, bar.top + 7.0f), 5.0f, D2D1::ColorF(0xFFFFFF, 0.15f));
    for (float mark : {0.25f, 0.50f})
    {
        const float x = bar.left + (bar.right - bar.left) * mark;
        DrawLine({x, bar.top - 2.0f}, {x, bar.bottom + 2.0f}, D2D1::ColorF(0xF6FF83, 0.55f), 1.4f);
    }
}

void PawlineGameImpl::DrawTutorialTips()
{
    if (m_screen != GameScreen::Playing || m_stageTime > 13.0f || m_escapeMenuOpen)
    {
        return;
    }

    const float fadeIn = Clamp01(m_stageTime / 0.8f);
    const float fadeOut = Clamp01((13.0f - m_stageTime) / 1.1f);
    const float alpha = std::min(fadeIn, fadeOut);
    const StageDefinition stage = CurrentStage();

    auto tip = [&](D2D1_RECT_F rect, const std::wstring& key, const std::wstring& body, D2D1_COLOR_F accent) {
        const float localAlpha = alpha * (Contains(rect, m_mouse) ? 0.18f : 1.0f);
        DrawCartoonPanel(rect, D2D1::ColorF(0x071017, 0.86f * localAlpha), FadeColor(accent, localAlpha), false);
        DrawPixelText(key, {rect.left + 14.0f, rect.top + 12.0f}, 2.25f, D2D1::ColorF(0xF3FBFF, localAlpha), localAlpha);
        DrawOutlinedString(body, D2D1::RectF(rect.left + 18.0f, rect.top + 40.0f, rect.right - 18.0f, rect.bottom - 8.0f), m_smallFormat, D2D1::ColorF(0xDFF2FF, localAlpha), 0.72f * localAlpha);
        return localAlpha;
    };

    if (m_stageTime < 3.25f)
    {
        const float tipAlpha = tip(D2D1::RectF(58.0f, 512.0f, 390.0f, 590.0f), L"GUIDE 1", L"카드나 KEY 1~5로 유닛을 소환해.", D2D1::ColorF(0x65B8FF, alpha));
        DrawLine({126.0f, 590.0f}, {92.0f, 628.0f}, D2D1::ColorF(0x65B8FF, 0.42f * tipAlpha), 2.4f);
        DrawLine({92.0f, 628.0f}, {106.0f, 616.0f}, D2D1::ColorF(0x65B8FF, 0.42f * tipAlpha), 2.4f);
    }
    else if (m_stageTime < 6.5f)
    {
        const float tipAlpha = tip(D2D1::RectF(436.0f, 512.0f, 772.0f, 590.0f), L"GUIDE 2", L"월렛은 비용 감소, 회복, 보급 펄스를 만든다.", D2D1::ColorF(0xB8FF89, alpha));
        DrawLine({650.0f, 590.0f}, {736.0f, 628.0f}, D2D1::ColorF(0xB8FF89, 0.42f * tipAlpha), 2.4f);
    }
    else if (m_stageTime < 9.75f)
    {
        const float tipAlpha = tip(D2D1::RectF(880.0f, 118.0f, 1198.0f, 196.0f), L"GUIDE 3", L"마우스를 가장자리로 가져가거나 드래그해서 전선을 봐.", stage.lineColor);
        DrawLine({880.0f, 158.0f}, {804.0f, 104.0f}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.42f * tipAlpha), 2.4f);
    }
    else
    {
        const float tipAlpha = tip(D2D1::RectF(870.0f, 512.0f, 1214.0f, 590.0f), L"GUIDE 4", L"SPACE 캐논은 100%일 때 전선을 한 번에 정리해.", D2D1::ColorF(0xF6FF83, alpha));
        DrawLine({940.0f, 590.0f}, {736.0f, 708.0f}, D2D1::ColorF(0xF6FF83, 0.38f * tipAlpha), 2.4f);
    }
}

void PawlineGameImpl::DrawShowcaseBadge()
{
    if (!m_showcaseMode || m_screen != GameScreen::Playing)
    {
        return;
    }

    const D2D1_RECT_F badge = D2D1::RectF(1010.0f, 112.0f, 1238.0f, 148.0f);
    DrawCartoonPanel(badge, D2D1::ColorF(0x0F1A22, 0.92f), D2D1::ColorF(0xF6FF83), true);
    DrawPixelTextCentered(L"DEMO MODE", badge, 2.4f, D2D1::ColorF(0xF6FF83), 1.0f);
    const D2D1_RECT_F panel = D2D1::RectF(1010.0f, 154.0f, 1238.0f, 222.0f);
    DrawCartoonPanel(panel, D2D1::ColorF(0x071017, 0.86f), CurrentStage().lineColor);
    DrawOutlinedString(DemoStepText(), D2D1::RectF(panel.left + 12.0f, panel.top + 8.0f, panel.right - 12.0f, panel.top + 34.0f), m_smallFormat, D2D1::ColorF(0xEAF7FF), 0.68f);
    DrawString(CurrentStage().name + L"  " + ToWideTime(m_stageTime), D2D1::RectF(panel.left + 12.0f, panel.top + 38.0f, panel.right - 12.0f, panel.bottom - 8.0f), m_smallFormat, D2D1::ColorF(0xF6FF83));
    const D2D1_RECT_F bar = D2D1::RectF(panel.left + 14.0f, panel.bottom - 10.0f, panel.right - 14.0f, panel.bottom - 5.0f);
    FillRoundRect(bar, 3.0f, D2D1::ColorF(0x061019, 0.90f));
    FillRoundRect(D2D1::RectF(bar.left, bar.top, bar.left + (bar.right - bar.left) * Clamp01(m_showcaseTimer / 90.0f), bar.bottom), 3.0f, D2D1::ColorF(0xF6FF83));
}

std::wstring PawlineGameImpl::DemoStepText() const
{
    if (m_showcaseTimer < 5.0f)
    {
        return L"1/5 유닛 소환";
    }
    if (m_showcaseTimer < 12.0f)
    {
        return L"2/5 월렛 성장";
    }
    if (m_showcaseTimer < 22.0f)
    {
        return L"3/5 카메라 이동";
    }
    if (FindBossUnit())
    {
        return L"4/5 보스 패턴";
    }
    if (m_cannonCharge > 70.0f)
    {
        return L"4/5 캐논 준비";
    }
    return L"5/5 전선 압박";
}

void PawlineGameImpl::DrawDebugBadge()
{
    if (!m_debugMode)
    {
        return;
    }

    const D2D1_RECT_F badge = D2D1::RectF(1010.0f, m_showcaseMode && m_screen == GameScreen::Playing ? 230.0f : 112.0f, 1238.0f, m_showcaseMode && m_screen == GameScreen::Playing ? 266.0f : 148.0f);
    DrawCartoonPanel(badge, D2D1::ColorF(0x1E1520, 0.92f), D2D1::ColorF(0xFF9BA8), true);
    DrawPixelTextCentered(L"DEBUG F2", badge, 2.3f, D2D1::ColorF(0xFFB6C2), 1.0f);
}

void PawlineGameImpl::DrawBattleLogo()
{
    const StageDefinition stage = CurrentStage();
    const D2D1_RECT_F plate = D2D1::RectF(42.0f, 22.0f, 446.0f, 86.0f);
    DrawCartoonPanel(plate, D2D1::ColorF(0x071421, 0.94f), stage.lineColor);

    const Vec2 paw = {78.0f, 54.0f};
    FillEllipse({paw.x, paw.y + 8.0f}, 21.0f, 17.0f, D2D1::ColorF(0x061019, 0.92f));
    FillEllipse({paw.x, paw.y + 8.0f}, 16.0f, 12.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.96f));
    FillEllipse({paw.x - 18.0f, paw.y - 10.0f}, 7.0f, 8.0f, D2D1::ColorF(0x061019, 0.92f));
    FillEllipse({paw.x, paw.y - 15.0f}, 7.0f, 8.0f, D2D1::ColorF(0x061019, 0.92f));
    FillEllipse({paw.x + 18.0f, paw.y - 10.0f}, 7.0f, 8.0f, D2D1::ColorF(0x061019, 0.92f));
    FillEllipse({paw.x - 18.0f, paw.y - 10.0f}, 4.8f, 5.8f, D2D1::ColorF(0xF3FBFF));
    FillEllipse({paw.x, paw.y - 15.0f}, 4.8f, 5.8f, D2D1::ColorF(0xF3FBFF));
    FillEllipse({paw.x + 18.0f, paw.y - 10.0f}, 4.8f, 5.8f, D2D1::ColorF(0xF3FBFF));

    auto glyph = [](wchar_t c) {
        switch (c)
        {
        case L'A':
            return std::array<std::wstring, 7>{L"01110", L"10001", L"10001", L"11111", L"10001", L"10001", L"10001"};
        case L'D':
            return std::array<std::wstring, 7>{L"11110", L"10001", L"10001", L"10001", L"10001", L"10001", L"11110"};
        case L'E':
            return std::array<std::wstring, 7>{L"11111", L"10000", L"10000", L"11110", L"10000", L"10000", L"11111"};
        case L'F':
            return std::array<std::wstring, 7>{L"11111", L"10000", L"10000", L"11110", L"10000", L"10000", L"10000"};
        case L'I':
            return std::array<std::wstring, 7>{L"11111", L"00100", L"00100", L"00100", L"00100", L"00100", L"11111"};
        case L'L':
            return std::array<std::wstring, 7>{L"10000", L"10000", L"10000", L"10000", L"10000", L"10000", L"11111"};
        case L'N':
            return std::array<std::wstring, 7>{L"10001", L"11001", L"10101", L"10011", L"10001", L"10001", L"10001"};
        case L'P':
            return std::array<std::wstring, 7>{L"11110", L"10001", L"10001", L"11110", L"10000", L"10000", L"10000"};
        case L'S':
            return std::array<std::wstring, 7>{L"01111", L"10000", L"10000", L"01110", L"00001", L"00001", L"11110"};
        case L'W':
            return std::array<std::wstring, 7>{L"10001", L"10001", L"10001", L"10101", L"10101", L"10101", L"01010"};
        default:
            return std::array<std::wstring, 7>{L"00000", L"00000", L"00000", L"00000", L"00000", L"00000", L"00000"};
        }
    };

    auto drawWord = [&](const std::wstring& word, Vec2 origin, float cell, D2D1_COLOR_F color) {
        float cursor = origin.x;
        for (wchar_t c : word)
        {
            const auto rows = glyph(c);
            for (int r = 0; r < 7; ++r)
            {
                for (int col = 0; col < 5; ++col)
                {
                    if (rows[r][col] != L'1')
                    {
                        continue;
                    }
                    const float x = cursor + static_cast<float>(col) * cell;
                    const float y = origin.y + static_cast<float>(r) * cell;
                    FillRoundRect(D2D1::RectF(x + 1.4f, y + 1.6f, x + cell - 0.4f, y + cell - 0.2f), cell * 0.22f, D2D1::ColorF(0x000000, 0.48f));
                    FillRoundRect(D2D1::RectF(x, y, x + cell - 1.0f, y + cell - 1.0f), cell * 0.22f, color);
                }
            }
            cursor += cell * 6.0f;
        }
    };

    drawWord(L"PAWLINE", {120.0f, 34.0f}, 4.25f, D2D1::ColorF(0xF3FBFF));
    drawWord(L"DEFENSE", {122.0f, 66.0f}, 2.45f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.98f));
    DrawLine({348.0f, 67.0f}, {422.0f, 67.0f}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.60f), 2.4f);
}

void PawlineGameImpl::DrawHeader()
{
    const StageDefinition stage = CurrentStage();
    DrawBattleLogo();
    const D2D1_RECT_F stagePill = D2D1::RectF(458.0f, 42.0f, 568.0f, 76.0f);
    DrawCartoonPanel(stagePill, D2D1::ColorF(0x0A1720, 0.88f), stage.lineColor);
    DrawPixelTextCentered(L"STAGE " + ToWideInt(m_selectedStage + 1), stagePill, 3.0f, D2D1::ColorF(0xDDF7FF), 1.0f);

    DrawTopStat(584.0f, L"ENERGY", ToWideInt(static_cast<int>(m_energy)) + L" / " + ToWideInt(static_cast<int>(MaxEnergy())), D2D1::ColorF(0xB8FF89));
    DrawTopStat(802.0f, L"WALLET", L"Lv." + ToWideInt(m_walletLevel) + L" +" + ToWideInt(static_cast<int>(std::round((WalletUnitBoost() - 1.0f) * 100.0f))) + L"%", D2D1::ColorF(0xF6FF83));
    DrawTopStat(944.0f, L"TIME", ToWideTime(m_stageTime), D2D1::ColorF(0xC7D8FF));
    DrawTopStat(1080.0f, L"SCORE", ToWideInt(m_score), D2D1::ColorF(0xF3FBFF));
}

void PawlineGameImpl::DrawCameraHud()
{
    if (m_screen != GameScreen::Playing && m_screen != GameScreen::Result)
    {
        return;
    }

    const D2D1_RECT_F rail = D2D1::RectF(40.0f, 98.0f, 1240.0f, 108.0f);
    FillRoundRect(rail, 4.0f, D2D1::ColorF(0x071017, 0.76f));
    StrokeRoundRect(rail, 4.0f, D2D1::ColorF(0x38576A, 0.68f), 1.0f);

    const float railWidth = rail.right - rail.left;
    const float viewLeft = rail.left + (m_cameraX / kWorldWidth) * railWidth;
    const float viewRight = rail.left + ((m_cameraX + kWidth) / kWorldWidth) * railWidth;
    FillRoundRect(D2D1::RectF(viewLeft, rail.top + 1.0f, viewRight, rail.bottom - 1.0f), 4.0f, D2D1::ColorF(0x65B8FF, 0.42f));

    const float playerBase = rail.left + (kPlayerBaseX / kWorldWidth) * railWidth;
    const float enemyBase = rail.left + (kEnemyBaseX / kWorldWidth) * railWidth;
    FillEllipse({playerBase, (rail.top + rail.bottom) * 0.5f}, 4.5f, 4.5f, D2D1::ColorF(0x65B8FF));
    FillEllipse({enemyBase, (rail.top + rail.bottom) * 0.5f}, 4.5f, 4.5f, D2D1::ColorF(0xFF9BA8));

    for (const Unit& unit : m_units)
    {
        if (!unit.alive)
        {
            continue;
        }
        const float x = rail.left + (unit.pos.x / kWorldWidth) * railWidth;
        FillEllipse({x, (rail.top + rail.bottom) * 0.5f}, 2.4f, 2.4f,
                    unit.team == Team::Player ? D2D1::ColorF(0xB8FF89, 0.88f) : D2D1::ColorF(0xFF9BA8, 0.82f));
    }

    DrawLine({50.0f, 88.0f}, {40.0f, 103.0f}, D2D1::ColorF(0xCFE8F5, 0.40f), 2.0f);
    DrawLine({40.0f, 103.0f}, {50.0f, 118.0f}, D2D1::ColorF(0xCFE8F5, 0.40f), 2.0f);
    DrawLine({1230.0f, 88.0f}, {1240.0f, 103.0f}, D2D1::ColorF(0xCFE8F5, 0.40f), 2.0f);
    DrawLine({1240.0f, 103.0f}, {1230.0f, 118.0f}, D2D1::ColorF(0xCFE8F5, 0.40f), 2.0f);
}

void PawlineGameImpl::DrawTopStat(float x, const std::wstring& label, const std::wstring& value, D2D1_COLOR_F color)
{
    D2D1_RECT_F rect = D2D1::RectF(x, 22.0f, x + 126.0f, 72.0f);
    DrawCartoonPanel(rect, D2D1::ColorF(0x06131C, 0.99f), D2D1::ColorF(color.r, color.g, color.b, 0.96f), Contains(rect, m_mouse));
    bool labelPixelReady = true;
    for (wchar_t c : label)
    {
        if (!PixelHasInk(c))
        {
            labelPixelReady = false;
            break;
        }
    }
    if (labelPixelReady)
    {
        DrawPixelText(label, {rect.left + 10.0f, rect.top + 8.0f}, 2.0f, D2D1::ColorF(0xD7EAF4), 1.0f);
    }
    else
    {
        DrawOutlinedString(label, D2D1::RectF(rect.left + 10.0f, rect.top + 4.0f, rect.right - 10.0f, rect.top + 25.0f),
                           m_smallFormat, D2D1::ColorF(0xD7EAF4), 0.70f);
    }

    // 에너지처럼 숫자가 길어지는 HUD 값은 카드 안쪽 폭에 맞춰 픽셀 폰트 크기를 줄인다.
    const float innerWidth = rect.right - rect.left - 20.0f;
    const float estimatedWidth = std::max(1.0f, static_cast<float>(value.size()) * 6.0f);
    const float valueCell = std::max(1.65f, std::min(2.55f, innerWidth / estimatedWidth));
    DrawPixelTextCentered(value, D2D1::RectF(rect.left + 8.0f, rect.top + 25.0f, rect.right - 8.0f, rect.bottom - 4.0f), valueCell, color, 1.0f);
}

void PawlineGameImpl::DrawCommandBar()
{
    const StageDefinition stage = CurrentStage();
    FillRect(D2D1::RectF(0.0f, 612.0f, kWidth, kHeight), D2D1::ColorF(0x031017, 0.99f));
    FillRect(D2D1::RectF(0.0f, 612.0f, kWidth, 636.0f), D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.18f));
    StrokeRect(D2D1::RectF(0.0f, 614.0f, kWidth, kHeight), D2D1::ColorF(0x061019), 4.0f);
    StrokeRect(D2D1::RectF(0.0f, 614.0f, kWidth, kHeight), D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.56f), 1.6f);

    for (int i = 0; i < kLoadoutSize; ++i)
    {
        DrawUnitCard(i);
    }

    DrawWalletButton();
    DrawCannonButton();

    DrawCombatHelpPanel();

    DrawButton(PauseButtonRect(), m_paused ? L"RESUME" : L"PAUSE", true, D2D1::ColorF(0x22323F));
    DrawButton(RestartButtonRect(), L"RESTART", true, D2D1::ColorF(0x332337));
    DrawButton(SpeedDownButtonRect(), L"-", true, D2D1::ColorF(0x202833));
    DrawPixelTextCentered(L"SPEED", D2D1::RectF(1064.0f, 718.0f, 1186.0f, 738.0f), 1.65f, D2D1::ColorF(0x9AB2BF), 0.92f);
    DrawPixelTextCentered(L"X" + ToWideFloat(m_gameSpeed), D2D1::RectF(1064.0f, 737.0f, 1186.0f, 764.0f), 2.55f, D2D1::ColorF(0xF3FBFF), 1.0f);
    DrawButton(SpeedUpButtonRect(), L"+", true, D2D1::ColorF(0x202833));
}

void PawlineGameImpl::DrawFullscreenFrameExtensions()
{
    if (!m_renderTarget)
    {
        return;
    }

    const D2D1_SIZE_F size = m_renderTarget->GetSize();
    const float left = std::max(0.0f, m_viewOffsetX);
    const float top = std::max(0.0f, m_viewOffsetY);
    const float right = std::min(size.width, m_viewOffsetX + kWidth * m_viewScale);
    const float bottom = std::min(size.height, m_viewOffsetY + kHeight * m_viewScale);
    if (left <= 0.5f && top <= 0.5f && right >= size.width - 0.5f && bottom >= size.height - 0.5f)
    {
        return;
    }

    const StageDefinition stage = CurrentStage();
    m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    auto fillSideBands = [&](float bandLeft, float bandRight) {
        if (bandRight <= bandLeft)
        {
            return;
        }
        FillRect(D2D1::RectF(bandLeft, 0.0f, bandRight, size.height), D2D1::ColorF(0x031017, 0.96f));
        FillRect(D2D1::RectF(bandLeft, 0.0f, bandRight, top + 112.0f * m_viewScale), D2D1::ColorF(0x061019, 0.98f));
        FillRect(D2D1::RectF(bandLeft, top + 612.0f * m_viewScale, bandRight, size.height), D2D1::ColorF(0x031017, 0.99f));
        FillRect(D2D1::RectF(bandLeft, top + 612.0f * m_viewScale, bandRight, top + 636.0f * m_viewScale), D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.12f));
    };

    fillSideBands(0.0f, left);
    fillSideBands(right, size.width);

    if (top > 0.5f)
    {
        FillRect(D2D1::RectF(0.0f, 0.0f, size.width, top), D2D1::ColorF(0x031017, 0.98f));
    }
    if (bottom < size.height - 0.5f)
    {
        FillRect(D2D1::RectF(0.0f, bottom, size.width, size.height), D2D1::ColorF(0x031017, 0.98f));
    }
    if (left > 0.5f)
    {
        DrawLine({left, 0.0f}, {left, size.height}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.28f), 1.0f);
    }
    if (right < size.width - 0.5f)
    {
        DrawLine({right, 0.0f}, {right, size.height}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.28f), 1.0f);
    }

    SetViewTransform();
}

void PawlineGameImpl::DrawCombatHelpPanel()
{
    const StageDefinition stage = CurrentStage();
    const D2D1_RECT_F panel = D2D1::RectF(824.0f, 628.0f, 992.0f, 774.0f);
    DrawCartoonPanel(panel, D2D1::ColorF(0x07131C, 0.99f), D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.96f));
    DrawPixelTextCentered(L"COMMAND", D2D1::RectF(panel.left + 12.0f, panel.top + 10.0f, panel.right - 12.0f, panel.top + 36.0f), 2.8f, D2D1::ColorF(0xF3FBFF), 1.0f);

    const std::array<std::wstring, 5> lines = {
        L"1-5  UNIT CARD",
        L"W    WALLET",
        L"SPACE  BEAM",
        L"ESC  MENU",
        L"DRAG / WHEEL"
    };
    for (int i = 0; i < static_cast<int>(lines.size()); ++i)
    {
        const float y = panel.top + 43.0f + static_cast<float>(i) * 20.0f;
        bool linePixelReady = true;
        for (wchar_t c : lines[i])
        {
            if (!PixelHasInk(c))
            {
                linePixelReady = false;
                break;
            }
        }
        const D2D1_COLOR_F lineColor = i == 4 ? D2D1::ColorF(0xD8E8F0) : D2D1::ColorF(0xF6FF83);
        if (linePixelReady)
        {
            DrawPixelText(lines[i], {panel.left + 14.0f, y + 3.0f}, 1.95f, lineColor, 0.96f);
        }
        else
        {
            DrawOutlinedString(lines[i], D2D1::RectF(panel.left + 14.0f, y - 1.0f, panel.right - 12.0f, y + 20.0f),
                               m_smallFormat, lineColor, 0.66f);
        }
    }
}

void PawlineGameImpl::DrawUnitCard(int index)
{
    const PlayerUnit type = m_loadout[index];
    const UnitStats stats = PlayerStats(type);
    const int cost = UnitEnergyCost(type);
    const float cooldown = UnitCooldown(type);
    const D2D1_RECT_F rect = CardRect(index);
    const bool affordable = m_energy >= static_cast<float>(cost);
    const bool ready = m_cardCooldowns[index] <= 0.0f;
    const bool hover = Contains(rect, m_mouse);
    const D2D1_COLOR_F border = ready && affordable ? stats.accent : D2D1::ColorF(0x394955);

    DrawCartoonPanel(rect, hover ? D2D1::ColorF(0x123044, 0.99f) : D2D1::ColorF(0x06131C, 0.99f), D2D1::ColorF(border.r, border.g, border.b, 0.96f), hover);
    if (ready && affordable)
    {
        FillRoundRect(D2D1::RectF(rect.left + 7.0f, rect.top + 7.0f, rect.left + 35.0f, rect.top + 24.0f), 5.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.34f));
    }

    DrawPlayerIcon(type, {rect.left + 59.0f, rect.top + 37.0f}, 0.82f, affordable);
    const D2D1_COLOR_F roleColor = UnitRoleColor(type, stats);
    FillRoundRect(D2D1::RectF(rect.right - 61.0f, rect.top + 7.0f, rect.right - 8.0f, rect.top + 29.0f),
                  5.0f,
                  D2D1::ColorF(roleColor.r, roleColor.g, roleColor.b, affordable ? 0.24f : 0.12f));
    DrawPixelTextCentered(UnitRoleLabel(type, stats),
                          D2D1::RectF(rect.right - 60.0f, rect.top + 7.0f, rect.right - 8.0f, rect.top + 29.0f),
                          0.76f,
                          affordable ? roleColor : D2D1::ColorF(0x7E919C),
                          1.0f);

    DrawPixelTextCentered(stats.name, D2D1::RectF(rect.left + 6.0f, rect.top + 73.0f, rect.right - 6.0f, rect.top + 95.0f), 1.75f, affordable ? D2D1::ColorF(0xFFFFFF) : D2D1::ColorF(0xB5C1C8), 1.0f);
    DrawPixelTextCentered(L"LV." + ToWideInt(UnitLevel(type)) + L" COST " + ToWideInt(cost), D2D1::RectF(rect.left + 6.0f, rect.top + 101.0f, rect.right - 6.0f, rect.top + 122.0f), 1.55f, affordable ? D2D1::ColorF(0xC6FF9B) : D2D1::ColorF(0xFFB6C2), 1.0f);
    FillRoundRect(D2D1::RectF(rect.left + 15.0f, rect.bottom - 28.0f, rect.right - 15.0f, rect.bottom - 8.0f), 6.0f, D2D1::ColorF(0x061019, 0.58f));
    DrawPixelTextCentered(L"KEY " + ToWideInt(index + 1), D2D1::RectF(rect.left + 12.0f, rect.bottom - 27.0f, rect.right - 12.0f, rect.bottom - 7.0f), 1.65f, D2D1::ColorF(0xE5F6FF), 1.0f);

    if (!ready)
    {
        const float pct = Clamp01(m_cardCooldowns[index] / cooldown);
        FillRoundRect(D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), 8.0f, D2D1::ColorF(0x000000, 0.46f));
        FillRoundRect(D2D1::RectF(rect.left, rect.bottom - (rect.bottom - rect.top) * pct, rect.right, rect.bottom), 8.0f, D2D1::ColorF(0xFFFFFF, 0.11f));
        DrawPixelTextCentered(ToWideInt(static_cast<int>(std::ceil(m_cardCooldowns[index]))), rect, 5.0f, D2D1::ColorF(0xF3FBFF), 1.0f);
    }
}

void PawlineGameImpl::DrawWalletButton()
{
    const int cost = WalletUpgradeCost();
    const bool enabled = cost > 0 && m_energy >= static_cast<float>(cost);
    const bool maxed = cost <= 0;
    const D2D1_RECT_F rect = WalletButtonRect();
    const bool hover = Contains(rect, m_mouse);
    DrawCartoonPanel(rect, hover ? D2D1::ColorF(0x182E28, 0.98f) : D2D1::ColorF(0x10241E, 0.98f),
                     enabled || maxed ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0x4E6253), hover && cost > 0);
    DrawPixelTextCentered(maxed ? L"WALLET MAX" : L"WALLET +",
                          D2D1::RectF(rect.left + 8.0f, rect.top + 7.0f, rect.right - 8.0f, rect.top + 28.0f),
                          maxed ? 1.48f : 1.85f,
                          maxed ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0xEAF7FF),
                          1.0f);
    DrawPixelTextCentered(L"LV." + ToWideInt(m_walletLevel) + L"  +" + ToWideInt(static_cast<int>(std::round((WalletUnitBoost() - 1.0f) * 100.0f))) + L"%",
                          D2D1::RectF(rect.left + 9.0f, rect.top + 29.0f, rect.right - 9.0f, rect.top + 48.0f),
                          1.34f,
                          D2D1::ColorF(0xF6FF83),
                          1.0f);

    const std::wstring pulseText = L"PULSE " + ToWideInt(static_cast<int>(std::ceil(std::max(0.0f, m_walletPulseTimer)))) + L"S";
    const D2D1_RECT_F bottom = D2D1::RectF(rect.left + 10.0f, rect.bottom - 24.0f, rect.right - 10.0f, rect.bottom - 6.0f);
    FillRoundRect(bottom, 6.0f, D2D1::ColorF(0x061019, 0.56f));
    const float pulsePct = 1.0f - Clamp01(m_walletPulseTimer / WalletPulseInterval());
    const D2D1_RECT_F strip = D2D1::RectF(bottom.left + 6.0f, bottom.bottom - 5.0f, bottom.right - 6.0f, bottom.bottom - 2.0f);
    FillRoundRect(strip, 2.0f, D2D1::ColorF(0x071017, 0.92f));
    FillRoundRect(D2D1::RectF(strip.left, strip.top, strip.left + (strip.right - strip.left) * pulsePct, strip.bottom), 2.0f, D2D1::ColorF(0xB8FF89, 0.82f));
    if (maxed)
    {
        DrawPixelTextCentered(L"MAX", D2D1::RectF(bottom.left + 4.0f, bottom.top + 1.0f, bottom.left + 46.0f, bottom.bottom - 6.0f),
                              0.96f, D2D1::ColorF(0xB8FF89), 1.0f);
        DrawPixelTextCentered(pulseText, D2D1::RectF(bottom.left + 48.0f, bottom.top + 1.0f, bottom.right - 4.0f, bottom.bottom - 6.0f),
                              0.84f, D2D1::ColorF(0xDFFFD1), 1.0f);
        return;
    }

    DrawPixelTextCentered(L"COST " + ToWideInt(cost),
                          D2D1::RectF(bottom.left + 4.0f, bottom.top + 1.0f, bottom.left + 64.0f, bottom.bottom - 6.0f),
                          0.82f,
                          enabled ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xB5C1C8),
                          1.0f);
    DrawPixelTextCentered(pulseText,
                          D2D1::RectF(bottom.left + 66.0f, bottom.top + 1.0f, bottom.right - 4.0f, bottom.bottom - 6.0f),
                          0.74f,
                          D2D1::ColorF(0xF3FBFF),
                          1.0f);
}

void PawlineGameImpl::DrawCannonButton()
{
    const bool ready = m_cannonCharge >= 100.0f;
    DrawButton(CannonButtonRect(), ready ? L"MOONBEAM" : L"CHARGING", true, ready ? D2D1::ColorF(0x4B4321) : D2D1::ColorF(0x202833));
    const D2D1_RECT_F bar = D2D1::RectF(CannonButtonRect().left + 14.0f, CannonButtonRect().bottom - 22.0f, CannonButtonRect().right - 14.0f, CannonButtonRect().bottom - 12.0f);
    FillRoundRect(bar, 4.0f, D2D1::ColorF(0x071017));
    FillRoundRect(D2D1::RectF(bar.left, bar.top, bar.left + (bar.right - bar.left) * Clamp01(m_cannonCharge / 100.0f), bar.bottom), 4.0f, D2D1::ColorF(0xF6FF83));
}

void PawlineGameImpl::DrawButton(D2D1_RECT_F rect, const std::wstring& label, bool enabled, D2D1_COLOR_F fill)
{
    const bool hover = Contains(rect, m_mouse);
    D2D1_COLOR_F actualFill = enabled ? fill : D2D1::ColorF(0x171D23);
    if (hover && enabled)
    {
        actualFill.r = std::min(1.0f, actualFill.r + 0.04f);
        actualFill.g = std::min(1.0f, actualFill.g + 0.04f);
        actualFill.b = std::min(1.0f, actualFill.b + 0.04f);
    }
    if (hover && enabled)
    {
        FillRoundRect(InflateRectF(rect, 6.0f, 6.0f), 10.0f, D2D1::ColorF(0x65B8FF, 0.075f));
    }
    DrawCartoonPanel(rect, actualFill, enabled ? (hover ? D2D1::ColorF(0x9EDFFF) : D2D1::ColorF(0x476779)) : D2D1::ColorF(0x2B333A), hover && enabled);
    DrawPixelTextCentered(label, rect, 2.55f, enabled ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x7E8B94), enabled ? 1.0f : 0.72f);
}

void PawlineGameImpl::DrawMessage()
{
    if (m_autoSaveNoticeTimer > 0.0f && !m_autoSaveNotice.empty())
    {
        const float saveAlpha = Clamp01(m_autoSaveNoticeTimer / 0.42f);
        const D2D1_RECT_F saveRect = D2D1::RectF(944.0f, 102.0f, 1220.0f, 134.0f);
        DrawCartoonPanel(saveRect, D2D1::ColorF(0x071017, 0.78f * saveAlpha), D2D1::ColorF(0xB8FF89, saveAlpha));
        DrawPixelTextCentered(m_autoSaveNotice, D2D1::RectF(saveRect.left + 10.0f, saveRect.top + 7.0f, saveRect.right - 10.0f, saveRect.bottom - 5.0f), 1.8f, D2D1::ColorF(0xDFFFCE, saveAlpha), saveAlpha);
    }

    if (m_messageTimer <= 0.0f || m_message.empty())
    {
        return;
    }

    float alpha = Clamp01(m_messageTimer / 0.45f);
    const D2D1_RECT_F rect = MessageToastRect();
    if (Contains(rect, m_mouse))
    {
        alpha *= 0.42f;
    }
    DrawCartoonPanel(rect, D2D1::ColorF(0x071017, 0.82f * alpha), D2D1::ColorF(0x65B8FF, alpha));
    const float innerWidth = rect.right - rect.left - 28.0f;
    const float safeCell = std::max(1.35f, std::min(2.0f, innerWidth / std::max(1.0f, PixelTextWidth(m_message, 1.0f))));
    DrawPixelTextCentered(m_message, D2D1::RectF(rect.left + 12.0f, rect.top + 7.0f, rect.right - 12.0f, rect.bottom - 5.0f), safeCell, D2D1::ColorF(0xEAF7FF, alpha), alpha);
}

void PawlineGameImpl::DrawOverlay()
{
    if (!m_paused || m_screen != GameScreen::Playing)
    {
        return;
    }

    FillViewport(D2D1::ColorF(0x000000, 0.46f));
    std::wstring title = L"PAUSED";
    std::wstring body = L"Press P to resume.";
    D2D1_COLOR_F color = D2D1::ColorF(0xEAF7FF);

    DrawPixelTextCentered(title, D2D1::RectF(320.0f, 296.0f, 960.0f, 348.0f), 5.0f, color, 1.0f);
    DrawPixelTextCentered(body, D2D1::RectF(320.0f, 352.0f, 960.0f, 386.0f), 2.4f, D2D1::ColorF(0xF3FBFF), 1.0f);
}

void PawlineGameImpl::DrawSceneTransition()
{
    if (m_sceneTransitionTimer <= 0.0f || m_sceneTransitionMax <= 0.0f)
    {
        return;
    }

    const float t = Clamp01(m_sceneTransitionTimer / m_sceneTransitionMax);
    const float cover = Smooth01(t);
    const float side = cover * kWidth * 0.52f;
    const float topBottom = cover * kHeight * 0.52f;
    const D2D1_COLOR_F curtain = D2D1::ColorF(0x000000, 0.92f);
    const D2D1_COLOR_F edge = D2D1::ColorF(0x071017, 0.98f);

    FillRect(D2D1::RectF(0.0f, 0.0f, side, kHeight), curtain);
    FillRect(D2D1::RectF(kWidth - side, 0.0f, kWidth, kHeight), curtain);
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, topBottom), D2D1::ColorF(0x000000, 0.72f * cover));
    FillRect(D2D1::RectF(0.0f, kHeight - topBottom, kWidth, kHeight), D2D1::ColorF(0x000000, 0.72f * cover));

    if (side > 2.0f)
    {
        FillRect(D2D1::RectF(side - 6.0f, 0.0f, side + 2.0f, kHeight), edge);
        FillRect(D2D1::RectF(kWidth - side - 2.0f, 0.0f, kWidth - side + 6.0f, kHeight), edge);
    }
    if (cover > 0.72f)
    {
        FillViewport(D2D1::ColorF(0x000000, (cover - 0.72f) * 1.45f));
    }
}

void PawlineGameImpl::DrawEscapeMenuClean()
{
    FillViewport(D2D1::ColorF(0x000000, 0.68f));
    const D2D1_RECT_F panel = D2D1::RectF(286.0f, 104.0f, 994.0f, 734.0f);
    FillRoundRect(panel, 10.0f, D2D1::ColorF(0x0A121A, 0.97f));
    StrokeRoundRect(panel, 10.0f, D2D1::ColorF(0x65B8FF), 1.8f);

    DrawString(L"메뉴", D2D1::RectF(panel.left + 34.0f, panel.top + 28.0f, panel.right - 34.0f, panel.top + 80.0f), m_titleFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"언제든 계속하거나 빠져나갈 수 있어.", D2D1::RectF(panel.left + 34.0f, panel.top + 82.0f, panel.right - 34.0f, panel.top + 110.0f), m_centerFormat, D2D1::ColorF(0xBFD1DB));

    const auto drawEscapeSlider = [&](const std::wstring& label, D2D1_RECT_F bar, float normalized, D2D1_COLOR_F accent) {
        const float value = Clamp01(normalized);
        const bool hover = Contains(bar, m_mouse);
        DrawString(label, D2D1::RectF(bar.left, bar.top - 34.0f, bar.right - 92.0f, bar.top - 8.0f), m_smallFormat, D2D1::ColorF(0xEAF7FF));
        DrawString(ToWideInt(static_cast<int>(std::round(value * 100.0f))) + L"%", D2D1::RectF(bar.right - 76.0f, bar.top - 34.0f, bar.right + 10.0f, bar.top - 8.0f), m_smallFormat, D2D1::ColorF(0xF6FF83));
        FillRoundRect(bar, 9.0f, D2D1::ColorF(0x02080D, 0.92f));
        const float knobX = Lerp(bar.left, bar.right, value);
        FillRoundRect(D2D1::RectF(bar.left, bar.top, knobX, bar.bottom), 9.0f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.72f));
        StrokeRoundRect(bar, 9.0f, D2D1::ColorF(accent.r, accent.g, accent.b, hover ? 0.92f : 0.54f), hover ? 2.0f : 1.2f);
        FillEllipse({knobX, (bar.top + bar.bottom) * 0.5f}, hover ? 11.0f : 9.0f, hover ? 11.0f : 9.0f, D2D1::ColorF(0xF3FBFF, 0.96f));
        StrokeEllipse({knobX, (bar.top + bar.bottom) * 0.5f}, hover ? 11.0f : 9.0f, hover ? 11.0f : 9.0f, accent, 2.0f);
    };

    DrawPixelText(L"COMBAT", {332.0f, 154.0f}, 2.1f, D2D1::ColorF(0x65B8FF));
    DrawPixelText(L"AUDIO", {682.0f, 154.0f}, 2.1f, D2D1::ColorF(0xF6FF83));

    DrawButton(EscapeResumeButtonRect(), L"RESUME", true, D2D1::ColorF(0x173C4B));
    DrawButton(EscapeShakeButtonRect(), m_hitShakeEnabled ? L"SHAKE ON" : L"SHAKE OFF", true, m_hitShakeEnabled ? D2D1::ColorF(0x173C4B) : D2D1::ColorF(0x302735));

    const float speed = m_screen == GameScreen::Playing ? m_gameSpeed : m_defaultGameSpeed;
    DrawString(L"게임 속도", D2D1::RectF(332.0f, 332.0f, 608.0f, 358.0f), m_centerFormat, D2D1::ColorF(0xEAF7FF));
    DrawButton(EscapeSpeedDownButtonRect(), L"-", true, D2D1::ColorF(0x202833));
    DrawString(L"x" + ToWideFloat(speed), D2D1::RectF(392.0f, 382.0f, 548.0f, 412.0f), m_centerFormat, D2D1::ColorF(0xF6FF83));
    DrawButton(EscapeSpeedUpButtonRect(), L"+", true, D2D1::ColorF(0x202833));

    DrawButton(EscapeSaveButtonRect(), L"SAVE", true, D2D1::ColorF(0x283B27));
    DrawButton(EscapeLoadButtonRect(), L"LOAD", true, D2D1::ColorF(0x22323F));
    DrawButton(EscapeStoryButtonRect(), L"STORY", true, D2D1::ColorF(0x2D3722));
    DrawButton(EscapeStageButtonRect(), L"STAGE SELECT", true, D2D1::ColorF(0x283B27));
    DrawButton(EscapeQuitButtonRect(), L"QUIT GAME", true, D2D1::ColorF(0x332337));

    drawEscapeSlider(L"효과음 볼륨", EscapeSfxSliderRect(), m_sfxVolume, D2D1::ColorF(0x65B8FF));
    drawEscapeSlider(L"UI 볼륨", EscapeUiSliderRect(), m_uiVolume, D2D1::ColorF(0xC8B7FF));
    drawEscapeSlider(L"브금 볼륨", EscapeBgmSliderRect(), m_bgmVolume, D2D1::ColorF(0xB8FF89));
    DrawButton(EscapeAudioResetButtonRect(), L"AUDIO RESET", true, D2D1::ColorF(0x283B27));
}

void PawlineGameImpl::DrawResultScreen()
{
    FillViewport(D2D1::ColorF(0x000000, 0.62f));
    const D2D1_RECT_F panel = D2D1::RectF(340.0f, 210.0f, 940.0f, 650.0f);
    const bool finalClear = m_victory && m_selectedStage == kStageCount - 1;
    FillRoundRect(panel, 10.0f, D2D1::ColorF(0x0A121A, 0.96f));
    StrokeRoundRect(panel, 10.0f, m_victory ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFF9BA8), 2.0f);
    if (finalClear)
    {
        DrawFinalClearScene(panel);
    }

    const StageDefinition stage = CurrentStage();
    const std::wstring title = finalClear ? L"CAMPAIGN CLEAR" : (m_victory ? L"STAGE CLEAR" : L"BASE BROKEN");
    const std::wstring subtitle = finalClear ? L"수성에서 태양까지, 방어선 완주" : (m_victory ? L"전선을 완전히 밀어냈어." : L"적이 기지를 돌파했어.");
    DrawPixelTextCentered(title, D2D1::RectF(panel.left + 34.0f, panel.top + 30.0f, panel.right - 34.0f, panel.top + 78.0f), 4.2f, m_victory ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2), 1.0f);
    DrawPixelTextCentered(subtitle, D2D1::RectF(panel.left + 34.0f, panel.top + 84.0f, panel.right - 34.0f, panel.top + 112.0f), 2.2f, D2D1::ColorF(0xEAF7FF), 1.0f);

    DrawString(stage.name, D2D1::RectF(panel.left + 64.0f, panel.top + 142.0f, panel.right - 64.0f, panel.top + 170.0f), m_headerFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"TIME  " + ToWideTime(m_resultTime), D2D1::RectF(panel.left + 94.0f, panel.top + 190.0f, panel.left + 280.0f, panel.top + 218.0f), m_bodyFormat, D2D1::ColorF(0xC7D8FF));
    DrawString(L"SCORE  " + ToWideInt(m_resultScore), D2D1::RectF(panel.left + 320.0f, panel.top + 190.0f, panel.right - 94.0f, panel.top + 218.0f), m_bodyFormat, D2D1::ColorF(0xF6FF83));
    DrawString(L"WALLET Lv." + ToWideInt(m_walletLevel), D2D1::RectF(panel.left + 94.0f, panel.top + 224.0f, panel.left + 280.0f, panel.top + 252.0f), m_bodyFormat, D2D1::ColorF(0xB8FF89));
    DrawString(L"DIFFICULTY  " + DifficultyLabel(), D2D1::RectF(panel.left + 320.0f, panel.top + 224.0f, panel.right - 94.0f, panel.top + 252.0f), m_bodyFormat, D2D1::ColorF(0xD9E5F2));
    DrawString(m_victory ? L"LUMEN +" + ToWideInt(m_lastReward) + L"   TOTAL " + ToWideInt(m_lumen) : L"LUMEN +0   TOTAL " + ToWideInt(m_lumen),
               D2D1::RectF(panel.left + 94.0f, panel.top + 268.0f, panel.right - 94.0f, panel.top + 296.0f),
               m_bodyFormat,
               m_victory ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0x8EA9B8));

    const float hpPct = m_playerBaseMaxHp > 0.0f ? Clamp01(m_playerBaseHp / m_playerBaseMaxHp) : 0.0f;
    int medalCount = m_victory ? 1 : 0;
    if (m_victory && hpPct > 0.45f)
    {
        ++medalCount;
    }
    if (m_victory && m_resultTime < CurrentStage().bossFirstTime + 55.0f)
    {
        ++medalCount;
    }
    const std::wstring rank = medalCount >= 3 ? L"RANK S" : (medalCount == 2 ? L"RANK A" : (medalCount == 1 ? L"RANK B" : L"RANK C"));
    DrawPixelTextCentered(rank, D2D1::RectF(panel.left + 94.0f, panel.top + 292.0f, panel.left + 262.0f, panel.top + 330.0f), 3.2f, m_victory ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0xFFB6C2), 1.0f);
    for (int i = 0; i < 3; ++i)
    {
        const Vec2 medal = {panel.left + 330.0f + static_cast<float>(i) * 62.0f, panel.top + 314.0f};
        const bool earned = i < medalCount;
        FillEllipse(medal, 20.0f, 20.0f, earned ? D2D1::ColorF(0xF6FF83, 0.24f) : D2D1::ColorF(0x0F1A22, 0.86f));
        StrokeEllipse(medal, 20.0f, 20.0f, earned ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0x394955), 2.0f);
        DrawPixelTextCentered(earned ? L"OK" : L"--", D2D1::RectF(medal.x - 18.0f, medal.y - 10.0f, medal.x + 18.0f, medal.y + 12.0f), 1.8f, earned ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x65727C), 1.0f);
    }
    const D2D1_RECT_F rewardBadge = D2D1::RectF(panel.left + 522.0f, panel.top + 288.0f, panel.right - 70.0f, panel.top + 338.0f);
    FillRoundRect(rewardBadge, 8.0f, D2D1::ColorF(0x071017, 0.58f));
    StrokeRoundRect(rewardBadge, 8.0f, m_victory ? D2D1::ColorF(0xB8FF89, 0.62f) : D2D1::ColorF(0x8EA9B8, 0.42f), 1.2f);
    DrawPixelTextCentered(m_victory ? L"REWARD" : L"NO REWARD",
                          D2D1::RectF(rewardBadge.left + 8.0f, rewardBadge.top + 7.0f, rewardBadge.right - 8.0f, rewardBadge.top + 27.0f),
                          m_victory ? 1.75f : 1.38f,
                          m_victory ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0x8EA9B8),
                          1.0f);
    DrawPixelTextCentered(m_victory ? L"SAVED" : L"TRY AGAIN",
                          D2D1::RectF(rewardBadge.left + 8.0f, rewardBadge.top + 27.0f, rewardBadge.right - 8.0f, rewardBadge.bottom - 6.0f),
                          m_victory ? 1.55f : 1.25f,
                          D2D1::ColorF(0xF3FBFF),
                          1.0f);
    DrawOutlinedString(finalClear ? L"태양 방어선 완주" : GrowthRecommendation(),
                       D2D1::RectF(panel.left + 64.0f, panel.bottom - 40.0f, panel.right - 64.0f, panel.bottom - 10.0f),
                       m_smallFormat,
                       D2D1::ColorF(0xF6FF83),
                       0.74f);

    DrawButton(ResultRetryButtonRect(), L"RETRY", true, D2D1::ColorF(0x173C4B));
    DrawButton(ResultNextButtonRect(), (m_victory && m_selectedStage < kStageCount - 1) ? L"NEXT" : L"CLOSE", true, D2D1::ColorF(0x283B27));
    DrawButton(ResultMenuButtonRect(), L"MENU", true, D2D1::ColorF(0x332337));
}

void PawlineGameImpl::DrawFinalClearScene(D2D1_RECT_F panel)
{
    // 마지막 스테이지 클리어 전용 연출. 낮은 알파의 태양광과 행성 경로만 깔아 통계 UI를 가리지 않는다.
    const Vec2 sun = {panel.right - 92.0f, panel.top + 92.0f};
    for (int i = 0; i < 18; ++i)
    {
        const float a = static_cast<float>(i) / 18.0f * kPi * 2.0f + m_uiTime * 0.25f;
        DrawLine({sun.x + std::cos(a) * 38.0f, sun.y + std::sin(a) * 38.0f},
                 {sun.x + std::cos(a) * 88.0f, sun.y + std::sin(a) * 88.0f},
                 D2D1::ColorF(0xFFB347, 0.18f), 3.0f);
    }
    FillEllipse(sun, 44.0f, 44.0f, D2D1::ColorF(0xFFB347, 0.22f));
    FillEllipse(sun, 23.0f, 23.0f, D2D1::ColorF(0xFFF0B5, 0.45f));

    const float y = panel.top + 420.0f;
    DrawLine({panel.left + 74.0f, y}, {panel.right - 74.0f, y}, D2D1::ColorF(0xF6FF83, 0.18f), 2.0f);
    for (int i = 0; i < kStageCount; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(kStageCount - 1);
        const Vec2 node = {Lerp(panel.left + 74.0f, panel.right - 74.0f, t), y};
        const StageDefinition stage = GetStageDefinition(i);
        FillEllipse(node, i == kStageCount - 1 ? 8.0f : 5.0f, i == kStageCount - 1 ? 8.0f : 5.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.72f));
    }
}
