#include "AudioManager.h"

#include <algorithm>
#include <windows.h>
#include <mmsystem.h>

namespace framework
{
void AudioManager::SetVolume(float volume)
{
    m_volume = std::clamp(volume, 0.0f, 1.0f);
}

float AudioManager::Volume() const
{
    return m_volume;
}

bool AudioManager::PlayEffect(const std::wstring& absolutePath) const
{
    if (m_volume <= 0.001f)
    {
        return false;
    }

    // PlaySoundW는 개별 채널 볼륨이 없어서, 재생 직전에 wave 출력 볼륨을 맞춘다.
    const DWORD channelVolume = static_cast<DWORD>(std::clamp(m_volume, 0.0f, 1.0f) * 0xFFFF);
    waveOutSetVolume(nullptr, channelVolume | (channelVolume << 16));
    return PlaySoundW(absolutePath.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT) != FALSE;
}
}
