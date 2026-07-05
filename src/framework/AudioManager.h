#pragma once

#include <string>
#include <unordered_map>

#if defined(PAWLINE_WITH_FMOD)
namespace FMOD
{
class Channel;
class Sound;
class System;
}
#endif

namespace framework
{
// 효과음 재생을 한 곳에서 관리한다.
// FMOD SDK가 연결되면 3D 위치음향을 사용하고, 기본 빌드에서는 WinMM으로 좌우 볼륨을 보정한다.
class AudioManager
{
public:
    bool Initialize();
    void Shutdown();
    void Update();

    void SetVolume(float volume);
    float Volume() const;

    bool PlayEffect(const std::wstring& absolutePath) const;
    bool PlayEffectAt(const std::wstring& absolutePath, float worldX, float volumeScale = 1.0f) const;

    bool PlayMusic(const std::wstring& absolutePath, float volume, bool loop = true);
    void StopMusic();
    void SetMusicVolume(float volume);

    void SetListener(float worldX, float audibleWidth);

private:
    float EffectGainFor(const std::wstring& absolutePath) const;
    // 0.0은 무음, 1.0은 최대 볼륨이다.
    float m_volume = 0.78f;
    // 현재 카메라가 듣고 있는 월드 X 좌표다.
    float m_listenerX = 640.0f;
    // 좌우 패닝과 감쇠를 계산할 때 들을 수 있는 전장 폭이다.
    float m_audibleWidth = 1280.0f;
    // WAV 파일마다 다른 녹음 레벨을 한 번 계산해서 재생 볼륨 보정에 사용한다.
    mutable std::unordered_map<std::wstring, float> m_effectGainCache;
    float m_musicVolume = 0.32f;
    bool m_musicOpen = false;

#if defined(PAWLINE_WITH_FMOD)
    // 같은 효과음을 반복 재생할 때 매번 디스크에서 읽지 않도록 FMOD Sound를 캐시한다.
    FMOD::Sound* LoadFmodSound(const std::wstring& absolutePath) const;

    // FMOD Core API 시스템과 로드된 사운드 캐시다.
    mutable FMOD::System* m_fmodSystem = nullptr;
    mutable std::unordered_map<std::wstring, FMOD::Sound*> m_fmodSounds;
    FMOD::Sound* m_musicSound = nullptr;
    FMOD::Channel* m_musicChannel = nullptr;
#endif
};
}
