#pragma once

class DeltaTimer
{
public:
    void Reset();
    float Tick();

private:
    long long m_frequency = 0;
    long long m_lastCounter = 0;
};