#include <windows.h>
#include <memory>

#include "PawlineGame.h"
#include "framework/GameFramework.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    SetProcessDPIAware();

    auto app = std::make_unique<PawlineGame>();
    framework::GameFramework game(std::move(app));
    const HRESULT hr = game.Initialize();
    if (FAILED(hr))
    {
        MessageBoxW(nullptr, L"Failed to initialize Space Defanse.", L"Space Defanse", MB_ICONERROR | MB_OK);
        return -1;
    }

    return game.Run();
}
