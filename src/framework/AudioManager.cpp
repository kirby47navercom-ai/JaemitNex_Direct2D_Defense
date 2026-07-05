#include "AudioManager.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <vector>
#include <windows.h>
#include <mmsystem.h>

#if defined(PAWLINE_WITH_FMOD)
#include <fmod.hpp>
#endif

namespace framework
{
namespace
{
constexpr int kFmodChannelBudget = 256;
constexpr wchar_t kMusicAlias[] = L"DawnlineBgm";

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

    // 전투 중 여러 유닛이 동시에 공격해도 효과음이 쉽게 잘리지 않도록 채널 예산을 넉넉히 둔다.
    system->setSoftwareChannels(kFmodChannelBudget);
    if (system->init(kFmodChannelBudget, FMOD_INIT_3D_RIGHTHANDED, nullptr) != FMOD_OK)
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
    StopMusic();

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

bool AudioManager::PlayMusic(const std::wstring& absolutePath, float volume, bool loop)
{
    StopMusic();
    SetMusicVolume(volume);
    if (absolutePath.empty())
    {
        return false;
    }

#if defined(PAWLINE_WITH_FMOD)
    if (m_fmodSystem)
    {
        const std::string utf8Path = ToUtf8(absolutePath);
        const FMOD_MODE mode = FMOD_2D | FMOD_CREATESTREAM | (loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
        if (m_fmodSystem->createSound(utf8Path.c_str(), mode, nullptr, &m_musicSound) != FMOD_OK || !m_musicSound)
        {
            return false;
        }

        if (m_fmodSystem->playSound(m_musicSound, nullptr, false, &m_musicChannel) != FMOD_OK)
        {
            m_musicSound->release();
            m_musicSound = nullptr;
            m_musicChannel = nullptr;
            return false;
        }
        if (m_musicChannel)
        {
            m_musicChannel->setVolume(m_musicVolume);
        }
        return true;
    }
#endif

    const std::wstring open = L"open \"" + absolutePath + L"\" type mpegvideo alias " + kMusicAlias;
    if (mciSendStringW(open.c_str(), nullptr, 0, nullptr) != 0)
    {
        const std::wstring retry = L"open \"" + absolutePath + L"\" alias " + kMusicAlias;
        if (mciSendStringW(retry.c_str(), nullptr, 0, nullptr) != 0)
        {
            return false;
        }
    }

    m_musicOpen = true;
    SetMusicVolume(m_musicVolume);
    const std::wstring play = std::wstring(L"play ") + kMusicAlias + (loop ? L" repeat" : L"");
    return mciSendStringW(play.c_str(), nullptr, 0, nullptr) == 0;
}

void AudioManager::StopMusic()
{
#if defined(PAWLINE_WITH_FMOD)
    if (m_musicChannel)
    {
        m_musicChannel->stop();
        m_musicChannel = nullptr;
    }
    if (m_musicSound)
    {
        m_musicSound->release();
        m_musicSound = nullptr;
    }
#endif

    if (m_musicOpen)
    {
        const std::wstring stop = std::wstring(L"stop ") + kMusicAlias;
        const std::wstring close = std::wstring(L"close ") + kMusicAlias;
        mciSendStringW(stop.c_str(), nullptr, 0, nullptr);
        mciSendStringW(close.c_str(), nullptr, 0, nullptr);
        m_musicOpen = false;
    }
}

void AudioManager::SetMusicVolume(float volume)
{
    m_musicVolume = std::clamp(volume, 0.0f, 1.0f);

#if defined(PAWLINE_WITH_FMOD)
    if (m_musicChannel)
    {
        m_musicChannel->setVolume(m_musicVolume);
    }
#endif

    if (m_musicOpen)
    {
        const int mciVolume = static_cast<int>(std::round(m_musicVolume * 1000.0f));
        const std::wstring command = std::wstring(L"setaudio ") + kMusicAlias + L" volume to " + std::to_wstring(mciVolume);
        mciSendStringW(command.c_str(), nullptr, 0, nullptr);
    }
}

float AudioManager::EffectGainFor(const std::wstring& absolutePath) const
{
    const auto cached = m_effectGainCache.find(absolutePath);
    if (cached != m_effectGainCache.end())
    {
        return cached->second;
    }

    float gain = 1.0f;
    std::ifstream file(absolutePath, std::ios::binary);
    if (file)
    {
        char riff[4] = {};
        char wave[4] = {};
        std::uint32_t riffSize = 0;
        file.read(riff, 4);
        file.read(reinterpret_cast<char*>(&riffSize), sizeof(riffSize));
        file.read(wave, 4);

        std::uint16_t formatTag = 0;
        std::uint16_t bitsPerSample = 0;
        std::vector<char> sampleData;

        while (file)
        {
            char chunkId[4] = {};
            std::uint32_t chunkSize = 0;
            file.read(chunkId, 4);
            file.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
            if (!file)
            {
                break;
            }

            const std::string id(chunkId, chunkId + 4);
            const std::streamoff paddedSize = static_cast<std::streamoff>(chunkSize + (chunkSize & 1U));
            const std::streampos nextChunk = file.tellg() + paddedSize;
            if (id == "fmt " && chunkSize >= 16)
            {
                std::uint16_t channelCount = 0;
                std::uint32_t sampleRate = 0;
                std::uint32_t byteRate = 0;
                std::uint16_t blockAlign = 0;
                file.read(reinterpret_cast<char*>(&formatTag), sizeof(formatTag));
                file.read(reinterpret_cast<char*>(&channelCount), sizeof(channelCount));
                file.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
                file.read(reinterpret_cast<char*>(&byteRate), sizeof(byteRate));
                file.read(reinterpret_cast<char*>(&blockAlign), sizeof(blockAlign));
                file.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));
            }
            else if (id == "data" && chunkSize > 0)
            {
                sampleData.resize(chunkSize);
                file.read(sampleData.data(), static_cast<std::streamsize>(sampleData.size()));
            }
            file.seekg(nextChunk);
        }

        if (formatTag == 1 && !sampleData.empty())
        {
            double sumSquares = 0.0;
            double peak = 0.0;
            std::size_t activeSamples = 0;

            if (bitsPerSample == 16)
            {
                for (std::size_t i = 0; i + 1 < sampleData.size(); i += 2)
                {
                    const auto lo = static_cast<unsigned char>(sampleData[i]);
                    const auto hi = static_cast<unsigned char>(sampleData[i + 1]);
                    const auto sample = static_cast<std::int16_t>(static_cast<std::uint16_t>(lo | (hi << 8)));
                    const double normalized = std::abs(static_cast<double>(sample) / 32768.0);
                    if (normalized > 0.003)
                    {
                        sumSquares += normalized * normalized;
                        peak = std::max(peak, normalized);
                        ++activeSamples;
                    }
                }
            }
            else if (bitsPerSample == 8)
            {
                for (char raw : sampleData)
                {
                    const double normalized = std::abs((static_cast<int>(static_cast<unsigned char>(raw)) - 128) / 128.0);
                    if (normalized > 0.003)
                    {
                        sumSquares += normalized * normalized;
                        peak = std::max(peak, normalized);
                        ++activeSamples;
                    }
                }
            }

            if (activeSamples > 0)
            {
                const double rms = std::sqrt(sumSquares / static_cast<double>(activeSamples));
                const double loudness = std::max(rms, peak * 0.18);
                if (loudness > 0.001)
                {
                    gain = std::clamp(static_cast<float>(0.145 / loudness), 0.42f, 1.85f);
                }
            }
        }
    }

    m_effectGainCache.emplace(absolutePath, gain);
    return gain;
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
            channel->setMode(FMOD_2D);
            channel->setVolume(std::clamp(m_volume * EffectGainFor(absolutePath), 0.0f, 1.0f));
        }
        return true;
    }
#endif

    // PlaySoundW는 개별 채널 볼륨이 없어서, 재생 직전에 wave 출력 볼륨을 맞춘다.
    const DWORD channelVolume = static_cast<DWORD>(std::clamp(m_volume * EffectGainFor(absolutePath), 0.0f, 1.0f) * 0xFFFF);
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
    const float attenuation = std::clamp(1.0f - distance * 0.48f, 0.34f, 1.0f) * std::clamp(volumeScale, 0.0f, 1.8f);
    const float fileGain = EffectGainFor(absolutePath);

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
            channel->setMode(FMOD_3D);
            FMOD_VECTOR position = {worldX / 96.0f, 0.0f, 0.0f};
            FMOD_VECTOR velocity = {0.0f, 0.0f, 0.0f};
            channel->set3DAttributes(&position, &velocity);
            channel->set3DMinMaxDistance(0.9f, 18.0f);
            channel->setVolume(std::clamp(m_volume * attenuation * fileGain, 0.0f, 1.0f));
            channel->setPaused(false);
        }
        return true;
    }
#endif

    const float pan = normalized;
    const float left = std::clamp(attenuation * (pan > 0.0f ? 1.0f - pan * 0.72f : 1.0f), 0.0f, 1.0f);
    const float right = std::clamp(attenuation * (pan < 0.0f ? 1.0f + pan * 0.72f : 1.0f), 0.0f, 1.0f);
    const DWORD leftVolume = static_cast<DWORD>(std::clamp(m_volume * left * fileGain, 0.0f, 1.0f) * 0xFFFF);
    const DWORD rightVolume = static_cast<DWORD>(std::clamp(m_volume * right * fileGain, 0.0f, 1.0f) * 0xFFFF);
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
    const FMOD_MODE mode = FMOD_3D | FMOD_CREATESAMPLE;
    if (m_fmodSystem->createSound(utf8Path.c_str(), mode, nullptr, &sound) != FMOD_OK)
    {
        return nullptr;
    }

    m_fmodSounds.emplace(absolutePath, sound);
    return sound;
}
#endif
}
