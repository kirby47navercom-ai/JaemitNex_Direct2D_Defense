cmake_minimum_required(VERSION 3.20)

project(SpaceDefanse LANGUAGES CXX)

option(USE_FMOD "Build with FMOD Core API when FMOD_SDK_DIR is provided" OFF)
set(FMOD_SDK_DIR "" CACHE PATH "FMOD Engine SDK root directory")

add_executable(SpaceDefanse WIN32
    src/main.cpp
    src/SpaceDefanseGame.cpp
    src/SpaceDefanseGameImplCore.cpp
    src/SpaceDefanseGameImplCombat.cpp
    src/SpaceDefanseGameImplFeedback.cpp
    src/SpaceDefanseGameImplInput.cpp
    src/SpaceDefanseGameImplLayout.cpp
    src/SpaceDefanseGameImplRender.cpp
    src/GameData.cpp
    src/framework/GameFramework.cpp
    src/framework/DeltaTimer.cpp
    src/framework/SceneManager.cpp
    src/framework/AudioManager.cpp
)

target_compile_features(SpaceDefanse PRIVATE cxx_std_17)
target_compile_definitions(SpaceDefanse PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MSVC)
    target_compile_options(SpaceDefanse PRIVATE /utf-8 /W4 /permissive- /EHsc)
    set_property(TARGET SpaceDefanse PROPERTY VS_GLOBAL_VcpkgEnabled false)
endif()

target_link_libraries(SpaceDefanse PRIVATE
    d2d1
    dwrite
    windowscodecs
    winmm
)

if(USE_FMOD)
    if(NOT FMOD_SDK_DIR)
        message(FATAL_ERROR "USE_FMOD is ON, but FMOD_SDK_DIR is empty.")
    endif()

    set(FMOD_CORE_INC "${FMOD_SDK_DIR}/api/core/inc")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FMOD_CORE_LIB "${FMOD_SDK_DIR}/api/core/lib/x64/fmod_vc.lib")
        set(FMOD_CORE_DLL "${FMOD_SDK_DIR}/api/core/lib/x64/fmod.dll")
    else()
        set(FMOD_CORE_LIB "${FMOD_SDK_DIR}/api/core/lib/x86/fmod_vc.lib")
        set(FMOD_CORE_DLL "${FMOD_SDK_DIR}/api/core/lib/x86/fmod.dll")
    endif()

    if(NOT EXISTS "${FMOD_CORE_INC}/fmod.hpp" OR NOT EXISTS "${FMOD_CORE_LIB}")
        message(FATAL_ERROR "FMOD SDK files were not found under FMOD_SDK_DIR.")
    endif()

    target_compile_definitions(SpaceDefanse PRIVATE SPACE_DEFANSE_WITH_FMOD)
    target_include_directories(SpaceDefanse PRIVATE "${FMOD_CORE_INC}")
    target_link_libraries(SpaceDefanse PRIVATE "${FMOD_CORE_LIB}")
    if(EXISTS "${FMOD_CORE_DLL}")
        add_custom_command(TARGET SpaceDefanse POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${FMOD_CORE_DLL}"
                    "$<TARGET_FILE_DIR:SpaceDefanse>/fmod.dll")
    endif()
endif()
