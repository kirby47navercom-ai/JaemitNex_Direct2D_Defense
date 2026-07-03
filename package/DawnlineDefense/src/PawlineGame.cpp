#include <windows.h>
#include "PawlineGame.h"
#include "PawlineGameImpl.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "winmm.lib")

PawlineGame::PawlineGame()
    : m_impl(new PawlineGameImpl())
{
}

PawlineGame::~PawlineGame()
{
    delete m_impl;
    m_impl = nullptr;
}

HRESULT PawlineGame::Initialize()
{
    return m_impl->Initialize();
}

int PawlineGame::Run()
{
    return m_impl->Run();
}
