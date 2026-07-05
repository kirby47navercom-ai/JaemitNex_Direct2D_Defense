$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root "build"
$packageRoot = Join-Path $root "package"
$dist = Join-Path $packageRoot "SpaceDefence"
$zip = Join-Path $packageRoot "SpaceDefence.zip"

# FMOD SDK가 설치되어 있으면 자동으로 FMOD 빌드를 사용한다.
# 없을 때는 WinMM fallback으로 빌드되므로 실행 파일은 계속 만들 수 있다.
$fmodDefault = Join-Path ${env:ProgramFiles(x86)} "FMOD SoundSystem\FMOD Studio API Windows"
$fmodHeader = Join-Path $fmodDefault "api\core\inc\fmod.hpp"
$fmodLib = Join-Path $fmodDefault "api\core\lib\x64\fmod_vc.lib"
$cmakeArgs = @("-S", $root, "-B", $build, "-G", "Visual Studio 17 2022", "-A", "x64")
if ((Test-Path $fmodHeader) -and (Test-Path $fmodLib)) {
    $cmakeArgs += @("-DUSE_FMOD=ON", "-DFMOD_SDK_DIR=$fmodDefault")
    Write-Host "FMOD SDK detected: $fmodDefault"
} else {
    $cmakeArgs += @("-DUSE_FMOD=OFF")
    Write-Host "FMOD SDK not found. Building with WinMM fallback."
}

cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE"
}

# 이전 빌드의 object/tlog가 남아 있으면 파일명 변경 뒤에도 예전 오브젝트를 참조할 수 있다.
# 제출 빌드는 항상 같은 조건에서 다시 컴파일되도록 Release 중간 산출물을 지운다.
$releaseObjDir = Join-Path $build "SpaceDefence.dir\Release"
if (Test-Path $releaseObjDir) {
    Get-ChildItem -LiteralPath $releaseObjDir -Filter "*.obj" -ErrorAction SilentlyContinue |
        Remove-Item -Force
    $tlog = Join-Path $releaseObjDir "SpaceDefence.tlog"
    if (Test-Path $tlog) {
        Remove-Item -LiteralPath $tlog -Recurse -Force
    }
}

cmake --build $build --config Release
if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed with exit code $LASTEXITCODE"
}

if (Test-Path $dist) {
    Remove-Item -LiteralPath $dist -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $dist | Out-Null

Copy-Item -LiteralPath (Join-Path $build "Release\SpaceDefence.exe") -Destination $dist
$fmodDll = Join-Path $build "Release\fmod.dll"
if (Test-Path $fmodDll) {
    Copy-Item -LiteralPath $fmodDll -Destination $dist
}

# 빌드 폴더에서도 바로 실행할 수 있도록 런타임 에셋을 복사한다.
if (Test-Path (Join-Path $root "data")) {
    Copy-Item -LiteralPath (Join-Path $root "data") -Destination (Join-Path $build "Release") -Recurse -Force
}
if (Test-Path (Join-Path $root "assets")) {
    Copy-Item -LiteralPath (Join-Path $root "assets") -Destination (Join-Path $build "Release") -Recurse -Force
}

# 제출 zip에는 실행 파일, 에셋, 문서, 소스, 빌드 스크립트를 함께 넣는다.
Copy-Item -LiteralPath (Join-Path $root "README.md") -Destination $dist
Copy-Item -LiteralPath (Join-Path $root "docs") -Destination $dist -Recurse
if (Test-Path (Join-Path $root "tools")) {
    Copy-Item -LiteralPath (Join-Path $root "tools") -Destination $dist -Recurse
}
Copy-Item -LiteralPath (Join-Path $root "CMakeLists.txt") -Destination $dist
Copy-Item -LiteralPath (Join-Path $root "build.ps1") -Destination $dist
Copy-Item -LiteralPath (Join-Path $root "src") -Destination $dist -Recurse
if (Test-Path (Join-Path $root "shaders")) {
    Copy-Item -LiteralPath (Join-Path $root "shaders") -Destination $dist -Recurse
}
if (Test-Path (Join-Path $root "data")) {
    Copy-Item -LiteralPath (Join-Path $root "data") -Destination $dist -Recurse
}
if (Test-Path (Join-Path $root "assets")) {
    Copy-Item -LiteralPath (Join-Path $root "assets") -Destination $dist -Recurse
}

if (Test-Path $zip) {
    Remove-Item -LiteralPath $zip -Force
}
Compress-Archive -Path (Join-Path $dist "*") -DestinationPath $zip -Force

Write-Host "Built: $($dist)\SpaceDefence.exe"
Write-Host "Packaged: $zip"
