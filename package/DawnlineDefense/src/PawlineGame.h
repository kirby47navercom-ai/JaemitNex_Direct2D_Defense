#pragma once

#include <windows.h>
#include <memory>

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
    // 게임 구현 객체는 PawlineGame이 단독 소유하므로 unique_ptr로 생명주기를 자동 관리한다.
    std::unique_ptr<PawlineGameImpl> m_impl;
};
