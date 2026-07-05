#pragma once

#include <windows.h>
#include <memory>

#include "framework/GameFramework.h"

class SpaceDefenceGameImpl;

class SpaceDefenceGame final : public framework::IGameApplication
{
public:
    SpaceDefenceGame();
    ~SpaceDefenceGame();

    SpaceDefenceGame(const SpaceDefenceGame&) = delete;
    SpaceDefenceGame& operator=(const SpaceDefenceGame&) = delete;

    HRESULT Initialize() override;
    int Run() override;

private:
    // 게임 구현 객체는 SpaceDefenceGame이 단독 소유하므로 unique_ptr로 생명주기를 자동 관리한다.
    std::unique_ptr<SpaceDefenceGameImpl> m_impl;
};
