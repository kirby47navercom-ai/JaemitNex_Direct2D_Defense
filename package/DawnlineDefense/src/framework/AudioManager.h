#pragma once

#include <string>

namespace framework
{
// WinMM 기반의 가벼운 효과음 매니저다.
// 지금 프로젝트는 Direct2D 단일 실행 파일 구조라, 외부 런타임 없이 WAV를 바로 재생하는 쪽을 선택했다.
class AudioManager
{
public:
    void SetVolume(float volume);
    float Volume() const;

    bool PlayEffect(const std::wstring& absolutePath) const;

private:
    // 0.0은 무음, 1.0은 최대 볼륨이다.
    float m_volume = 0.78f;
};
}
