#pragma once

#include <windows.h>

class PawlineGameImpl;

class PawlineGame
{
public:
    PawlineGame();
    ~PawlineGame();

    PawlineGame(const PawlineGame&) = delete;
    PawlineGame& operator=(const PawlineGame&) = delete;

    HRESULT Initialize();
    int Run();

private:
    PawlineGameImpl* m_impl = nullptr;
};
