#include "GameFramework.h"

#include <utility>

namespace framework
{
GameFramework::GameFramework(std::unique_ptr<IGameApplication> application)
    : m_application(std::move(application))
{
}

HRESULT GameFramework::Initialize()
{
    if (!m_application)
    {
        return E_POINTER;
    }
    return m_application->Initialize();
}

int GameFramework::Run()
{
    if (!m_application)
    {
        return -1;
    }
    return m_application->Run();
}
}
