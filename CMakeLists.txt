cmake_minimum_required(VERSION 3.20)

project(DawnlineDefense LANGUAGES CXX)

add_executable(DawnlineDefense WIN32
    src/main.cpp
    src/PawlineGame.cpp
    src/PawlineGameImplCore.cpp
    src/PawlineGameImplCombat.cpp
    src/PawlineGameImplFeedback.cpp
    src/PawlineGameImplInput.cpp
    src/PawlineGameImplLayout.cpp
    src/PawlineGameImplRender.cpp
    src/GameData.cpp
    src/framework/GameFramework.cpp
    src/framework/DeltaTimer.cpp
    src/framework/SceneManager.cpp
    src/framework/AudioManager.cpp
)

target_compile_features(DawnlineDefense PRIVATE cxx_std_17)
target_compile_definitions(DawnlineDefense PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MSVC)
    target_compile_options(DawnlineDefense PRIVATE /utf-8 /W4 /permissive- /EHsc)
    set_property(TARGET DawnlineDefense PROPERTY VS_GLOBAL_VcpkgEnabled false)
endif()

target_link_libraries(DawnlineDefense PRIVATE
    d2d1
    dwrite
    windowscodecs
    winmm
)
