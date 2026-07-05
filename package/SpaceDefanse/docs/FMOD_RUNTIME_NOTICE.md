# FMOD Runtime Notice

이 프로젝트는 로컬 PC에 설치된 FMOD Studio API Windows SDK를 감지하면 FMOD Core API로 빌드합니다.

- 기본 감지 경로: `C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows`
- 빌드 옵션: `-DUSE_FMOD=ON -DFMOD_SDK_DIR=...`
- 패키지 포함 파일: 빌드 결과 폴더의 `fmod.dll`

FMOD 공식 정책상 SDK 설치파일과 전체 SDK는 저장소에 재배포하지 않습니다. 게임 실행에 필요한 FMOD Engine 런타임 DLL만 제품 패키지에 포함합니다.

공식 다운로드와 라이선스 확인:

- https://www.fmod.com/download
- https://www.fmod.com/legal
- https://www.fmod.com/licensing
