#pragma once

#include <memory>
#include <vector>

namespace framework
{
// 게임 화면 하나가 가져야 하는 최소 생명주기다.
// 참고 레포의 Scene 인터페이스처럼 Init/Update/Draw/Finish를 분리해서,
// 나중에 타이틀, 전투, 상점 화면을 독립 씬으로 옮길 수 있게 한다.
class IScene
{
public:
    virtual ~IScene() = default;

    virtual void Init() {}
    virtual void OnPause() {}
    virtual void OnResume() {}
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Finish() {}
};

// 씬을 unique_ptr로 소유하는 스택형 매니저다.
// raw pointer 없이 화면 전환, 일시정지 팝업, 결과창 같은 구조를 안전하게 쌓고 뺄 수 있다.
class SceneManager
{
public:
    SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void ChangeScene(std::unique_ptr<IScene> scene);
    void PushScene(std::unique_ptr<IScene> scene);
    void PopScene();
    void Clear();

    void Update(float deltaTime);
    void Draw();

    bool Empty() const;
    IScene* CurrentScene();
    const IScene* CurrentScene() const;

private:
    // 앞쪽부터 오래된 씬, 뒤쪽이 현재 활성 씬이다.
    std::vector<std::unique_ptr<IScene>> m_sceneStack;
};
}
