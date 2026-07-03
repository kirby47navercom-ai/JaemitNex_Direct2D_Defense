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

    if (m_screen == GameScreen::Codex)
    {
        DrawCodex();
        DrawMessage();
        DrawUiPulses();
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
    DrawUiPulses();
    DrawMessage();
    DrawOverlay();
    DrawScreenFlash();
    if (m_screen == GameScreen::Result)
    {
        DrawResultScreen();
    }
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
    m_renderTarget->FillRectangle(rect, m_brush);
}

void PawlineGameImpl::StrokeRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawRectangle(rect, m_brush, width);
}

void PawlineGameImpl::FillRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color)
{
    SetColor(color);
    m_renderTarget->FillRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), m_brush);
}

void PawlineGameImpl::StrokeRoundRect(D2D1_RECT_F rect, float radius, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), m_brush, width);
}

void PawlineGameImpl::FillEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color)
{
    SetColor(color);
    m_renderTarget->FillEllipse(Ellipse(pos, rx, ry), m_brush);
}

void PawlineGameImpl::StrokeEllipse(Vec2 pos, float rx, float ry, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawEllipse(Ellipse(pos, rx, ry), m_brush, width);
}

void PawlineGameImpl::DrawLine(Vec2 a, Vec2 b, D2D1_COLOR_F color, float width)
{
    SetColor(color);
    m_renderTarget->DrawLine(Point(a), Point(b), m_brush, width, m_roundStroke);
}

void PawlineGameImpl::DrawString(const std::wstring& text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color)
{
    SetColor(color);
    m_renderTarget->DrawTextW(text.c_str(), static_cast<UINT32>(text.size()), format, rect, m_brush);
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

void PawlineGameImpl::DrawCartoonPanel(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F accent, bool hover)
{
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, 0.94f);
    FillRoundRect(OffsetRectF(rect, 4.0f, 5.0f), 8.0f, D2D1::ColorF(0x000000, 0.30f));
    FillRoundRect(rect, 8.0f, fill);
    FillRoundRect(D2D1::RectF(rect.left + 6.0f, rect.top + 6.0f, rect.right - 6.0f, rect.top + 18.0f), 6.0f, D2D1::ColorF(0xFFFFFF, hover ? 0.12f : 0.075f));
    StrokeRoundRect(rect, 8.0f, ink, hover ? 3.6f : 3.0f);
    StrokeRoundRect(InsetRectF(rect, 3.0f, 3.0f), 6.0f, D2D1::ColorF(accent.r, accent.g, accent.b, hover ? 0.72f : 0.45f), hover ? 2.0f : 1.2f);
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

void PawlineGameImpl::DrawTitle()
{
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x071017));
    for (float y = 38.0f; y < kHeight; y += 46.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.24f), 1.0f);
    }
    for (float x = 36.0f; x < kWidth; x += 54.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.16f), 1.0f);
    }

    FillEllipse({640.0f, 264.0f}, 118.0f, 118.0f, D2D1::ColorF(0xFFB347, 0.16f));
    StrokeEllipse({640.0f, 264.0f}, 118.0f, 118.0f, D2D1::ColorF(0xFFB347, 0.48f), 2.0f);
    FillEllipse({640.0f, 264.0f}, 44.0f, 44.0f, D2D1::ColorF(0xF3FBFF));
    StrokeEllipse({640.0f, 264.0f}, 44.0f, 44.0f, D2D1::ColorF(0x65B8FF), 3.0f);
    FillEllipse({626.0f, 255.0f}, 4.0f, 6.0f, D2D1::ColorF(0x071017));
    FillEllipse({654.0f, 255.0f}, 4.0f, 6.0f, D2D1::ColorF(0x071017));
    DrawLine({623.0f, 281.0f}, {657.0f, 281.0f}, D2D1::ColorF(0x071017), 2.0f);

    DrawString(L"PAWLINE DEFENSE", D2D1::RectF(250.0f, 360.0f, 1030.0f, 414.0f), m_titleFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"수성에서 태양까지 이어지는 탑뷰 라인 디펜스", D2D1::RectF(250.0f, 418.0f, 1030.0f, 448.0f), m_centerFormat, D2D1::ColorF(0xBFD1DB));

    DrawButton(TitleStartButtonRect(), L"시작하기", true, D2D1::ColorF(0x173C4B));
    DrawButton(TitleOptionsButtonRect(), L"옵션", true, D2D1::ColorF(0x283B27));
    DrawButton(TitleQuitButtonRect(), L"나가기", true, D2D1::ColorF(0x332337));

    const std::wstring route = L"수성  금성  지구  화성  목성  토성  천왕성  해왕성  명왕성  태양";
    DrawString(route, D2D1::RectF(120.0f, 742.0f, 1160.0f, 772.0f), m_centerFormat, D2D1::ColorF(0xF6FF83));
}

void PawlineGameImpl::DrawOptions()
{
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x071017));
    for (float y = 38.0f; y < kHeight; y += 46.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.24f), 1.0f);
    }
    for (float x = 36.0f; x < kWidth; x += 54.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.16f), 1.0f);
    }

    DrawString(L"옵션", D2D1::RectF(250.0f, 132.0f, 1030.0f, 188.0f), m_titleFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"전투 감각, 기본 속도, 화면 안전 여백을 조정합니다.", D2D1::RectF(250.0f, 192.0f, 1030.0f, 222.0f), m_centerFormat, D2D1::ColorF(0xBFD1DB));

    DrawButton(OptionsShakeButtonRect(), m_hitShakeEnabled ? L"피격 흔들림 켜짐" : L"피격 흔들림 꺼짐", true, m_hitShakeEnabled ? D2D1::ColorF(0x173C4B) : D2D1::ColorF(0x302735));
    DrawString(L"H", D2D1::RectF(OptionsShakeButtonRect().right + 16.0f, OptionsShakeButtonRect().top + 14.0f, OptionsShakeButtonRect().right + 56.0f, OptionsShakeButtonRect().bottom), m_centerFormat, D2D1::ColorF(0x8EA9B8));

    DrawString(L"기본 게임 속도", D2D1::RectF(490.0f, 398.0f, 790.0f, 424.0f), m_centerFormat, D2D1::ColorF(0xEAF7FF));
    DrawButton(OptionsSpeedDownButtonRect(), L"-", true, D2D1::ColorF(0x202833));
    DrawString(L"x" + ToWideFloat(m_defaultGameSpeed), D2D1::RectF(552.0f, 442.0f, 728.0f, 474.0f), m_centerFormat, D2D1::ColorF(0xF6FF83));
    DrawButton(OptionsSpeedUpButtonRect(), L"+", true, D2D1::ColorF(0x202833));

    DrawString(L"화면 안전 여백", D2D1::RectF(490.0f, 508.0f, 790.0f, 534.0f), m_centerFormat, D2D1::ColorF(0xEAF7FF));
    DrawButton(OptionsViewDownButtonRect(), L"-", true, D2D1::ColorF(0x202833));
    DrawString(ToWideInt(static_cast<int>(std::round(m_userViewScale * 100.0f))) + L"%", D2D1::RectF(552.0f, 552.0f, 728.0f, 584.0f), m_centerFormat, D2D1::ColorF(0xF6FF83));
    DrawButton(OptionsViewUpButtonRect(), L"+", true, D2D1::ColorF(0x202833));
    DrawButton(OptionsViewResetButtonRect(), L"자동 맞춤", true, D2D1::ColorF(0x2D3722));
    DrawString(L"화면이 잘리면 값을 낮춰줘.", D2D1::RectF(442.0f, 652.0f, 838.0f, 674.0f), m_centerFormat, D2D1::ColorF(0x8EA9B8));

    DrawButton(OptionsBackButtonRect(), L"뒤로", true, D2D1::ColorF(0x173C4B));
    DrawString(L"Esc / Backspace", D2D1::RectF(OptionsBackButtonRect().left, OptionsBackButtonRect().bottom + 12.0f, OptionsBackButtonRect().right, OptionsBackButtonRect().bottom + 38.0f), m_centerFormat, D2D1::ColorF(0x8EA9B8));
}

void PawlineGameImpl::DrawMenu()
{
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x071017));
    for (float y = 30.0f; y < kHeight; y += 44.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.24f), 1.0f);
    }
    for (float x = 32.0f; x < kWidth; x += 48.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.18f), 1.0f);
    }

    DrawPixelText(L"PAWLINE DEFENSE", {48.0f, 42.0f}, 4.2f, D2D1::ColorF(0xF3FBFF));
    DrawPixelText(L"CHOOSE A STAGE AND FIVE UNITS", {58.0f, 92.0f}, 2.1f, D2D1::ColorF(0x9AB2BF));
    DrawPixelText(L"LUMEN " + ToWideInt(m_lumen), {1000.0f, 50.0f}, 3.0f, D2D1::ColorF(0xF6FF83));
    if (m_debugMode)
    {
        DrawPixelText(L"DEBUG ON", {1004.0f, 88.0f}, 2.0f, D2D1::ColorF(0xFFB6C2));
    }

    DrawString(L"Stage", D2D1::RectF(48.0f, 112.0f, 260.0f, 138.0f), m_headerFormat, D2D1::ColorF(0xEAF7FF));
    for (int i = 0; i < kStageCount; ++i)
    {
        DrawStageCard(i);
    }

    DrawString(L"Loadout", D2D1::RectF(58.0f, 326.0f, 280.0f, 352.0f), m_headerFormat, D2D1::ColorF(0xEAF7FF));
    for (int i = 0; i < kLoadoutSize; ++i)
    {
        DrawLoadoutSlot(i);
    }

    DrawString(L"Roster", D2D1::RectF(650.0f, 326.0f, 850.0f, 352.0f), m_headerFormat, D2D1::ColorF(0xEAF7FF));
    for (int i = 0; i < kRosterCount; ++i)
    {
        DrawRosterCard(i);
    }

    const StageDefinition stage = CurrentStage();
    FillRoundRect(D2D1::RectF(58.0f, 510.0f, 570.0f, 724.0f), 8.0f, D2D1::ColorF(0x0E1D26));
    StrokeRoundRect(D2D1::RectF(58.0f, 510.0f, 570.0f, 724.0f), 8.0f, D2D1::ColorF(0x2C4A5B), 1.0f);
    DrawString(stage.name, D2D1::RectF(82.0f, 532.0f, 540.0f, 562.0f), m_headerFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(stage.subtitle, D2D1::RectF(82.0f, 566.0f, 540.0f, 590.0f), m_bodyFormat, D2D1::ColorF(0xBFD1DB));
    DrawString(stage.gimmick, D2D1::RectF(82.0f, 594.0f, 540.0f, 618.0f), m_bodyFormat, D2D1::ColorF(0xF6FF83));
    DrawString(L"Enemies  " + StageEnemySummary(), D2D1::RectF(82.0f, 622.0f, 540.0f, 646.0f), m_bodyFormat, D2D1::ColorF(0xFFB6C2));
    DrawString(L"Enemy base " + ToWideInt(static_cast<int>(stage.enemyHp)) + L" HP", D2D1::RectF(82.0f, 650.0f, 540.0f, 674.0f), m_bodyFormat, D2D1::ColorF(0xFFB6C2));
    DrawString(L"Start energy " + ToWideInt(static_cast<int>(stage.startEnergy)), D2D1::RectF(82.0f, 676.0f, 540.0f, 700.0f), m_bodyFormat, D2D1::ColorF(0xB8FF89));

    const bool selectedUnlocked = IsStageUnlocked(m_selectedStage);
    DrawButton(MenuCodexButtonRect(), L"Codex", true, D2D1::ColorF(0x22323F));
    DrawButton(MenuShopButtonRect(), L"Shop", true, D2D1::ColorF(0x4B4321));
    DrawButton(StartGameButtonRect(), selectedUnlocked ? L"Start Stage" : L"Locked", selectedUnlocked, D2D1::ColorF(0x173C4B));
    DrawString(L"C", D2D1::RectF(MenuCodexButtonRect().left, MenuCodexButtonRect().bottom + 4.0f, MenuCodexButtonRect().right, MenuCodexButtonRect().bottom + 24.0f), m_centerFormat, D2D1::ColorF(0x8EA9B8));
    DrawString(L"S / B", D2D1::RectF(MenuShopButtonRect().left, MenuShopButtonRect().bottom + 4.0f, MenuShopButtonRect().right, MenuShopButtonRect().bottom + 24.0f), m_centerFormat, D2D1::ColorF(0x8EA9B8));
    DrawString(selectedUnlocked ? L"Enter / Space" : L"Clear previous", D2D1::RectF(StartGameButtonRect().left, StartGameButtonRect().bottom + 4.0f, StartGameButtonRect().right, StartGameButtonRect().bottom + 24.0f), m_centerFormat, selectedUnlocked ? D2D1::ColorF(0x8EA9B8) : D2D1::ColorF(0xFFB6C2));
}

void PawlineGameImpl::DrawCodex()
{
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x071017));
    for (float y = 28.0f; y < kHeight; y += 42.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.18f), 1.0f);
    }
    for (float x = 34.0f; x < kWidth; x += 50.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.12f), 1.0f);
    }

    DrawPixelText(L"FIELD CODEX", {52.0f, 42.0f}, 4.2f, D2D1::ColorF(0xF3FBFF));
    DrawString(L"유닛, 적, 행성 정보를 한곳에서 확인합니다.", D2D1::RectF(58.0f, 92.0f, 520.0f, 120.0f), m_bodyFormat, D2D1::ColorF(0xBFD1DB));

    const std::array<std::wstring, 3> tabs = {L"유닛", L"적", L"스테이지"};
    for (int i = 0; i < 3; ++i)
    {
        const bool active = m_codexTab == i;
        DrawButton(CodexTabRect(i), tabs[i], true, active ? D2D1::ColorF(0x173C4B) : D2D1::ColorF(0x202833));
    }

    const D2D1_RECT_F board = D2D1::RectF(52.0f, 186.0f, 1228.0f, 690.0f);
    DrawCartoonPanel(board, D2D1::ColorF(0x0D1821, 0.96f), D2D1::ColorF(0x65B8FF));

    if (m_codexTab == 0)
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
    else if (m_codexTab == 1)
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

    DrawButton(CodexBackButtonRect(), L"Back", true, D2D1::ColorF(0x173C4B));
    DrawString(L"Esc / M", D2D1::RectF(CodexBackButtonRect().right + 12.0f, CodexBackButtonRect().top + 14.0f, CodexBackButtonRect().right + 120.0f, CodexBackButtonRect().bottom), m_bodyFormat, D2D1::ColorF(0x8EA9B8));
}

void PawlineGameImpl::DrawBriefing()
{
    const StageDefinition stage = CurrentStage();
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x071017));
    FillEllipse({250.0f, 184.0f}, 360.0f, 170.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.10f));
    FillEllipse({1040.0f, 554.0f}, 420.0f, 210.0f, D2D1::ColorF(stage.laneColor.r, stage.laneColor.g, stage.laneColor.b, 0.30f));
    for (int i = 0; i < 120; ++i)
    {
        const float x = 34.0f + std::fmod(static_cast<float>(i * 127 + m_selectedStage * 31), 1212.0f);
        const float y = 28.0f + std::fmod(static_cast<float>(i * 83 + m_selectedStage * 47), 732.0f);
        const float alpha = 0.10f + Hash01(static_cast<float>(i), static_cast<float>(m_selectedStage), std::floor(m_uiTime * 2.0f)) * 0.18f;
        FillEllipse({x, y}, 1.2f, 1.2f, D2D1::ColorF(0xEAF7FF, alpha));
    }

    DrawPixelText(L"MISSION BRIEF", {74.0f, 48.0f}, 5.0f, D2D1::ColorF(0xF3FBFF));
    DrawPixelText(L"LUMEN " + ToWideInt(m_lumen), {1028.0f, 54.0f}, 2.8f, D2D1::ColorF(0xF6FF83));

    const D2D1_RECT_F planet = D2D1::RectF(78.0f, 136.0f, 516.0f, 500.0f);
    DrawCartoonPanel(planet, D2D1::ColorF(0x0D1821, 0.96f), stage.lineColor);
    FillEllipse({208.0f, 282.0f}, 84.0f, 84.0f, D2D1::ColorF(stage.laneColor.r, stage.laneColor.g, stage.laneColor.b, 0.92f));
    FillEllipse({176.0f, 246.0f}, 34.0f, 16.0f, D2D1::ColorF(0xFFFFFF, 0.10f));
    StrokeEllipse({208.0f, 282.0f}, 84.0f, 84.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.92f), 4.0f);
    if (m_selectedStage == 5)
    {
        StrokeEllipse({208.0f, 282.0f}, 138.0f, 42.0f, D2D1::ColorF(0xE6D392, 0.62f), 4.0f);
    }
    if (m_selectedStage == 9)
    {
        for (int i = 0; i < 12; ++i)
        {
            const float a = static_cast<float>(i) / 12.0f * kPi * 2.0f + m_uiTime * 0.4f;
            DrawLine({208.0f + std::cos(a) * 94.0f, 282.0f + std::sin(a) * 94.0f},
                     {208.0f + std::cos(a) * 126.0f, 282.0f + std::sin(a) * 126.0f},
                     D2D1::ColorF(0xFFB347, 0.46f), 4.0f);
        }
    }
    DrawString(stage.name, D2D1::RectF(324.0f, 170.0f, 488.0f, 216.0f), m_titleFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(stage.subtitle, D2D1::RectF(324.0f, 224.0f, 488.0f, 252.0f), m_bodyFormat, D2D1::ColorF(0xBFD1DB));
    DrawPixelText(L"EVENT", {324.0f, 278.0f}, 2.5f, D2D1::ColorF(0xF6FF83));
    DrawString(stage.gimmick, D2D1::RectF(324.0f, 304.0f, 488.0f, 344.0f), m_bodyFormat, D2D1::ColorF(0xF6FF83));
    DrawPixelText(L"ENEMIES", {324.0f, 372.0f}, 2.5f, D2D1::ColorF(0xFFB6C2));
    DrawString(StageEnemySummary(), D2D1::RectF(324.0f, 398.0f, 496.0f, 444.0f), m_bodyFormat, D2D1::ColorF(0xFFCAD1));

    const D2D1_RECT_F plan = D2D1::RectF(560.0f, 136.0f, 1202.0f, 500.0f);
    DrawCartoonPanel(plan, D2D1::ColorF(0x0D1821, 0.96f), D2D1::ColorF(0x65B8FF));
    DrawPixelText(L"LOADOUT CHECK", {598.0f, 166.0f}, 3.4f, D2D1::ColorF(0xEAF7FF));
    DrawPixelText(L"CLICK A SLOT TO EDIT", {598.0f, 210.0f}, 2.2f, D2D1::ColorF(0x9AB2BF));
    for (int i = 0; i < kLoadoutSize; ++i)
    {
        const D2D1_RECT_F src = MenuLoadoutSlotRect(i);
        const D2D1_RECT_F rect = D2D1::RectF(604.0f + static_cast<float>(i) * 112.0f, 260.0f, 700.0f + static_cast<float>(i) * 112.0f, 390.0f);
        const PlayerUnit unit = m_loadout[i];
        const UnitStats stats = PlayerStats(unit);
        const bool hover = Contains(src, m_mouse) || Contains(rect, m_mouse);
        DrawCartoonPanel(rect, hover ? D2D1::ColorF(0x142633, 0.98f) : D2D1::ColorF(0x0F1A22, 0.98f), stats.accent, hover);
        DrawPlayerIcon(unit, {rect.left + 48.0f, rect.top + 38.0f}, 0.78f, true);
        DrawPixelTextCentered(stats.name, D2D1::RectF(rect.left + 6.0f, rect.top + 76.0f, rect.right - 6.0f, rect.top + 100.0f), 1.55f, D2D1::ColorF(0xF3FBFF), 1.0f);
        DrawPixelTextCentered(L"KEY " + ToWideInt(i + 1), D2D1::RectF(rect.left + 8.0f, rect.bottom - 26.0f, rect.right - 8.0f, rect.bottom - 7.0f), 1.55f, D2D1::ColorF(0xCFE8F5), 1.0f);
    }
    DrawString(L"Enemy Base HP  " + ToWideInt(static_cast<int>(stage.enemyHp)), D2D1::RectF(604.0f, 420.0f, 820.0f, 448.0f), m_bodyFormat, D2D1::ColorF(0xFFB6C2));
    DrawString(L"Start Energy  " + ToWideInt(static_cast<int>(stage.startEnergy)), D2D1::RectF(836.0f, 420.0f, 1060.0f, 448.0f), m_bodyFormat, D2D1::ColorF(0xB8FF89));
    DrawString(L"Boss First  " + ToWideInt(static_cast<int>(stage.bossFirstTime)) + L"s", D2D1::RectF(604.0f, 454.0f, 820.0f, 482.0f), m_bodyFormat, D2D1::ColorF(0xFFB347));
    DrawString(L"Event Every  " + ToWideInt(static_cast<int>(GimmickInterval())) + L"s", D2D1::RectF(836.0f, 454.0f, 1060.0f, 482.0f), m_bodyFormat, D2D1::ColorF(0xF6FF83));

    DrawPixelText(L"DIFFICULTY", {604.0f, 500.0f}, 2.3f, D2D1::ColorF(0xEAF7FF));
    const std::array<std::wstring, 3> labels = {L"EASY", L"NORMAL", L"HARD"};
    for (int i = 0; i < 3; ++i)
    {
        const bool active = static_cast<int>(m_difficulty) == i;
        DrawButton(BriefingDifficultyRect(i), labels[i], true, active ? D2D1::ColorF(0x283B27) : D2D1::ColorF(0x202833));
    }
    DrawSynergyPanel(D2D1::RectF(604.0f, 584.0f, 1196.0f, 666.0f));

    DrawButton(BriefingBackButtonRect(), L"Back", true, D2D1::ColorF(0x173C4B));
    DrawButton(BriefingShopButtonRect(), L"Shop", true, D2D1::ColorF(0x4B4321));
    DrawButton(BriefingStartButtonRect(), L"Launch", true, D2D1::ColorF(0x283B27));
    DrawPixelTextCentered(L"ENTER / SPACE", D2D1::RectF(BriefingStartButtonRect().left, BriefingStartButtonRect().bottom + 8.0f, BriefingStartButtonRect().right, BriefingStartButtonRect().bottom + 28.0f), 1.8f, D2D1::ColorF(0x8EA9B8), 1.0f);
}

void PawlineGameImpl::DrawShop()
{
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x071017));
    for (float y = 30.0f; y < kHeight; y += 44.0f)
    {
        DrawLine({24.0f, y}, {1256.0f, y}, D2D1::ColorF(0x17303C, 0.20f), 1.0f);
    }
    for (float x = 32.0f; x < kWidth; x += 48.0f)
    {
        DrawLine({x, 18.0f}, {x, 780.0f}, D2D1::ColorF(0x17303C, 0.13f), 1.0f);
    }

    DrawPixelText(L"UNIT SHOP", {48.0f, 42.0f}, 4.4f, D2D1::ColorF(0xF3FBFF));
    DrawPixelText(L"BUY LOCKED UNITS OR UPGRADE OWNED UNITS", {58.0f, 92.0f}, 2.1f, D2D1::ColorF(0x9AB2BF));
    DrawPixelText(L"LUMEN " + ToWideInt(m_lumen), {990.0f, 50.0f}, 3.0f, D2D1::ColorF(0xF6FF83));

    for (int i = 0; i < kRosterCount; ++i)
    {
        DrawShopUnitCard(i);
    }

    DrawShopUnitDetail();

    DrawButton(ShopBackButtonRect(), L"Back", true, D2D1::ColorF(0x173C4B));
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
    DrawString(unlocked ? L"Lv." + ToWideInt(level) + L" / " + ToWideInt(kMaxUnitLevel) : L"LOCKED", D2D1::RectF(rect.left + 92.0f, rect.top + 42.0f, rect.right - 12.0f, rect.top + 64.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2));
    DrawString(L"Battle cost " + ToWideInt(stats.cost), D2D1::RectF(rect.left + 92.0f, rect.top + 64.0f, rect.right - 12.0f, rect.top + 86.0f), m_smallFormat, D2D1::ColorF(0xC7D8FF));

    std::wstring action = maxed ? L"MAX" : (unlocked ? L"Upgrade " : L"Buy ");
    if (!maxed)
    {
        action += ToWideInt(cost);
    }
    DrawString(action, D2D1::RectF(rect.left + 92.0f, rect.bottom - 28.0f, rect.right - 12.0f, rect.bottom - 8.0f), m_smallFormat, maxed ? D2D1::ColorF(0xF6FF83) : (affordable ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0x7E919C)));
}

void PawlineGameImpl::DrawSynergyPanel(D2D1_RECT_F rect)
{
    // 편성 조합에 따라 활성화된 보너스를 한눈에 보여주는 패널이다.
    DrawCartoonPanel(rect, D2D1::ColorF(0x0F1A22, 0.96f), D2D1::ColorF(0xB8FF89));
    DrawPixelText(L"SYNERGY", {rect.left + 14.0f, rect.top + 12.0f}, 2.4f, D2D1::ColorF(0xB8FF89));
    DrawPixelTextCentered(SynergySummary(), D2D1::RectF(rect.left + 12.0f, rect.top + 38.0f, rect.right - 12.0f, rect.bottom - 10.0f), 1.55f, D2D1::ColorF(0xF3FBFF), 1.0f);
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
    DrawString(unlocked ? stageLabel : L"LOCKED", D2D1::RectF(rect.left + 84.0f, rect.bottom - 24.0f, rect.right - 10.0f, rect.bottom - 6.0f), m_smallFormat, active ? D2D1::ColorF(0xF6FF83) : (unlocked ? D2D1::ColorF(0x7E919C) : D2D1::ColorF(0xFFB6C2)));
    if (!unlocked)
    {
        FillRoundRect(rect, 8.0f, D2D1::ColorF(0x000000, 0.22f));
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
    DrawString(L"Lv." + ToWideInt(UnitLevel(m_loadout[index])) + L"  Key " + ToWideInt(index + 1), D2D1::RectF(rect.left + 8.0f, rect.bottom - 24.0f, rect.right - 8.0f, rect.bottom - 6.0f), m_smallFormat, D2D1::ColorF(0x9AB2BF));
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
    DrawPlayerIcon(unit, {rect.left + 34.0f, rect.top + 31.0f}, 0.62f, unlocked);
    DrawString(stats.name, D2D1::RectF(rect.left + 62.0f, rect.top + 12.0f, rect.right - 8.0f, rect.top + 36.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x73818A));
    DrawString(unlocked ? L"Lv." + ToWideInt(UnitLevel(unit)) : L"LOCKED", D2D1::RectF(rect.left + 62.0f, rect.top + 42.0f, rect.right - 8.0f, rect.top + 62.0f), m_smallFormat, unlocked ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2));
    if (picked)
    {
        DrawString(L"IN", D2D1::RectF(rect.left + 8.0f, rect.bottom - 24.0f, rect.left + 38.0f, rect.bottom - 6.0f), m_smallFormat, stats.accent);
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
        DrawLine({34.0f, y}, {kWorldWidth - 34.0f, y}, D2D1::ColorF(0x27475B, 0.22f), 1.0f);
    }
    for (float x = 42.0f; x < kWorldWidth - 30.0f; x += 46.0f)
    {
        DrawLine({x, kBattleTop + 10.0f}, {x, kBattleBottom - 10.0f}, D2D1::ColorF(0x27475B, 0.13f), 1.0f);
    }

    DrawStageDecorations();
    DrawLongRangeDecorations();

    FillRoundRect(D2D1::RectF(48.0f, kLaneY - kLaneHalfHeight, kWorldWidth - 48.0f, kLaneY + kLaneHalfHeight), 18.0f, D2D1::ColorF(stage.laneColor.r, stage.laneColor.g, stage.laneColor.b, 0.82f));
    FillRoundRect(D2D1::RectF(58.0f, kLaneY - kLaneHalfHeight + 14.0f, kWorldWidth - 58.0f, kLaneY + kLaneHalfHeight - 14.0f), 16.0f, D2D1::ColorF(stage.laneInnerColor.r + 0.02f, stage.laneInnerColor.g + 0.025f, stage.laneInnerColor.b + 0.035f, 0.90f));
    FillRoundRect(D2D1::RectF(58.0f, kLaneY - kLaneHalfHeight + 14.0f, kWorldWidth - 58.0f, kLaneY - 14.0f), 16.0f, D2D1::ColorF(0xFFFFFF, 0.035f));
    FillRoundRect(D2D1::RectF(58.0f, kLaneY + 18.0f, kWorldWidth - 58.0f, kLaneY + kLaneHalfHeight - 14.0f), 16.0f, D2D1::ColorF(0x000000, 0.12f));
    DrawLine({96.0f, kLaneY}, {kEnemyBaseX + 16.0f, kLaneY}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.62f), 3.0f);
    DrawStageLanePattern();

    if (m_cannonFlash > 0.0f)
    {
        const float alpha = Clamp01(m_cannonFlash / 0.42f);
        FillRoundRect(D2D1::RectF(108.0f, kLaneY - 40.0f, kEnemyBaseX + 2.0f, kLaneY + 40.0f), 32.0f, D2D1::ColorF(0xF6FF83, 0.24f * alpha));
        DrawLine({116.0f, kLaneY}, {kEnemyBaseX - 2.0f, kLaneY}, D2D1::ColorF(0xF6FF83, 0.88f * alpha), 11.0f);
    }

    StrokeRoundRect(D2D1::RectF(24.0f, kBattleTop, kWorldWidth - 24.0f, kBattleBottom), 8.0f, D2D1::ColorF(0x2B4A5B), 1.2f);
}

void PawlineGameImpl::DrawSpaceBackdrop()
{
    const StageDefinition stage = CurrentStage();
    const D2D1_RECT_F arena = D2D1::RectF(24.0f, kBattleTop, kWorldWidth - 24.0f, kBattleBottom);

    FillRect(arena, D2D1::ColorF(stage.backColor.r + 0.020f, stage.backColor.g + 0.028f, stage.backColor.b + 0.040f, 0.72f));
    FillEllipse({360.0f + m_cameraX * 0.08f, 170.0f}, 420.0f, 190.0f, D2D1::ColorF(0x325D86, 0.16f));
    FillEllipse({920.0f + m_cameraX * 0.05f, 535.0f}, 520.0f, 210.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.115f));
    FillEllipse({1640.0f - m_cameraX * 0.04f, 214.0f}, 620.0f, 240.0f, D2D1::ColorF(0x7B63CC, 0.105f));
    FillEllipse({2140.0f, 480.0f}, 480.0f, 180.0f, D2D1::ColorF(0x46B5B9, 0.080f));

    for (int i = 0; i < 170; ++i)
    {
        const float x = 48.0f + std::fmod(static_cast<float>(i * 157 + m_selectedStage * 71), kWorldWidth - 96.0f);
        const float y = kBattleTop + 24.0f + std::fmod(static_cast<float>(i * 89 + m_selectedStage * 43), kBattleBottom - kBattleTop - 48.0f);
        const float twinkle = 0.42f + 0.58f * Hash01(static_cast<float>(i), static_cast<float>(m_selectedStage), std::floor(m_stageTime * 2.0f));
        const float radius = 0.85f + static_cast<float>(i % 5) * 0.32f;
        D2D1_COLOR_F star = (i % 7 == 0) ? D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.16f * twinkle)
                                         : D2D1::ColorF(0xEAF7FF, 0.10f + 0.12f * twinkle);
        FillEllipse({x, y}, radius, radius, star);
        if (i % 19 == 0)
        {
            DrawLine({x - 5.0f, y}, {x + 5.0f, y}, D2D1::ColorF(0xFFFFFF, 0.10f * twinkle), 1.0f);
            DrawLine({x, y - 5.0f}, {x, y + 5.0f}, D2D1::ColorF(0xFFFFFF, 0.08f * twinkle), 1.0f);
        }
    }

    for (int i = 0; i < 9; ++i)
    {
        const float y = kBattleTop + 62.0f + static_cast<float>(i) * 54.0f;
        const float wave = std::sin(m_stageTime * 0.38f + static_cast<float>(i) * 0.77f) * 28.0f;
        DrawLine({60.0f + wave, y}, {kWorldWidth - 76.0f + wave * 0.35f, y + 26.0f}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.040f), 5.0f);
    }

    FillRect(D2D1::RectF(arena.left, arena.top, arena.right, arena.top + 76.0f), D2D1::ColorF(0xFFFFFF, 0.020f));
    FillRect(D2D1::RectF(arena.left, arena.bottom - 100.0f, arena.right, arena.bottom), D2D1::ColorF(0x000000, 0.16f));
}

void PawlineGameImpl::DrawCrater(Vec2 center, float rx, float ry, D2D1_COLOR_F rim, D2D1_COLOR_F shade)
{
    FillEllipse(center, rx, ry, shade);
    StrokeEllipse(center, rx, ry, rim, 1.3f);
    FillEllipse({center.x - rx * 0.25f, center.y - ry * 0.22f}, rx * 0.34f, ry * 0.28f, D2D1::ColorF(0xFFFFFF, 0.08f));
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
    const StageDefinition stage = CurrentStage();
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

        DrawLine({x - 140.0f, kLaneY - kLaneHalfHeight + 20.0f}, {x + 260.0f, kLaneY - kLaneHalfHeight + 20.0f}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.08f), 2.0f);
        DrawLine({x - 80.0f, kLaneY + kLaneHalfHeight - 22.0f}, {x + 320.0f, kLaneY + kLaneHalfHeight - 22.0f}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.07f), 2.0f);
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
        DrawLine({82.0f, 320.0f}, {1210.0f, 294.0f}, D2D1::ColorF(0xFFD27A, 0.20f), 5.0f);
        DrawLine({82.0f, 414.0f}, {1210.0f, 442.0f}, D2D1::ColorF(0xFF9BA8, 0.15f), 4.0f);
        break;
    case 2:
        FillEllipse({356.0f, 398.0f}, 88.0f, 24.0f, D2D1::ColorF(0x4BAA75, 0.18f));
        FillEllipse({906.0f, 312.0f}, 110.0f, 26.0f, D2D1::ColorF(0x2B9B8D, 0.15f));
        break;
    case 3:
        DrawLine({74.0f, 298.0f}, {1206.0f, 384.0f}, D2D1::ColorF(0xFF8B60, 0.18f), 5.0f);
        DrawLine({110.0f, 430.0f}, {1170.0f, 344.0f}, D2D1::ColorF(0x6D2D28, 0.18f), 3.0f);
        break;
    case 4:
        FillRoundRect(D2D1::RectF(72.0f, 282.0f, 1210.0f, 304.0f), 10.0f, D2D1::ColorF(0xEAC089, 0.16f));
        FillRoundRect(D2D1::RectF(72.0f, 414.0f, 1210.0f, 436.0f), 10.0f, D2D1::ColorF(0xA66445, 0.15f));
        break;
    case 5:
        StrokeEllipse({640.0f, kLaneY}, 470.0f, 74.0f, D2D1::ColorF(0xE6D392, 0.20f), 3.0f);
        StrokeEllipse({640.0f, kLaneY}, 336.0f, 52.0f, D2D1::ColorF(0xF1E5B9, 0.12f), 2.0f);
        break;
    case 6:
        DrawLine({178.0f, 460.0f}, {318.0f, 270.0f}, D2D1::ColorF(0xB9FFF5, 0.20f), 4.0f);
        DrawLine({784.0f, 462.0f}, {924.0f, 270.0f}, D2D1::ColorF(0x80E5D4, 0.20f), 4.0f);
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
        for (int i = 0; i < 8; ++i)
        {
            const float x = 112.0f + static_cast<float>(i) * 148.0f;
            DrawLine({x, 448.0f}, {x + 64.0f, 268.0f}, D2D1::ColorF(0xFFE66D, 0.16f), 4.0f);
        }
        FillEllipse({646.0f, kLaneY}, 154.0f, 42.0f, D2D1::ColorF(0xFFB347, 0.13f));
        break;
    }
}

void PawlineGameImpl::DrawBases()
{
    const float playerShake = m_playerBaseShake > 0.0f ? std::sin(m_stageTime * 94.0f) * 4.0f : 0.0f;
    const float enemyShake = m_enemyBaseShake > 0.0f ? std::sin(m_stageTime * 94.0f) * 4.0f : 0.0f;
    Vec2 player = {kPlayerBaseX + playerShake, kLaneY};
    Vec2 enemy = {kEnemyBaseX + enemyShake, kLaneY};

    const float playerPulse = 0.55f + 0.45f * std::sin(m_stageTime * 2.2f);
    const float enemyPulse = 0.55f + 0.45f * std::sin(m_stageTime * 2.0f + 1.7f);
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
    FillRoundRect(D2D1::RectF(enemy.x - 62.0f, enemy.y - 72.0f, enemy.x + 62.0f, enemy.y + 60.0f), 10.0f, D2D1::ColorF(0x342731));
    StrokeRoundRect(D2D1::RectF(enemy.x - 62.0f, enemy.y - 72.0f, enemy.x + 62.0f, enemy.y + 60.0f), 10.0f, D2D1::ColorF(0xFF9BA8), 2.5f);
    FillRect(D2D1::RectF(enemy.x - 36.0f, enemy.y - 36.0f, enemy.x - 10.0f, enemy.y + 4.0f), D2D1::ColorF(0xFF9BA8));
    FillRect(D2D1::RectF(enemy.x + 10.0f, enemy.y - 36.0f, enemy.x + 36.0f, enemy.y + 4.0f), D2D1::ColorF(0xFF9BA8));
    DrawLine({enemy.x - 30.0f, enemy.y + 30.0f}, {enemy.x + 30.0f, enemy.y + 30.0f}, D2D1::ColorF(0xFF9BA8), 4.0f);

    DrawBaseHp(player, m_playerBaseHp, m_playerBaseMaxHp, D2D1::ColorF(0x65B8FF));
    DrawBaseHp(enemy, m_enemyBaseHp, m_enemyBaseMaxHp, D2D1::ColorF(0xFF9BA8));
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

void PawlineGameImpl::DrawUnitLighting()
{
    const Vec2 lightDir = Normalize({-0.52f, -0.86f});
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
        const D2D1_COLOR_F accent = unit.team == Team::Player
                                        ? GetPlayerStats(static_cast<PlayerUnit>(unit.kind)).accent
                                        : GetEnemyStats(static_cast<EnemyUnit>(unit.kind), ThreatLevel()).accent;

        const float shadowPush = unit.radius * (0.55f + attack * 0.35f);
        const Vec2 shadow = {pos.x + shadowDir.x * shadowPush + unit.attackDir * attack * 7.0f,
                             pos.y + unit.radius * 0.72f + shadowDir.y * shadowPush * 0.35f};
        FillEllipse(shadow, unit.radius * (1.50f + attack * 0.45f), unit.radius * (0.34f + attack * 0.10f), D2D1::ColorF(0x000000, 0.34f));
        FillEllipse({shadow.x + 8.0f, shadow.y + 2.0f}, unit.radius * (0.95f + attack * 0.30f), unit.radius * 0.22f, D2D1::ColorF(0x000000, 0.20f));

        if (attack > 0.0f)
        {
            const Vec2 front = {pos.x + unit.attackDir * (unit.radius + 28.0f + strike * 28.0f), pos.y - 6.0f};
            FillEllipse(pos, unit.radius * (2.35f + strike * 1.10f), unit.radius * (1.65f + strike * 0.60f), D2D1::ColorF(accent.r, accent.g, accent.b, 0.045f + strike * 0.085f));
            FillEllipse(front, unit.radius * (1.80f + strike * 1.35f), unit.radius * (1.00f + strike * 0.70f), D2D1::ColorF(accent.r, accent.g, accent.b, 0.055f + strike * 0.115f));
            FillEllipse({pos.x - lightDir.x * unit.radius * 0.38f, pos.y - unit.radius * 0.42f}, unit.radius * 0.66f, unit.radius * 0.32f, D2D1::ColorF(0xFFFFFF, 0.045f + strike * 0.045f));
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
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, 0.72f);

    if (windup > 0.0f)
    {
        const float alpha = windup * 0.42f;
        StrokeEllipse({pos.x - dir * (unit.radius + 10.0f), pos.y + 2.0f}, unit.radius + 12.0f, unit.radius * 0.65f, D2D1::ColorF(accent.r, accent.g, accent.b, alpha), 2.2f);
        DrawLine({pos.x - dir * 22.0f, pos.y - unit.radius - 18.0f}, {pos.x - dir * 44.0f, pos.y - unit.radius - 4.0f}, D2D1::ColorF(accent.r, accent.g, accent.b, alpha), 2.0f);
    }

    if (strike > 0.0f)
    {
        const Vec2 front = {pos.x + dir * (unit.radius + 34.0f + strike * 28.0f), pos.y - 2.0f};
        const float width = unit.ranged ? 2.4f : 3.4f;
        for (int i = -1; i <= 1; ++i)
        {
            const float yy = static_cast<float>(i) * 15.0f;
            Vec2 a = {front.x - dir * (18.0f + static_cast<float>(i + 1) * 4.0f), front.y + yy * 0.25f + 6.0f};
            Vec2 b = {front.x + dir * (10.0f + static_cast<float>(i + 1) * 4.0f), front.y + yy * 0.45f};
            DrawLine(a, b, ink, width + 2.8f);
            DrawLine(a, b, D2D1::ColorF(accent.r, accent.g, accent.b, 0.58f * strike), width);
        }
        FillEllipse(front, 18.0f + strike * 14.0f, 9.0f + strike * 6.0f, D2D1::ColorF(accent.r, accent.g, accent.b, 0.12f * strike));
    }

    if (recoil > 0.0f)
    {
        const float alpha = recoil * 0.22f;
        DrawLine({pos.x + dir * (unit.radius + 8.0f), pos.y + unit.radius + 10.0f},
                 {pos.x - dir * (unit.radius + 18.0f), pos.y + unit.radius + 15.0f},
                 D2D1::ColorF(0xFFFFFF, alpha), 2.0f);
    }
}

void PawlineGameImpl::DrawPlayerWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil)
{
    const PlayerUnit type = static_cast<PlayerUnit>(unit.kind);
    const float dir = unit.attackDir;
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, 0.96f);
    const D2D1_COLOR_F white = D2D1::ColorF(0xFFF7D6);
    const float reach = strike * 22.0f - windup * 8.0f + recoil * 4.0f;
    const Vec2 hand = {pos.x + dir * (unit.radius * 0.72f), pos.y + 2.0f};
    const Vec2 front = {pos.x + dir * (unit.radius + 26.0f + reach), pos.y - 4.0f};

    auto drawStroke = [&](Vec2 a, Vec2 b, D2D1_COLOR_F color, float width) {
        DrawLine(a, b, ink, width + 3.2f);
        DrawLine(a, b, color, width);
    };
    auto drawOrb = [&](Vec2 center, float r, D2D1_COLOR_F color) {
        FillEllipse(center, r + 3.0f, r + 3.0f, ink);
        FillEllipse(center, r * 1.8f, r * 1.8f, D2D1::ColorF(color.r, color.g, color.b, 0.11f + strike * 0.08f));
        FillEllipse(center, r, r, color);
        FillEllipse({center.x - r * 0.32f, center.y - r * 0.34f}, r * 0.30f, r * 0.20f, D2D1::ColorF(0xFFFFFF, 0.34f));
    };

    switch (type)
    {
    case PlayerUnit::Paw:
        FillEllipse(front, 13.0f, 10.0f, ink);
        FillEllipse(front, 9.0f, 7.0f, white);
        for (int i = -1; i <= 1; ++i)
        {
            const float yy = static_cast<float>(i) * 7.0f;
            drawStroke({front.x + dir * 7.0f, front.y + yy}, {front.x + dir * (22.0f + strike * 16.0f), front.y + yy - 2.0f}, D2D1::ColorF(0xEAF7FF), 2.0f);
        }
        break;
    case PlayerUnit::Box:
    {
        const D2D1_RECT_F shield = D2D1::RectF(front.x - 13.0f, front.y - 24.0f, front.x + 13.0f, front.y + 24.0f);
        FillRoundRect(shield, 5.0f, ink);
        FillRoundRect(InsetRectF(shield, 3.0f, 3.0f), 4.0f, D2D1::ColorF(0xDCA85B));
        StrokeRoundRect(InsetRectF(shield, 5.0f, 5.0f), 3.0f, D2D1::ColorF(0xFFF0B5), 2.0f);
        if (strike > 0.0f)
        {
            StrokeEllipse({front.x + dir * 12.0f, front.y + 16.0f}, 38.0f, 12.0f, D2D1::ColorF(0xFFF0B5, 0.38f * strike), 3.0f);
        }
        break;
    }
    case PlayerUnit::Spark:
        drawStroke(hand, {hand.x + dir * 12.0f, hand.y - 42.0f}, stats.accent, 3.0f);
        drawOrb({hand.x + dir * 15.0f, hand.y - 47.0f}, 7.0f + strike * 3.0f, D2D1::ColorF(0xF6FF83));
        if (strike > 0.0f)
        {
            DrawLine({hand.x + dir * 15.0f, hand.y - 47.0f}, front, D2D1::ColorF(0x061019, 0.80f), 6.0f);
            DrawLine({hand.x + dir * 15.0f, hand.y - 47.0f}, front, D2D1::ColorF(0xF6FF83, 0.78f * strike), 2.6f);
            DrawLine({front.x - dir * 10.0f, front.y - 8.0f}, {front.x + dir * 8.0f, front.y + 9.0f}, D2D1::ColorF(0xEAF7FF, 0.50f * strike), 1.8f);
        }
        break;
    case PlayerUnit::Dash:
        drawStroke({hand.x - dir * 3.0f, hand.y - 4.0f}, {front.x + dir * 16.0f, front.y - 18.0f}, D2D1::ColorF(0xB8FF89), 2.6f);
        drawStroke({hand.x - dir * 3.0f, hand.y + 9.0f}, {front.x + dir * 18.0f, front.y + 12.0f}, D2D1::ColorF(0xEAF7FF), 2.2f);
        if (strike > 0.0f)
        {
            FillEllipse({pos.x - dir * 42.0f, pos.y + 10.0f}, 28.0f, 7.0f, D2D1::ColorF(0xB8FF89, 0.18f * strike));
        }
        break;
    case PlayerUnit::Bell:
        drawStroke(hand, {hand.x + dir * 18.0f, hand.y - 28.0f}, stats.accent, 2.6f);
        FillEllipse({hand.x + dir * 22.0f, hand.y - 34.0f}, 12.0f, 9.0f, ink);
        FillEllipse({hand.x + dir * 22.0f, hand.y - 34.0f}, 8.0f, 6.0f, D2D1::ColorF(0xF2C94C));
        if (strike > 0.0f)
        {
            StrokeEllipse(front, 30.0f + strike * 24.0f, 16.0f + strike * 12.0f, D2D1::ColorF(0xF6FF83, 0.48f * strike), 2.5f);
            StrokeEllipse(front, 52.0f + strike * 28.0f, 26.0f + strike * 15.0f, D2D1::ColorF(0xF6FF83, 0.26f * strike), 1.8f);
        }
        break;
    case PlayerUnit::Titan:
    case PlayerUnit::Solar:
    {
        const bool solar = type == PlayerUnit::Solar;
        const D2D1_COLOR_F blade = solar ? D2D1::ColorF(0xFFE66D) : D2D1::ColorF(0xFFF0B5);
        const Vec2 head = {front.x + dir * 9.0f, front.y + (solar ? -26.0f : 22.0f)};
        drawStroke(hand, head, blade, solar ? 4.0f : 5.0f);
        if (solar)
        {
            drawStroke(head, {head.x + dir * (30.0f + strike * 20.0f), head.y - 12.0f}, blade, 4.4f);
            for (int i = -1; i <= 1; ++i)
            {
                DrawLine(head, {head.x + dir * 36.0f, head.y + static_cast<float>(i) * 18.0f}, D2D1::ColorF(0xFFB347, 0.34f + strike * 0.20f), 2.2f);
            }
        }
        else
        {
            FillRoundRect(D2D1::RectF(head.x - 19.0f, head.y - 12.0f, head.x + 19.0f, head.y + 12.0f), 5.0f, ink);
            FillRoundRect(D2D1::RectF(head.x - 15.0f, head.y - 8.0f, head.x + 15.0f, head.y + 8.0f), 4.0f, D2D1::ColorF(0xDCA85B));
            if (strike > 0.0f)
            {
                StrokeEllipse({head.x, head.y + 15.0f}, 48.0f, 14.0f, D2D1::ColorF(0xFFF0B5, 0.36f * strike), 3.4f);
            }
        }
        break;
    }
    case PlayerUnit::Frost:
        drawStroke(hand, {front.x + dir * 20.0f, front.y - 20.0f}, D2D1::ColorF(0xD9FFF8), 3.4f);
        DrawLine({front.x + dir * 12.0f, front.y - 30.0f}, {front.x + dir * 30.0f, front.y - 12.0f}, D2D1::ColorF(0xB9FFF5, 0.72f), 2.0f);
        StrokeRoundRect(D2D1::RectF(pos.x - dir * 6.0f - 16.0f, pos.y + 5.0f, pos.x - dir * 6.0f + 16.0f, pos.y + 30.0f), 5.0f, D2D1::ColorF(0xD9FFF8, 0.82f), 2.0f);
        break;
    case PlayerUnit::Comet:
        drawStroke(hand, {front.x + dir * 34.0f, front.y - 2.0f}, D2D1::ColorF(0xFFCA7A), 4.0f);
        FillEllipse({pos.x - dir * 44.0f, pos.y + 14.0f}, 34.0f, 8.0f, D2D1::ColorF(0xFFB347, 0.16f + strike * 0.12f));
        break;
    case PlayerUnit::Orbit:
    {
        drawStroke(hand, {hand.x + dir * 8.0f, hand.y - 36.0f}, stats.accent, 2.6f);
        const float orbit = m_stageTime * 3.4f + unit.id * 0.7f;
        for (int i = 0; i < 3; ++i)
        {
            const float a = orbit + static_cast<float>(i) * kPi * 2.0f / 3.0f;
            drawOrb({pos.x + std::cos(a) * (30.0f + strike * 12.0f), pos.y + std::sin(a) * (14.0f + strike * 8.0f)}, 4.5f, stats.accent);
        }
        break;
    }
    case PlayerUnit::Mint:
        drawStroke(hand, {hand.x + dir * 20.0f, hand.y - 34.0f}, D2D1::ColorF(0xD8FFF3), 2.8f);
        DrawLine({front.x - 10.0f, front.y}, {front.x + 10.0f, front.y}, D2D1::ColorF(0xD8FFF3), 3.0f);
        DrawLine({front.x, front.y - 10.0f}, {front.x, front.y + 10.0f}, D2D1::ColorF(0xD8FFF3), 3.0f);
        if (strike > 0.0f)
        {
            FillEllipse(front, 18.0f, 18.0f, D2D1::ColorF(0xD8FFF3, 0.15f * strike));
        }
        break;
    case PlayerUnit::Drill:
        drawStroke(hand, {front.x + dir * 40.0f, front.y}, D2D1::ColorF(0xFFF0C8), 5.0f);
        StrokeEllipse({front.x + dir * 24.0f, front.y}, 24.0f + strike * 8.0f, 9.0f + strike * 3.0f, stats.accent, 2.4f);
        DrawLine({front.x + dir * 8.0f, front.y - 8.0f}, {front.x + dir * 38.0f, front.y + 8.0f}, D2D1::ColorF(0xFFF0C8), 1.8f);
        DrawLine({front.x + dir * 8.0f, front.y + 8.0f}, {front.x + dir * 38.0f, front.y - 8.0f}, D2D1::ColorF(0xFFF0C8), 1.8f);
        break;
    case PlayerUnit::Prism:
        drawStroke(hand, {hand.x + dir * 18.0f, hand.y - 40.0f}, D2D1::ColorF(0xF7D6FF), 3.2f);
        FillRoundRect(D2D1::RectF(front.x - 8.0f, front.y - 24.0f, front.x + 12.0f, front.y + 4.0f), 3.0f, ink);
        StrokeRoundRect(D2D1::RectF(front.x - 8.0f, front.y - 24.0f, front.x + 12.0f, front.y + 4.0f), 3.0f, D2D1::ColorF(0xF7D6FF), 2.0f);
        if (strike > 0.0f)
        {
            DrawLine({front.x, front.y - 10.0f}, {front.x + dir * 72.0f, front.y - 6.0f}, D2D1::ColorF(0xF7D6FF, 0.72f * strike), 3.0f);
            DrawLine({front.x, front.y - 10.0f}, {front.x + dir * 72.0f, front.y + 10.0f}, D2D1::ColorF(0x65D8FF, 0.36f * strike), 2.0f);
        }
        break;
    case PlayerUnit::Nebula:
        drawStroke(hand, {hand.x + dir * 12.0f, hand.y - 44.0f}, D2D1::ColorF(0xC8B7FF), 3.0f);
        drawOrb({hand.x + dir * 14.0f, hand.y - 50.0f}, 9.0f + strike * 4.0f, D2D1::ColorF(0xE5D9FF));
        StrokeEllipse(pos, unit.radius + 22.0f + strike * 14.0f, unit.radius * 0.82f + strike * 8.0f, D2D1::ColorF(0xC8B7FF, 0.38f), 2.0f);
        break;
    }
}

void PawlineGameImpl::DrawEnemyWeapon(const Unit& unit, Vec2 pos, const UnitStats& stats, float windup, float strike, float recoil)
{
    const EnemyUnit type = static_cast<EnemyUnit>(unit.kind);
    const float dir = unit.attackDir;
    const D2D1_COLOR_F ink = D2D1::ColorF(0x08080F, 0.96f);
    const float reach = strike * 22.0f - windup * 7.0f + recoil * 4.0f;
    const Vec2 hand = {pos.x + dir * (unit.radius * 0.68f), pos.y + 4.0f};
    const Vec2 front = {pos.x + dir * (unit.radius + 24.0f + reach), pos.y - 2.0f};

    auto drawStroke = [&](Vec2 a, Vec2 b, D2D1_COLOR_F color, float width) {
        DrawLine(a, b, ink, width + 3.0f);
        DrawLine(a, b, color, width);
    };
    auto drawBlade = [&](Vec2 a, Vec2 b, D2D1_COLOR_F color) {
        drawStroke(a, b, color, 3.0f);
        DrawLine({b.x - dir * 7.0f, b.y - 8.0f}, {b.x + dir * 7.0f, b.y + 8.0f}, ink, 4.2f);
        DrawLine({b.x - dir * 7.0f, b.y - 8.0f}, {b.x + dir * 7.0f, b.y + 8.0f}, D2D1::ColorF(0xFFE3E8), 2.0f);
    };

    switch (type)
    {
    case EnemyUnit::Dust:
        for (int i = -1; i <= 1; ++i)
        {
            drawStroke({hand.x, hand.y + static_cast<float>(i) * 6.0f}, {front.x + dir * 16.0f, front.y + static_cast<float>(i) * 4.0f}, stats.accent, 1.8f);
        }
        break;
    case EnemyUnit::Brute:
        drawStroke(hand, {front.x + dir * 4.0f, front.y + 24.0f}, D2D1::ColorF(0xC09A75), 5.0f);
        FillRoundRect(D2D1::RectF(front.x - 18.0f, front.y + 14.0f, front.x + 18.0f, front.y + 34.0f), 6.0f, ink);
        FillRoundRect(D2D1::RectF(front.x - 14.0f, front.y + 18.0f, front.x + 14.0f, front.y + 30.0f), 5.0f, D2D1::ColorF(0x8E6B52));
        break;
    case EnemyUnit::Skitter:
        drawBlade(hand, {front.x + dir * 28.0f, front.y - 8.0f}, stats.accent);
        drawBlade({hand.x, hand.y + 12.0f}, {front.x + dir * 24.0f, front.y + 13.0f}, D2D1::ColorF(0xFFB6C2));
        break;
    case EnemyUnit::Sulfur:
        FillRoundRect(D2D1::RectF(front.x - 18.0f, front.y - 12.0f, front.x + 24.0f, front.y + 12.0f), 8.0f, ink);
        FillRoundRect(D2D1::RectF(front.x - 13.0f, front.y - 8.0f, front.x + 17.0f, front.y + 8.0f), 6.0f, D2D1::ColorF(0xFFD27A));
        FillEllipse({front.x + dir * 30.0f, front.y}, 16.0f + strike * 12.0f, 10.0f + strike * 7.0f, D2D1::ColorF(0xFFD27A, 0.18f + strike * 0.12f));
        break;
    case EnemyUnit::Moss:
        drawStroke(hand, {front.x + dir * 18.0f, front.y - 12.0f}, D2D1::ColorF(0xB8FF89), 2.6f);
        drawStroke({hand.x, hand.y + 10.0f}, {front.x + dir * 20.0f, front.y + 18.0f}, D2D1::ColorF(0x6BAA5C), 2.2f);
        StrokeEllipse(front, 24.0f + strike * 12.0f, 10.0f + strike * 6.0f, D2D1::ColorF(0xB8FF89, 0.30f * (0.5f + strike)), 2.0f);
        break;
    case EnemyUnit::Rust:
        drawBlade(hand, {front.x + dir * 20.0f, front.y - 18.0f}, D2D1::ColorF(0xD77A5C));
        FillEllipse({front.x + dir * 13.0f, front.y - 18.0f}, 9.0f, 5.0f, D2D1::ColorF(0xB25E4C));
        break;
    case EnemyUnit::Storm:
        FillRoundRect(D2D1::RectF(front.x - 24.0f, front.y - 20.0f, front.x + 12.0f, front.y + 22.0f), 8.0f, ink);
        StrokeRoundRect(D2D1::RectF(front.x - 20.0f, front.y - 16.0f, front.x + 8.0f, front.y + 18.0f), 6.0f, stats.accent, 3.0f);
        StrokeEllipse(front, 44.0f + strike * 22.0f, 18.0f + strike * 7.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.24f + strike * 0.20f), 2.2f);
        break;
    case EnemyUnit::Ring:
        drawStroke(hand, {front.x + dir * 34.0f, front.y - 6.0f}, stats.accent, 3.4f);
        StrokeEllipse({front.x + dir * 20.0f, front.y - 7.0f}, 15.0f, 9.0f, D2D1::ColorF(0xE6D392), 2.0f);
        break;
    case EnemyUnit::Frost:
        drawBlade(hand, {front.x + dir * 26.0f, front.y - 22.0f}, D2D1::ColorF(0xD9FFF8));
        DrawLine({front.x + dir * 8.0f, front.y - 28.0f}, {front.x + dir * 28.0f, front.y - 8.0f}, D2D1::ColorF(0xB9FFF5, 0.62f), 2.0f);
        break;
    case EnemyUnit::Tide:
        drawStroke(hand, {front.x + dir * 26.0f, front.y - 4.0f}, D2D1::ColorF(0xBFD9FF), 3.2f);
        DrawLine({front.x + dir * 20.0f, front.y - 14.0f}, {front.x + dir * 34.0f, front.y - 4.0f}, D2D1::ColorF(0xBFD9FF), 2.0f);
        DrawLine({front.x + dir * 20.0f, front.y + 6.0f}, {front.x + dir * 34.0f, front.y - 4.0f}, D2D1::ColorF(0xBFD9FF), 2.0f);
        StrokeEllipse(front, 30.0f + strike * 18.0f, 12.0f + strike * 8.0f, D2D1::ColorF(0x75A7FF, 0.24f + strike * 0.16f), 2.2f);
        break;
    case EnemyUnit::Void:
    case EnemyUnit::Boss:
        FillEllipse(front, unit.radius * 0.58f + strike * 8.0f, unit.radius * 0.58f + strike * 8.0f, ink);
        FillEllipse(front, unit.radius * 0.38f + strike * 6.0f, unit.radius * 0.38f + strike * 6.0f, stats.accent);
        StrokeEllipse(pos, unit.radius + 20.0f + strike * 20.0f, unit.radius + 10.0f + strike * 12.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.32f + strike * 0.18f), 2.5f);
        if (type == EnemyUnit::Boss)
        {
            for (int i = -2; i <= 2; ++i)
            {
                DrawLine(pos, {front.x + dir * 42.0f, front.y + static_cast<float>(i) * 16.0f}, D2D1::ColorF(0xFFB347, 0.20f + strike * 0.18f), 2.2f);
            }
        }
        break;
    case EnemyUnit::Flare:
        drawBlade(hand, {front.x + dir * 28.0f, front.y - 12.0f}, D2D1::ColorF(0xFFDB7A));
        FillEllipse({front.x - dir * 18.0f, front.y + 8.0f}, 28.0f, 7.0f, D2D1::ColorF(0xFF6A3D, 0.22f + strike * 0.12f));
        break;
    case EnemyUnit::Spore:
        FillEllipse(front, 15.0f, 12.0f, ink);
        FillEllipse(front, 10.0f, 8.0f, D2D1::ColorF(0xFFB6E8));
        for (int i = -1; i <= 1; ++i)
        {
            FillEllipse({front.x + dir * (24.0f + strike * 18.0f), front.y + static_cast<float>(i) * 10.0f}, 5.0f, 5.0f, D2D1::ColorF(0xFFB6E8, 0.30f + strike * 0.25f));
        }
        break;
    case EnemyUnit::Quake:
        drawStroke(hand, {front.x + dir * 8.0f, front.y + 28.0f}, D2D1::ColorF(0xC09A75), 5.0f);
        FillRoundRect(D2D1::RectF(front.x - 24.0f, front.y + 18.0f, front.x + 24.0f, front.y + 38.0f), 7.0f, ink);
        FillRoundRect(D2D1::RectF(front.x - 19.0f, front.y + 22.0f, front.x + 19.0f, front.y + 34.0f), 5.0f, D2D1::ColorF(0x7B5D45));
        if (strike > 0.0f)
        {
            StrokeEllipse({front.x, front.y + 34.0f}, 58.0f, 16.0f, D2D1::ColorF(0xD8A66A, 0.34f * strike), 3.2f);
        }
        break;
    case EnemyUnit::Mirror:
        FillRoundRect(D2D1::RectF(front.x - 16.0f, front.y - 28.0f, front.x + 16.0f, front.y + 14.0f), 5.0f, ink);
        FillRoundRect(D2D1::RectF(front.x - 11.0f, front.y - 23.0f, front.x + 11.0f, front.y + 9.0f), 4.0f, D2D1::ColorF(0xEAF7FF, 0.66f));
        DrawLine({front.x - 7.0f, front.y - 15.0f}, {front.x + 8.0f, front.y - 3.0f}, stats.accent, 1.8f);
        break;
    case EnemyUnit::Comet:
        drawStroke(hand, {front.x + dir * 34.0f, front.y + 2.0f}, D2D1::ColorF(0xFFDB7A), 3.8f);
        FillEllipse({pos.x - dir * 44.0f, pos.y + 12.0f}, 36.0f, 8.0f, D2D1::ColorF(0xFFDB7A, 0.18f + strike * 0.12f));
        break;
    }
}

void PawlineGameImpl::DrawPlayerUnit(const Unit& unit)
{
    const PlayerUnit playerType = static_cast<PlayerUnit>(unit.kind);
    const UnitStats stats = PlayerStats(playerType);
    const Vec2 pos = UnitRenderPos(unit);
    const float attack = AttackIntensity(unit);
    const float windup = AttackWindup(unit);
    const float strike = AttackStrike(unit);
    const float recoil = AttackRecoil(unit);
    const float dir = unit.attackDir;
    const float flash = unit.hitFlash > 0.0f ? 1.0f : 0.0f;
    const D2D1_COLOR_F ink = D2D1::ColorF(0x061019, 0.96f);
    D2D1_COLOR_F body = stats.color;
    body.r = std::min(1.0f, body.r + flash * 0.25f);
    body.g = std::min(1.0f, body.g + flash * 0.25f);
    body.b = std::min(1.0f, body.b + flash * 0.25f);
    const float bodyRx = unit.radius * (1.0f + strike * 0.22f - windup * 0.08f + recoil * 0.05f);
    const float bodyRy = unit.radius * (1.0f - strike * 0.14f + windup * 0.08f + recoil * 0.03f);

    FillEllipse({pos.x, pos.y + unit.radius + 9.0f}, unit.radius * 1.25f, 7.0f, D2D1::ColorF(0x000000, 0.28f));
    FillEllipse({pos.x - unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.46f, unit.radius * 0.46f, ink);
    FillEllipse({pos.x + unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.46f, unit.radius * 0.46f, ink);
    FillEllipse(pos, bodyRx + 3.4f, bodyRy + 3.4f, ink);
    FillEllipse({pos.x - unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.42f, unit.radius * 0.42f, body);
    FillEllipse({pos.x + unit.radius * 0.52f, pos.y - unit.radius * 0.72f}, unit.radius * 0.42f, unit.radius * 0.42f, body);
    FillEllipse(pos, bodyRx, bodyRy, body);
    FillEllipse({pos.x - unit.radius * 0.24f, pos.y - unit.radius * 0.46f}, unit.radius * 0.30f, unit.radius * 0.18f, D2D1::ColorF(0xFFFFFF, 0.18f));
    StrokeEllipse(pos, bodyRx, bodyRy, stats.accent, 2.4f);
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

    DrawPlayerWeapon(unit, pos, stats, windup, strike, recoil);

    FillEllipse({pos.x - unit.radius * 0.34f, pos.y - unit.radius * 0.12f}, 2.6f, 4.2f, D2D1::ColorF(0x071017));
    FillEllipse({pos.x + unit.radius * 0.34f, pos.y - unit.radius * 0.12f}, 2.6f, 4.2f, D2D1::ColorF(0x071017));
    DrawLine({pos.x - unit.radius * 0.25f, pos.y + unit.radius * 0.32f},
             {pos.x + unit.radius * 0.25f, pos.y + unit.radius * 0.32f},
             D2D1::ColorF(0x071017), 1.8f);

    if (attack > 0.0f)
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

void PawlineGameImpl::DrawEnemyUnit(const Unit& unit)
{
    const EnemyUnit type = static_cast<EnemyUnit>(unit.kind);
    const UnitStats stats = GetEnemyStats(type, ThreatLevel());
    const Vec2 pos = UnitRenderPos(unit);
    const float attack = AttackIntensity(unit);
    const float windup = AttackWindup(unit);
    const float strike = AttackStrike(unit);
    const float recoil = AttackRecoil(unit);
    const float dir = unit.attackDir;
    const D2D1_COLOR_F ink = D2D1::ColorF(0x08080F, 0.96f);
    D2D1_COLOR_F body = stats.color;
    if (unit.hitFlash > 0.0f)
    {
        body.r = std::min(1.0f, body.r + 0.28f);
        body.g = std::min(1.0f, body.g + 0.18f);
        body.b = std::min(1.0f, body.b + 0.18f);
    }

    FillEllipse({pos.x, pos.y + unit.radius + 9.0f}, unit.radius * 1.25f, 7.0f, D2D1::ColorF(0x000000, 0.30f));
    const float bodyRx = unit.radius * (1.08f + strike * 0.24f - windup * 0.07f + recoil * 0.04f);
    const float bodyRy = unit.radius * (1.0f - strike * 0.13f + windup * 0.07f);
    FillEllipse(pos, bodyRx + 3.2f, bodyRy + 3.2f, ink);
    FillEllipse(pos, bodyRx, bodyRy, body);
    FillEllipse({pos.x - unit.radius * 0.18f, pos.y - unit.radius * 0.42f}, unit.radius * 0.28f, unit.radius * 0.15f, D2D1::ColorF(0xFFFFFF, 0.12f));
    StrokeEllipse(pos, bodyRx, bodyRy, stats.accent, 2.2f);
    DrawEnemyWeapon(unit, pos, stats, windup, strike, recoil);
    FillEllipse({pos.x - unit.radius * 0.32f, pos.y - unit.radius * 0.12f}, 3.0f, 4.6f, stats.accent);
    FillEllipse({pos.x + unit.radius * 0.32f, pos.y - unit.radius * 0.12f}, 3.0f, 4.6f, stats.accent);
    DrawLine({pos.x - unit.radius * 0.30f, pos.y + unit.radius * 0.34f},
             {pos.x + unit.radius * 0.30f, pos.y + unit.radius * 0.25f},
             stats.accent, 2.0f);

    if (attack > 0.0f)
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

    if (unit.elite || type == EnemyUnit::Boss)
    {
        StrokeEllipse(pos, unit.radius + 8.0f, unit.radius + 8.0f, D2D1::ColorF(0xFF9BA8, 0.62f), 3.0f);
        DrawPixelTextCentered(type == EnemyUnit::Boss ? L"BOSS" : L"ELITE", D2D1::RectF(pos.x - 38.0f, pos.y - 59.0f, pos.x + 38.0f, pos.y - 35.0f), 1.8f, D2D1::ColorF(0xFFE3E8), 1.0f);
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

void PawlineGameImpl::DrawProjectiles()
{
    for (const Projectile& projectile : m_projectiles)
    {
        DrawLine(projectile.lastPos, projectile.pos, D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.18f), 9.0f);
        DrawLine(projectile.lastPos, projectile.pos, D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.64f), 3.2f);
        FillEllipse(projectile.pos, projectile.radius * 2.4f, projectile.radius * 2.4f, D2D1::ColorF(projectile.color.r, projectile.color.g, projectile.color.b, 0.18f));
        FillEllipse(projectile.pos, projectile.radius, projectile.radius, projectile.color);
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
        DrawLine(line.start, line.end, FadeColor(line.color, 0.16f * alpha), line.width * 3.3f);
        DrawLine(line.start, line.end, FadeColor(line.color, 0.72f * alpha), line.width);
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
    const float scanOffset = std::fmod(m_stageTime * 22.0f, 14.0f);

    for (float y = kBattleTop + 4.0f + scanOffset; y < kBattleBottom; y += 14.0f)
    {
        DrawLine({arena.left + 6.0f, y}, {arena.right - 6.0f, y}, FadeColor(stage.lineColor, 0.026f + action * 0.030f), 1.0f);
    }

    for (int i = 0; i < 26; ++i)
    {
        const float x = 62.0f + static_cast<float>((i * 211 + m_selectedStage * 37) % 1150);
        const float y = 118.0f + static_cast<float>((i * 89 + m_selectedStage * 53) % 462);
        const float twinkle = 0.25f + 0.75f * Hash01(static_cast<float>(i), static_cast<float>(m_selectedStage), std::floor(m_stageTime * 3.0f));
        const float radius = 1.0f + static_cast<float>(i % 3) * 0.7f;
        FillEllipse({x, y}, radius + twinkle * 1.5f, radius + twinkle * 1.5f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.035f + twinkle * 0.055f));
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
            FillEllipse({screen.x + unit.attackDir * 20.0f, screen.y - 4.0f}, unit.radius * (2.8f + strike * 2.0f), unit.radius * (1.45f + strike * 0.90f), D2D1::ColorF(accent.r, accent.g, accent.b, 0.030f + strike * 0.060f));
            FillEllipse({screen.x - 18.0f, screen.y + unit.radius + 14.0f}, unit.radius * 1.9f, unit.radius * 0.46f, D2D1::ColorF(0x000000, 0.16f + strike * 0.08f));
            StrokeEllipse({screen.x, screen.y + 4.0f}, unit.radius + 28.0f * strike, unit.radius * 0.58f + 15.0f * strike, D2D1::ColorF(accent.r, accent.g, accent.b, 0.20f * strike), 2.0f + strike * 1.8f);
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

    FillRect(D2D1::RectF(arena.left, arena.top, arena.right, arena.top + 44.0f), D2D1::ColorF(0x000000, 0.11f));
    FillRect(D2D1::RectF(arena.left, arena.bottom - 48.0f, arena.right, arena.bottom), D2D1::ColorF(0x000000, 0.13f));
    FillRect(D2D1::RectF(arena.left, arena.top, arena.left + 52.0f, arena.bottom), D2D1::ColorF(0x000000, 0.10f));
    FillRect(D2D1::RectF(arena.right - 52.0f, arena.top, arena.right, arena.bottom), D2D1::ColorF(0x000000, 0.10f));

    if (action > 0.0f)
    {
        FillRoundRect(D2D1::RectF(arena.left + 2.0f, arena.top + 2.0f, arena.right - 2.0f, arena.bottom - 2.0f), 8.0f, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, action * 0.045f));
        StrokeRoundRect(D2D1::RectF(arena.left + 6.0f, arena.top + 4.0f, arena.right + 2.0f, arena.bottom + 3.0f), 8.0f, D2D1::ColorF(0xFF6A8A, action * 0.26f), 1.4f);
        StrokeRoundRect(D2D1::RectF(arena.left - 2.0f, arena.top - 3.0f, arena.right - 6.0f, arena.bottom - 4.0f), 8.0f, D2D1::ColorF(0x65D8FF, action * 0.24f), 1.4f);
    }

    StrokeRoundRect(InflateRectF(arena, 1.0f, 1.0f), 9.0f, D2D1::ColorF(0x061019, 0.78f), 4.0f);
    StrokeRoundRect(arena, 8.0f, FadeColor(stage.lineColor, 0.22f + action * 0.16f + actionLevel * 0.10f), 1.8f);

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
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0xF6FF83, 0.055f * alpha));
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
        }
        else if (telegraph.shape == TelegraphShape::Line)
        {
            DrawLine(telegraph.start, telegraph.end, D2D1::ColorF(telegraph.color.r, telegraph.color.g, telegraph.color.b, alpha * 0.46f), telegraph.width);
            DrawLine(telegraph.start, telegraph.end, stroke, 3.0f + pct * 4.0f);
            DrawLine(telegraph.start, telegraph.end, D2D1::ColorF(0xFFFFFF, 0.20f + pct * 0.22f), 1.2f + pct * 1.4f);
        }
        else
        {
            FillRoundRect(D2D1::RectF(kPlayerBaseX + 28.0f, kLaneY - kLaneHalfHeight - 18.0f, kEnemyBaseX - 28.0f, kLaneY + kLaneHalfHeight + 18.0f), 18.0f, fill);
            StrokeRoundRect(D2D1::RectF(kPlayerBaseX + 28.0f, kLaneY - kLaneHalfHeight - 18.0f, kEnemyBaseX - 28.0f, kLaneY + kLaneHalfHeight + 18.0f), 18.0f, stroke, 2.0f + pct * 2.6f);
        }

        DrawPixelTextCentered(L"WARNING", D2D1::RectF(telegraph.start.x - 72.0f, telegraph.start.y - telegraph.radius * 0.62f - 32.0f, telegraph.start.x + 72.0f, telegraph.start.y - telegraph.radius * 0.62f - 8.0f), 1.8f, D2D1::ColorF(0xF3FBFF), 0.70f + pct * 0.30f);
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
    const Unit* boss = nullptr;
    for (const Unit& unit : m_units)
    {
        if (unit.team == Team::Enemy && unit.alive && (unit.elite || static_cast<EnemyUnit>(unit.kind) == EnemyUnit::Boss))
        {
            if (!boss || unit.maxHp > boss->maxHp)
            {
                boss = &unit;
            }
        }
    }

    if (m_bossBannerTimer > 0.0f)
    {
        const float alpha = Clamp01(m_bossBannerTimer / 0.55f);
        const float slide = (1.0f - Clamp01(m_bossBannerTimer / 3.15f)) * 18.0f;
        const D2D1_RECT_F panel = D2D1::RectF(286.0f, 198.0f + slide, 994.0f, 314.0f + slide);
        FillRect(D2D1::RectF(0.0f, kBattleTop, kWidth, kBattleBottom), D2D1::ColorF(0x000000, 0.24f * alpha));
        DrawCartoonPanel(panel, D2D1::ColorF(0x190D0F, 0.96f * alpha), D2D1::ColorF(0xFF9BA8, alpha), true);
        for (int i = 0; i < 11; ++i)
        {
            const float x = panel.left + 18.0f + static_cast<float>(i) * 64.0f;
            DrawLine({x, panel.top + 8.0f}, {x + 38.0f, panel.bottom - 8.0f}, D2D1::ColorF(0xFFB347, 0.16f * alpha), 5.0f);
        }
        DrawPixelTextCentered(L"WARNING", D2D1::RectF(panel.left + 30.0f, panel.top + 22.0f, panel.right - 30.0f, panel.top + 62.0f), 4.4f, D2D1::ColorF(0xFFB347), alpha);
        DrawPixelTextCentered(L"BOSS INCOMING", D2D1::RectF(panel.left + 30.0f, panel.top + 70.0f, panel.right - 30.0f, panel.bottom - 18.0f), 3.2f, D2D1::ColorF(0xF3FBFF), alpha);
    }

    if (!boss)
    {
        return;
    }

    const float pct = Clamp01(boss->hp / boss->maxHp);
    const UnitStats stats = GetEnemyStats(static_cast<EnemyUnit>(boss->kind), ThreatLevel());
    const D2D1_RECT_F panel = D2D1::RectF(378.0f, 104.0f, 902.0f, 142.0f);
    DrawCartoonPanel(panel, D2D1::ColorF(0x0D1118, 0.94f), D2D1::ColorF(0xFF9BA8));
    DrawPixelText(L"BOSS", {panel.left + 18.0f, panel.top + 10.0f}, 2.4f, D2D1::ColorF(0xFFB347), 1.0f);
    DrawPixelTextCentered(stats.name, D2D1::RectF(panel.left + 82.0f, panel.top + 7.0f, panel.left + 254.0f, panel.bottom - 7.0f), 2.2f, D2D1::ColorF(0xF3FBFF), 1.0f);
    const D2D1_RECT_F bar = D2D1::RectF(panel.left + 270.0f, panel.top + 13.0f, panel.right - 18.0f, panel.bottom - 13.0f);
    FillRoundRect(bar, 6.0f, D2D1::ColorF(0x061019, 0.94f));
    FillRoundRect(D2D1::RectF(bar.left, bar.top, bar.left + (bar.right - bar.left) * pct, bar.bottom), 6.0f, D2D1::ColorF(0xFF9BA8));
    FillRoundRect(D2D1::RectF(bar.left, bar.top, bar.left + (bar.right - bar.left) * pct, bar.top + 7.0f), 5.0f, D2D1::ColorF(0xFFFFFF, 0.15f));
}

void PawlineGameImpl::DrawTutorialTips()
{
    if (m_screen != GameScreen::Playing || m_stageTime > 14.0f || m_escapeMenuOpen)
    {
        return;
    }

    const float fadeIn = Clamp01(m_stageTime / 0.8f);
    const float fadeOut = Clamp01((14.0f - m_stageTime) / 1.4f);
    const float alpha = std::min(fadeIn, fadeOut);
    const StageDefinition stage = CurrentStage();

    auto tip = [&](D2D1_RECT_F rect, const std::wstring& title, const std::wstring& body, D2D1_COLOR_F accent) {
        DrawCartoonPanel(rect, D2D1::ColorF(0x071017, 0.86f * alpha), accent, false);
        DrawPixelText(title, {rect.left + 12.0f, rect.top + 10.0f}, 2.25f, D2D1::ColorF(0xF3FBFF, alpha), alpha);
        DrawPixelText(body, {rect.left + 12.0f, rect.top + 36.0f}, 1.75f, D2D1::ColorF(0xCFE8F5, alpha), alpha);
    };

    tip(D2D1::RectF(60.0f, 512.0f, 312.0f, 584.0f), L"SUMMON", L"PRESS 1-5 OR CLICK CARDS", D2D1::ColorF(0x65B8FF, alpha));
    DrawLine({144.0f, 584.0f}, {92.0f, 628.0f}, D2D1::ColorF(0x65B8FF, 0.42f * alpha), 2.4f);
    DrawLine({92.0f, 628.0f}, {104.0f, 616.0f}, D2D1::ColorF(0x65B8FF, 0.42f * alpha), 2.4f);

    tip(D2D1::RectF(452.0f, 512.0f, 726.0f, 584.0f), L"WALLET", L"W BOOSTS COST AND PULSE", D2D1::ColorF(0xB8FF89, alpha));
    DrawLine({626.0f, 584.0f}, {728.0f, 628.0f}, D2D1::ColorF(0xB8FF89, 0.42f * alpha), 2.4f);

    tip(D2D1::RectF(890.0f, 118.0f, 1194.0f, 190.0f), L"CAMERA", L"DRAG OR WHEEL THE LANE", stage.lineColor);
    DrawLine({890.0f, 154.0f}, {802.0f, 104.0f}, D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.42f * alpha), 2.4f);

    if (m_stageTime > 4.5f)
    {
        tip(D2D1::RectF(884.0f, 512.0f, 1194.0f, 584.0f), L"MOONBEAM", L"SPACE FIRES WHEN FULL", D2D1::ColorF(0xF6FF83, alpha));
        DrawLine({918.0f, 584.0f}, {740.0f, 708.0f}, D2D1::ColorF(0xF6FF83, 0.38f * alpha), 2.4f);
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
}

void PawlineGameImpl::DrawDebugBadge()
{
    if (!m_debugMode)
    {
        return;
    }

    const D2D1_RECT_F badge = D2D1::RectF(1010.0f, m_showcaseMode && m_screen == GameScreen::Playing ? 154.0f : 112.0f, 1238.0f, m_showcaseMode && m_screen == GameScreen::Playing ? 190.0f : 148.0f);
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
    DrawCartoonPanel(rect, D2D1::ColorF(0x0E1D26, 0.96f), color, Contains(rect, m_mouse));
    DrawPixelText(label, {rect.left + 10.0f, rect.top + 8.0f}, 2.0f, D2D1::ColorF(0xAFC9D5), 0.92f);
    DrawPixelText(value, {rect.left + 10.0f, rect.top + 27.0f}, 2.55f, color, 1.0f);
}

void PawlineGameImpl::DrawCommandBar()
{
    const StageDefinition stage = CurrentStage();
    FillRect(D2D1::RectF(0.0f, 612.0f, kWidth, kHeight), D2D1::ColorF(0x061019));
    FillRect(D2D1::RectF(0.0f, 612.0f, kWidth, 636.0f), D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.10f));
    StrokeRect(D2D1::RectF(0.0f, 614.0f, kWidth, kHeight), D2D1::ColorF(0x061019), 4.0f);
    StrokeRect(D2D1::RectF(0.0f, 614.0f, kWidth, kHeight), D2D1::ColorF(stage.lineColor.r, stage.lineColor.g, stage.lineColor.b, 0.32f), 1.4f);

    for (int i = 0; i < kLoadoutSize; ++i)
    {
        DrawUnitCard(i);
    }

    DrawWalletButton();
    DrawCannonButton();

    DrawCombatHelpPanel();

    DrawButton(PauseButtonRect(), m_paused ? L"Resume" : L"Pause", true, D2D1::ColorF(0x22323F));
    DrawButton(RestartButtonRect(), L"Restart", true, D2D1::ColorF(0x332337));
    DrawButton(SpeedDownButtonRect(), L"-", true, D2D1::ColorF(0x202833));
    DrawPixelTextCentered(L"SPEED X" + ToWideFloat(m_gameSpeed), D2D1::RectF(1062.0f, 731.0f, 1186.0f, 758.0f), 2.65f, D2D1::ColorF(0xF3FBFF), 1.0f);
    DrawButton(SpeedUpButtonRect(), L"+", true, D2D1::ColorF(0x202833));
}

void PawlineGameImpl::DrawCombatHelpPanel()
{
    const StageDefinition stage = CurrentStage();
    const D2D1_RECT_F panel = D2D1::RectF(824.0f, 628.0f, 992.0f, 774.0f);
    DrawCartoonPanel(panel, D2D1::ColorF(0x101A23, 0.96f), stage.lineColor);
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
        DrawPixelText(lines[i], {panel.left + 14.0f, y + 3.0f}, 1.95f,
                      i == 4 ? D2D1::ColorF(0xBFD1DB) : D2D1::ColorF(0xF6FF83),
                      0.96f);
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

    DrawCartoonPanel(rect, hover ? D2D1::ColorF(0x142633, 0.98f) : D2D1::ColorF(0x0F1A22, 0.98f), border, hover);
    if (ready && affordable)
    {
        FillRoundRect(D2D1::RectF(rect.left + 7.0f, rect.top + 7.0f, rect.left + 35.0f, rect.top + 24.0f), 5.0f, D2D1::ColorF(stats.accent.r, stats.accent.g, stats.accent.b, 0.34f));
    }

    DrawPlayerIcon(type, {rect.left + 59.0f, rect.top + 37.0f}, 0.82f, affordable);

    DrawPixelTextCentered(stats.name, D2D1::RectF(rect.left + 6.0f, rect.top + 73.0f, rect.right - 6.0f, rect.top + 95.0f), 1.75f, affordable ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x9AA7B0), 1.0f);
    DrawPixelTextCentered(L"LV." + ToWideInt(UnitLevel(type)) + L" COST " + ToWideInt(cost), D2D1::RectF(rect.left + 6.0f, rect.top + 101.0f, rect.right - 6.0f, rect.top + 122.0f), 1.65f, affordable ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFF9BA8), 1.0f);
    FillRoundRect(D2D1::RectF(rect.left + 15.0f, rect.bottom - 28.0f, rect.right - 15.0f, rect.bottom - 8.0f), 6.0f, D2D1::ColorF(0x061019, 0.58f));
    DrawPixelTextCentered(L"KEY " + ToWideInt(index + 1), D2D1::RectF(rect.left + 12.0f, rect.bottom - 27.0f, rect.right - 12.0f, rect.bottom - 7.0f), 1.65f, D2D1::ColorF(0xCFE8F5), 1.0f);

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
    std::wstring label = cost > 0 ? L"Wallet +" : L"Wallet Max";
    DrawButton(WalletButtonRect(), label, cost > 0, D2D1::ColorF(0x283B27));
    DrawPixelTextCentered(cost > 0 ? L"COST " + ToWideInt(cost) : L"LV.5 MAX",
                          D2D1::RectF(WalletButtonRect().left, WalletButtonRect().top + 50.0f, WalletButtonRect().right, WalletButtonRect().top + 68.0f),
                          1.75f,
                          enabled ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0x9AA7B0),
                          1.0f);
    DrawPixelTextCentered(L"BOOST +" + ToWideInt(static_cast<int>(std::round((WalletUnitBoost() - 1.0f) * 100.0f))) + L"% PULSE " + ToWideInt(static_cast<int>(std::ceil(std::max(0.0f, m_walletPulseTimer)))) + L"S",
                          D2D1::RectF(WalletButtonRect().left + 6.0f, WalletButtonRect().bottom - 24.0f, WalletButtonRect().right - 6.0f, WalletButtonRect().bottom - 6.0f),
                          1.25f,
                          D2D1::ColorF(0xF6FF83),
                          1.0f);
    const D2D1_RECT_F bar = D2D1::RectF(WalletButtonRect().left + 16.0f, WalletButtonRect().bottom - 34.0f, WalletButtonRect().right - 16.0f, WalletButtonRect().bottom - 28.0f);
    FillRoundRect(bar, 3.0f, D2D1::ColorF(0x071017, 0.86f));
    const float pulsePct = 1.0f - Clamp01(m_walletPulseTimer / WalletPulseInterval());
    FillRoundRect(D2D1::RectF(bar.left, bar.top, bar.left + (bar.right - bar.left) * pulsePct, bar.bottom), 3.0f, D2D1::ColorF(0xB8FF89));
}

void PawlineGameImpl::DrawCannonButton()
{
    const bool ready = m_cannonCharge >= 100.0f;
    DrawButton(CannonButtonRect(), ready ? L"Moonbeam" : L"Charging", true, ready ? D2D1::ColorF(0x4B4321) : D2D1::ColorF(0x202833));
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
    if (m_messageTimer <= 0.0f || m_message.empty())
    {
        return;
    }

    const float alpha = Clamp01(m_messageTimer / 0.45f);
    D2D1_RECT_F rect = D2D1::RectF(394.0f, 566.0f, 886.0f, 600.0f);
    DrawCartoonPanel(rect, D2D1::ColorF(0x071017, 0.82f * alpha), D2D1::ColorF(0x65B8FF, alpha));
    DrawPixelTextCentered(m_message, D2D1::RectF(rect.left + 12.0f, rect.top + 7.0f, rect.right - 12.0f, rect.bottom - 5.0f), 2.0f, D2D1::ColorF(0xEAF7FF, alpha), alpha);
}

void PawlineGameImpl::DrawOverlay()
{
    if (!m_paused || m_screen != GameScreen::Playing)
    {
        return;
    }

    FillRect(D2D1::RectF(24.0f, kBattleTop, 1256.0f, kBattleBottom), D2D1::ColorF(0x000000, 0.42f));
    std::wstring title = L"PAUSED";
    std::wstring body = L"Press P to resume.";
    D2D1_COLOR_F color = D2D1::ColorF(0xEAF7FF);

    DrawPixelTextCentered(title, D2D1::RectF(320.0f, 296.0f, 960.0f, 348.0f), 5.0f, color, 1.0f);
    DrawPixelTextCentered(body, D2D1::RectF(320.0f, 352.0f, 960.0f, 386.0f), 2.4f, D2D1::ColorF(0xF3FBFF), 1.0f);
}

void PawlineGameImpl::DrawEscapeMenuClean()
{
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x000000, 0.58f));
    const D2D1_RECT_F panel = D2D1::RectF(412.0f, 156.0f, 868.0f, 684.0f);
    FillRoundRect(panel, 10.0f, D2D1::ColorF(0x0A121A, 0.97f));
    StrokeRoundRect(panel, 10.0f, D2D1::ColorF(0x65B8FF), 1.8f);

    DrawString(L"메뉴", D2D1::RectF(panel.left + 34.0f, panel.top + 28.0f, panel.right - 34.0f, panel.top + 80.0f), m_titleFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"언제든 계속하거나 빠져나갈 수 있어.", D2D1::RectF(panel.left + 34.0f, panel.top + 82.0f, panel.right - 34.0f, panel.top + 110.0f), m_centerFormat, D2D1::ColorF(0xBFD1DB));

    DrawButton(EscapeResumeButtonRect(), L"계속하기", true, D2D1::ColorF(0x173C4B));
    DrawButton(EscapeShakeButtonRect(), m_hitShakeEnabled ? L"피격 흔들림 켜짐" : L"피격 흔들림 꺼짐", true, m_hitShakeEnabled ? D2D1::ColorF(0x173C4B) : D2D1::ColorF(0x302735));

    const float speed = m_screen == GameScreen::Playing ? m_gameSpeed : m_defaultGameSpeed;
    DrawString(L"게임 속도", D2D1::RectF(492.0f, 392.0f, 788.0f, 420.0f), m_centerFormat, D2D1::ColorF(0xEAF7FF));
    DrawButton(EscapeSpeedDownButtonRect(), L"-", true, D2D1::ColorF(0x202833));
    DrawString(L"x" + ToWideFloat(speed), D2D1::RectF(552.0f, 438.0f, 728.0f, 470.0f), m_centerFormat, D2D1::ColorF(0xF6FF83));
    DrawButton(EscapeSpeedUpButtonRect(), L"+", true, D2D1::ColorF(0x202833));

    DrawButton(EscapeStageButtonRect(), L"스테이지 선택", true, D2D1::ColorF(0x283B27));
    DrawButton(EscapeQuitButtonRect(), L"게임 종료", true, D2D1::ColorF(0x332337));
}

void PawlineGameImpl::DrawResultScreen()
{
    FillRect(D2D1::RectF(0.0f, 0.0f, kWidth, kHeight), D2D1::ColorF(0x000000, 0.54f));
    const D2D1_RECT_F panel = D2D1::RectF(340.0f, 210.0f, 940.0f, 555.0f);
    FillRoundRect(panel, 10.0f, D2D1::ColorF(0x0A121A, 0.96f));
    StrokeRoundRect(panel, 10.0f, m_victory ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFF9BA8), 2.0f);

    const StageDefinition stage = CurrentStage();
    const std::wstring title = m_victory ? L"STAGE CLEAR" : L"BASE BROKEN";
    const std::wstring subtitle = m_victory ? L"The lane is yours." : L"The enemy pushed through.";
    DrawPixelTextCentered(title, D2D1::RectF(panel.left + 34.0f, panel.top + 30.0f, panel.right - 34.0f, panel.top + 78.0f), 4.2f, m_victory ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0xFFB6C2), 1.0f);
    DrawPixelTextCentered(subtitle, D2D1::RectF(panel.left + 34.0f, panel.top + 84.0f, panel.right - 34.0f, panel.top + 112.0f), 2.2f, D2D1::ColorF(0xEAF7FF), 1.0f);

    DrawString(stage.name, D2D1::RectF(panel.left + 64.0f, panel.top + 142.0f, panel.right - 64.0f, panel.top + 170.0f), m_headerFormat, D2D1::ColorF(0xF3FBFF));
    DrawString(L"Time  " + ToWideTime(m_resultTime), D2D1::RectF(panel.left + 94.0f, panel.top + 190.0f, panel.left + 280.0f, panel.top + 218.0f), m_bodyFormat, D2D1::ColorF(0xC7D8FF));
    DrawString(L"Score  " + ToWideInt(m_resultScore), D2D1::RectF(panel.left + 320.0f, panel.top + 190.0f, panel.right - 94.0f, panel.top + 218.0f), m_bodyFormat, D2D1::ColorF(0xF6FF83));
    DrawString(L"Wallet Lv." + ToWideInt(m_walletLevel), D2D1::RectF(panel.left + 94.0f, panel.top + 224.0f, panel.left + 280.0f, panel.top + 252.0f), m_bodyFormat, D2D1::ColorF(0xB8FF89));
    DrawString(L"Difficulty  " + DifficultyLabel(), D2D1::RectF(panel.left + 320.0f, panel.top + 224.0f, panel.right - 94.0f, panel.top + 252.0f), m_bodyFormat, D2D1::ColorF(0xD9E5F2));
    DrawString(m_victory ? L"LUMEN +" + ToWideInt(m_lastReward) + L"   Total " + ToWideInt(m_lumen) : L"LUMEN +0   Total " + ToWideInt(m_lumen),
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
    DrawPixelTextCentered(rank, D2D1::RectF(panel.left + 94.0f, panel.top + 314.0f, panel.left + 262.0f, panel.top + 352.0f), 3.2f, m_victory ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0xFFB6C2), 1.0f);
    for (int i = 0; i < 3; ++i)
    {
        const Vec2 medal = {panel.left + 330.0f + static_cast<float>(i) * 62.0f, panel.top + 334.0f};
        const bool earned = i < medalCount;
        FillEllipse(medal, 20.0f, 20.0f, earned ? D2D1::ColorF(0xF6FF83, 0.24f) : D2D1::ColorF(0x0F1A22, 0.86f));
        StrokeEllipse(medal, 20.0f, 20.0f, earned ? D2D1::ColorF(0xF6FF83) : D2D1::ColorF(0x394955), 2.0f);
        DrawPixelTextCentered(earned ? L"OK" : L"--", D2D1::RectF(medal.x - 18.0f, medal.y - 10.0f, medal.x + 18.0f, medal.y + 12.0f), 1.8f, earned ? D2D1::ColorF(0xF3FBFF) : D2D1::ColorF(0x65727C), 1.0f);
    }
    DrawPixelTextCentered(m_victory ? L"CLEAR REWARD BANKED" : L"NO REWARD TRY AGAIN",
                          D2D1::RectF(panel.left + 532.0f, panel.top + 316.0f, panel.right - 52.0f, panel.top + 352.0f),
                          2.2f,
                          m_victory ? D2D1::ColorF(0xB8FF89) : D2D1::ColorF(0x8EA9B8),
                          1.0f);
    DrawPixelTextCentered(GrowthRecommendation(),
                          D2D1::RectF(panel.left + 64.0f, panel.top + 374.0f, panel.right - 64.0f, panel.top + 408.0f),
                          2.1f,
                          D2D1::ColorF(0xF6FF83),
                          1.0f);

    DrawButton(ResultRetryButtonRect(), L"Retry", true, D2D1::ColorF(0x173C4B));
    DrawButton(ResultNextButtonRect(), (m_victory && m_selectedStage < kStageCount - 1) ? L"Next" : L"Close", true, D2D1::ColorF(0x283B27));
    DrawButton(ResultMenuButtonRect(), L"Menu", true, D2D1::ColorF(0x332337));
}
