$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root "build"
$packageRoot = Join-Path $root "package"
$dist = Join-Path $packageRoot "DawnlineDefense"
$zip = Join-Path $packageRoot "DawnlineDefense.zip"

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

# build 폴더가 저장소에 남아 있으면 예전 .obj와 새 헤더가 섞여 접근 위반이 날 수 있다.
# 제출 빌드는 안정성이 우선이라, 대상 오브젝트와 tlog를 지워 매번 같은 상태에서 다시 컴파일한다.
$releaseObjDir = Join-Path $build "DawnlineDefense.dir\Release"
if (Test-Path $releaseObjDir) {
    Get-ChildItem -LiteralPath $releaseObjDir -Filter "*.obj" -ErrorAction SilentlyContinue |
        Remove-Item -Force
    $tlog = Join-Path $releaseObjDir "DawnlineDefense.tlog"
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

Copy-Item -LiteralPath (Join-Path $build "Release\DawnlineDefense.exe") -Destination $dist
$fmodDll = Join-Path $build "Release\fmod.dll"
if (Test-Path $fmodDll) {
    Copy-Item -LiteralPath $fmodDll -Destination $dist
}
if (Test-Path (Join-Path $root "data")) {
    Copy-Item -LiteralPath (Join-Path $root "data") -Destination (Join-Path $build "Release") -Recurse -Force
}
if (Test-Path (Join-Path $root "assets")) {
    Copy-Item -LiteralPath (Join-Path $root "assets") -Destination (Join-Path $build "Release") -Recurse -Force
}
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

Write-Host "Built: $($dist)\DawnlineDefense.exe"
Write-Host "Packaged: $zip"
