$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$src = Join-Path $root 'src'
$assets = Join-Path $root 'assets'
$build = Join-Path $root 'build'
New-Item -ItemType Directory -Force -Path $build | Out-Null

Write-Host '[1/3] Compiling resources...'
& windres --codepage=65001 -I $assets -i (Join-Path $assets 'app.rc') -o (Join-Path $build 'app_res.o')
if ($LASTEXITCODE -ne 0) { throw 'windres failed' }

Write-Host '[2/3] Compiling C++ sources...'
$objects = @()
foreach ($name in @('main', 'app', 'render', 'util', 'liquid', 'ui')) {
    $obj = Join-Path $build "$name.o"
    & g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -municode -c (Join-Path $src "$name.cpp") -I $src -o $obj
    if ($LASTEXITCODE -ne 0) { throw "compile failed: $name.cpp" }
    $objects += $obj
}

Write-Host '[3/3] Linking...'
$out = Join-Path $root 'HAVEGET-Whiteboard.exe'
& g++ -std=c++17 -O2 -municode -mwindows -static -static-libgcc -static-libstdc++ `
    $objects (Join-Path $build 'app_res.o') -o $out `
    -lgdiplus -lgdi32 -luser32 -lshell32 -lcomdlg32 -lole32 -lcomctl32 -lshlwapi -luuid -lmsimg32 -lwinpthread -lwinmm
if ($LASTEXITCODE -ne 0) { throw 'link failed' }

Copy-Item -LiteralPath $out -Destination (Join-Path (Split-Path -Parent $root) 'HAVEGET白板.exe') -Force
Write-Host "Done: $out"
