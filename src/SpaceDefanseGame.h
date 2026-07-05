#pragma once

#include <windows.h>
#include <memory>

#include "framework/GameFramework.h"

class SpaceDefanseGameImpl;

class SpaceDefanseGame final : public framework::IGameApplication
{
public:
    SpaceDefanseGame();
    ~SpaceDefanseGame();

    SpaceDefanseGame(const SpaceDefanseGame&) = delete;
    SpaceDefanseGame& operator=(const SpaceDefanseGame&) = delete;

    HRESULT Initialize() override;
    int Run() override;

private:
    // 게임 구현 객체는 SpaceDefanseGame이 단독 소유하므로 unique_ptr로 생명주기를 자동 관리한다.
    std::unique_ptr<SpaceDefanseGameImpl> m_impl;
};
