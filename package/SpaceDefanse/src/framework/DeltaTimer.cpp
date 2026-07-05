#include "DeltaTimer.h"

#include <windows.h>

void DeltaTimer::Reset()
{
    LARGE_INTEGER frequency = {};
    LARGE_INTEGER counter = {};
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    m_frequency = frequency.QuadPart;
    m_lastCounter = counter.QuadPart;
}

float DeltaTimer::Tick()
{
    if (m_frequency == 0 || m_lastCounter == 0)
    {
        Reset();
        return 0.0f;
    }

    LARGE_INTEGER now = {};
    QueryPerformanceCounter(&now);
    const float deltaTime = static_cast<float>(static_cast<double>(now.QuadPart - m_lastCounter) /
                                               static_cast<double>(m_frequency));
    m_lastCounter = now.QuadPart;
    return deltaTime;
}