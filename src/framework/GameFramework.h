#pragma once

#include <memory>
#include <windows.h>

namespace framework
{
// 게임 실행 객체가 반드시 제공해야 하는 최소 인터페이스다.
// 참고 레포의 GameFramework::Run(Scene*) 역할을 Win32/Direct2D 프로젝트에 맞게 얇게 바꾼 형태다.
class IGameApplication
{
public:
    virtual ~IGameApplication() = default;

    // 창, 렌더러, 리소스처럼 게임을 시작하기 전에 필요한 시스템을 준비한다.
    virtual HRESULT Initialize() = 0;

    // 플랫폼별 메시지 루프와 게임 루프를 실행한다.
    virtual int Run() = 0;
};

// main.cpp가 구체적인 게임 구현을 직접 붙잡지 않도록 감싸는 게임 프레임워크 진입점이다.
// 지금은 단일 애플리케이션을 실행하지만, 나중에 SceneManager를 넣어도 이 층을 통해 확장할 수 있다.
class GameFramework
{
public:
    explicit GameFramework(std::unique_ptr<IGameApplication> application);

    GameFramework(const GameFramework&) = delete;
    GameFramework& operator=(const GameFramework&) = delete;

    HRESULT Initialize();
    int Run();

private:
    // 프레임워크가 애플리케이션의 생명주기를 단독 소유한다.
    std::unique_ptr<IGameApplication> m_application;
};
}
