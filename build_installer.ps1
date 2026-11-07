$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$src = Join-Path $root 'installer'
$build = Join-Path $root 'build\installer'
$release = 'C:\Users\15797\Documents\deepseek-harness\default-workspace\HAVEGET-白板-发布'
New-Item -ItemType Directory -Force -Path $build | Out-Null
New-Item -ItemType Directory -Force -Path $src | Out-Null

$app = Join-Path $release 'HAVEGET白板.exe'
if (-not (Test-Path -LiteralPath $app)) { throw '缺少已签名 HAVEGET白板.exe' }
Copy-Item -LiteralPath $app -Destination (Join-Path $src 'payload_app.bin') -Force
Copy-Item -LiteralPath (Join-Path $release 'certs\HAVEGET-RootCA.cer') -Destination (Join-Path $src 'root.cer') -Force
Copy-Item -LiteralPath (Join-Path $release 'certs\HAVEGET-CodeSigning.cer') -Destination (Join-Path $src 'leaf.cer') -Force
Copy-Item -LiteralPath (Join-Path $root 'assets\app.ico') -Destination (Join-Path $src 'app.ico') -Force

Write-Host '[1/2] Compiling installer resources...'
& windres --codepage=65001 -I $src -i (Join-Path $src 'installer.rc') -o (Join-Path $build 'installer_res.o')
if ($LASTEXITCODE -ne 0) { throw 'windres failed' }

Write-Host '[2/2] Compiling and linking installer...'
$objs = @()
foreach ($name in @('installer', 'util')) {
    $obj = Join-Path $build "$name.o"
    $cpp = if ($name -eq 'installer') { Join-Path $src 'installer.cpp' } else { Join-Path $root 'src\util.cpp' }
    & g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -municode -c $cpp -I (Join-Path $root 'src') -I $src -o $obj
    if ($LASTEXITCODE -ne 0) { throw "compile failed: $name.cpp" }
    $objs += $obj
}
$out = Join-Path $root 'HAVEGET-Setup.exe'
& g++ -std=c++17 -O2 -municode -mwindows -static -static-libgcc -static-libstdc++ `
    $objs (Join-Path $build 'installer_res.o') -o $out `
    -lgdiplus -lgdi32 -luser32 -lshell32 -lcomdlg32 -lole32 -lcomctl32 -lshlwapi -luuid -lmsimg32 -lwinpthread `
    -ldwmapi -ladvapi32
if ($LASTEXITCODE -ne 0) { throw 'link failed' }
Copy-Item -LiteralPath $out -Destination (Join-Path $release 'HAVEGET白板安装器.exe') -Force
Write-Host "Done: $out"
