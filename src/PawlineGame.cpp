#include <windows.h>
#include <memory>
#include "PawlineGame.h"
#include "PawlineGameImpl.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "winmm.lib")

PawlineGame::PawlineGame()
    : m_impl(std::make_unique<PawlineGameImpl>())
{
}

PawlineGame::~PawlineGame() = default;

HRESULT PawlineGame::Initialize()
{
    return m_impl->Initialize();
}

int PawlineGame::Run()
{
    return m_impl->Run();
}
