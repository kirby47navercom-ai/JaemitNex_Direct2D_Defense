#include <windows.h>

#include "PawlineGame.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    SetProcessDPIAware();

    PawlineGame app;
    const HRESULT hr = app.Initialize();
    if (FAILED(hr))
    {
        MessageBoxW(nullptr, L"Failed to initialize Pawline Defense.", L"Pawline Defense", MB_ICONERROR | MB_OK);
        return -1;
    }

    return app.Run();
}