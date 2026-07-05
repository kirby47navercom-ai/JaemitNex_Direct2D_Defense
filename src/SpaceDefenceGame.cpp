#include <windows.h>
#include <memory>
#include "SpaceDefenceGame.h"
#include "SpaceDefenceGameImpl.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "winmm.lib")

SpaceDefenceGame::SpaceDefenceGame()
    : m_impl(std::make_unique<SpaceDefenceGameImpl>())
{
}

SpaceDefenceGame::~SpaceDefenceGame() = default;

HRESULT SpaceDefenceGame::Initialize()
{
    return m_impl->Initialize();
}

int SpaceDefenceGame::Run()
{
    return m_impl->Run();
}
