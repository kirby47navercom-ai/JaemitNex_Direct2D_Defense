#include "AudioManager.h"

#include <algorithm>
#include <cmath>
#include <windows.h>
#include <mmsystem.h>

#if defined(PAWLINE_WITH_FMOD)
#include <fmod.hpp>
#endif

namespace framework
{
namespace
{
std::string ToUtf8(const std::wstring& text)
{
    if (text.empty())
    {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0)
    {
        return {};
    }

    std::string result(static_cast<size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, result.data(), size, nullptr, nullptr);
    return result;
}
}

bool AudioManager::Initialize()
{
#if defined(PAWLINE_WITH_FMOD)
    if (m_fmodSystem)
    {
        return true;
    }

    FMOD::System* system = nullptr;
    if (FMOD::System_Create(&system) != FMOD_OK || !system)
    {
        return false;
    }

    if (system->init(96, FMOD_INIT_3D_RIGHTHANDED, nullptr) != FMOD_OK)
    {
        system->release();
        return false;
    }

    system->set3DSettings(1.0f, 96.0f, 1.0f);
    m_fmodSystem = system;
#endif
    return true;
}

void AudioManager::Shutdown()
{
#if defined(PAWLINE_WITH_FMOD)
    for (auto& [path, sound] : m_fmodSounds)
    {
        if (sound)
        {
            sound->release();
        }
    }
    m_fmodSounds.clear();

    if (m_fmodSystem)
    {
        m_fmodSystem->close();
        m_fmodSystem->release();
        m_fmodSystem = nullptr;
    }
#endif
}

void AudioManager::Update()
{
#if defined(PAWLINE_WITH_FMOD)
    if (!m_fmodSystem)
    {
        return;
    }

    FMOD_VECTOR position = {m_listenerX / 96.0f, 0.0f, 0.0f};
    FMOD_VECTOR velocity = {0.0f, 0.0f, 0.0f};
    FMOD_VECTOR forward = {0.0f, 0.0f, 1.0f};
    FMOD_VECTOR up = {0.0f, 1.0f, 0.0f};
    m_fmodSystem->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    m_fmodSystem->update();
#endif
}

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

#if defined(PAWLINE_WITH_FMOD)
    if (m_fmodSystem)
    {
        FMOD::Sound* sound = LoadFmodSound(absolutePath);
        if (!sound)
        {
            return false;
        }

        FMOD::Channel* channel = nullptr;
        if (m_fmodSystem->playSound(sound, nullptr, false, &channel) != FMOD_OK)
        {
            return false;
        }
        if (channel)
        {
            channel->setVolume(m_volume);
        }
        return true;
    }
#endif

    // PlaySoundW는 개별 채널 볼륨이 없어서, 재생 직전에 wave 출력 볼륨을 맞춘다.
    const DWORD channelVolume = static_cast<DWORD>(std::clamp(m_volume, 0.0f, 1.0f) * 0xFFFF);
    waveOutSetVolume(nullptr, channelVolume | (channelVolume << 16));
    return PlaySoundW(absolutePath.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT) != FALSE;
}

bool AudioManager::PlayEffectAt(const std::wstring& absolutePath, float worldX, float volumeScale) const
{
    if (m_volume <= 0.001f || volumeScale <= 0.001f)
    {
        return false;
    }

    const float halfWidth = std::max(1.0f, m_audibleWidth * 0.5f);
    const float normalized = std::clamp((worldX - m_listenerX) / halfWidth, -1.0f, 1.0f);
    const float distance = std::abs(normalized);
    const float attenuation = std::clamp(1.0f - distance * 0.58f, 0.24f, 1.0f) * std::clamp(volumeScale, 0.0f, 1.6f);

#if defined(PAWLINE_WITH_FMOD)
    if (m_fmodSystem)
    {
        FMOD::Sound* sound = LoadFmodSound(absolutePath);
        if (!sound)
        {
            return false;
        }

        FMOD::Channel* channel = nullptr;
        if (m_fmodSystem->playSound(sound, nullptr, true, &channel) != FMOD_OK)
        {
            return false;
        }
        if (channel)
        {
            FMOD_VECTOR position = {worldX / 96.0f, 0.0f, 0.0f};
            FMOD_VECTOR velocity = {0.0f, 0.0f, 0.0f};
            channel->set3DAttributes(&position, &velocity);
            channel->set3DMinMaxDistance(0.9f, 18.0f);
            channel->setVolume(m_volume * attenuation);
            channel->setPaused(false);
        }
        return true;
    }
#endif

    const float pan = normalized;
    const float left = std::clamp(attenuation * (pan > 0.0f ? 1.0f - pan * 0.72f : 1.0f), 0.0f, 1.0f);
    const float right = std::clamp(attenuation * (pan < 0.0f ? 1.0f + pan * 0.72f : 1.0f), 0.0f, 1.0f);
    const DWORD leftVolume = static_cast<DWORD>(std::clamp(m_volume * left, 0.0f, 1.0f) * 0xFFFF);
    const DWORD rightVolume = static_cast<DWORD>(std::clamp(m_volume * right, 0.0f, 1.0f) * 0xFFFF);
    waveOutSetVolume(nullptr, leftVolume | (rightVolume << 16));
    return PlaySoundW(absolutePath.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT) != FALSE;
}

void AudioManager::SetListener(float worldX, float audibleWidth)
{
    m_listenerX = worldX;
    m_audibleWidth = std::max(320.0f, audibleWidth);
}

#if defined(PAWLINE_WITH_FMOD)
FMOD::Sound* AudioManager::LoadFmodSound(const std::wstring& absolutePath) const
{
    const auto found = m_fmodSounds.find(absolutePath);
    if (found != m_fmodSounds.end())
    {
        return found->second;
    }

    if (!m_fmodSystem)
    {
        return nullptr;
    }

    FMOD::Sound* sound = nullptr;
    const std::string utf8Path = ToUtf8(absolutePath);
    const FMOD_MODE mode = FMOD_3D | FMOD_CREATECOMPRESSEDSAMPLE;
    if (m_fmodSystem->createSound(utf8Path.c_str(), mode, nullptr, &sound) != FMOD_OK)
    {
        return nullptr;
    }

    m_fmodSounds.emplace(absolutePath, sound);
    return sound;
}
#endif
}
