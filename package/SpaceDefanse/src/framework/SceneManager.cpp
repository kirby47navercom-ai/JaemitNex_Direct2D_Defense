#include "SceneManager.h"

namespace framework
{
void SceneManager::ChangeScene(std::unique_ptr<IScene> scene)
{
    Clear();
    if (!scene)
    {
        return;
    }

    scene->Init();
    m_sceneStack.push_back(std::move(scene));
}

void SceneManager::PushScene(std::unique_ptr<IScene> scene)
{
    if (!scene)
    {
        return;
    }

    if (!m_sceneStack.empty())
    {
        m_sceneStack.back()->OnPause();
    }

    scene->Init();
    m_sceneStack.push_back(std::move(scene));
}

void SceneManager::PopScene()
{
    if (m_sceneStack.empty())
    {
        return;
    }

    m_sceneStack.back()->Finish();
    m_sceneStack.pop_back();

    if (!m_sceneStack.empty())
    {
        m_sceneStack.back()->OnResume();
    }
}

void SceneManager::Clear()
{
    while (!m_sceneStack.empty())
    {
        m_sceneStack.back()->Finish();
        m_sceneStack.pop_back();
    }
}

void SceneManager::Update(float deltaTime)
{
    if (!m_sceneStack.empty())
    {
        m_sceneStack.back()->Update(deltaTime);
    }
}

void SceneManager::Draw()
{
    if (!m_sceneStack.empty())
    {
        m_sceneStack.back()->Draw();
    }
}

bool SceneManager::Empty() const
{
    return m_sceneStack.empty();
}

IScene* SceneManager::CurrentScene()
{
    return m_sceneStack.empty() ? nullptr : m_sceneStack.back().get();
}

const IScene* SceneManager::CurrentScene() const
{
    return m_sceneStack.empty() ? nullptr : m_sceneStack.back().get();
}
}
