$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root "build"
$packageRoot = Join-Path $root "package"
$dist = Join-Path $packageRoot "DawnlineDefense"
$zip = Join-Path $packageRoot "DawnlineDefense.zip"

cmake -S $root -B $build -G "Visual Studio 17 2022" -A x64
cmake --build $build --config Release

if (Test-Path $dist) {
    Remove-Item -LiteralPath $dist -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $dist | Out-Null

Copy-Item -LiteralPath (Join-Path $build "Release\DawnlineDefense.exe") -Destination $dist
Copy-Item -LiteralPath (Join-Path $root "README.md") -Destination $dist
Copy-Item -LiteralPath (Join-Path $root "docs") -Destination $dist -Recurse
Copy-Item -LiteralPath (Join-Path $root "CMakeLists.txt") -Destination $dist
Copy-Item -LiteralPath (Join-Path $root "build.ps1") -Destination $dist
Copy-Item -LiteralPath (Join-Path $root "src") -Destination $dist -Recurse
if (Test-Path (Join-Path $root "shaders")) {
    Copy-Item -LiteralPath (Join-Path $root "shaders") -Destination $dist -Recurse
}

if (Test-Path $zip) {
    Remove-Item -LiteralPath $zip -Force
}
Compress-Archive -Path (Join-Path $dist "*") -DestinationPath $zip -Force

Write-Host "Built: $($dist)\DawnlineDefense.exe"
Write-Host "Packaged: $zip"
