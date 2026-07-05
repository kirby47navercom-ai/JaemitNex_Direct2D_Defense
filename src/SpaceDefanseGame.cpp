#include <windows.h>
#include <memory>
#include "SpaceDefanseGame.h"
#include "SpaceDefanseGameImpl.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "winmm.lib")

SpaceDefanseGame::SpaceDefanseGame()
    : m_impl(std::make_unique<SpaceDefanseGameImpl>())
{
}

SpaceDefanseGame::~SpaceDefanseGame() = default;

HRESULT SpaceDefanseGame::Initialize()
{
    return m_impl->Initialize();
}

int SpaceDefanseGame::Run()
{
    return m_impl->Run();
}
