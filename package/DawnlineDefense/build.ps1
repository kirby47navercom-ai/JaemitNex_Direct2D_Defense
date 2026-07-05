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

# build 폴더가 저장소에 남아 있을 때 AudioManager.obj가 소스보다 새 파일로 판단되어
# BGM 함수 구현이 빠진 예전 오브젝트가 링크되는 경우가 있어, 이 파일만 안전하게 다시 만들게 한다.
$staleAudioObj = Join-Path $build "DawnlineDefense.dir\Release\AudioManager.obj"
if (Test-Path $staleAudioObj) {
    Remove-Item -LiteralPath $staleAudioObj -Force
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
