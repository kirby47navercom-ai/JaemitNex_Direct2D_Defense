#include <windows.h>
#include <memory>

#include "SpaceDefenceGame.h"
#include "framework/GameFramework.h"

namespace
{
void EnableDpiAwareness()
{
    // 다른 PC의 Windows 배율(125%, 150%)에서도 마우스 좌표와 Direct2D 좌표가 어긋나지 않게
    // 가능한 최신 Per-Monitor DPI 모드를 먼저 사용하고, 실패하면 기존 DPI 모드로 내려간다.
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll"))
    {
        using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
        auto setAwarenessContext = reinterpret_cast<SetProcessDpiAwarenessContextFn>(
            GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
        if (setAwarenessContext)
        {
            if (setAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) ||
                setAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
            {
                return;
            }
        }
    }

    SetProcessDPIAware();
}
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    EnableDpiAwareness();

    auto app = std::make_unique<SpaceDefenceGame>();
    framework::GameFramework game(std::move(app));
    const HRESULT hr = game.Initialize();
    if (FAILED(hr))
    {
        MessageBoxW(nullptr, L"Failed to initialize Space Defence.", L"Space Defence", MB_ICONERROR | MB_OK);
        return -1;
    }

    return game.Run();
}
