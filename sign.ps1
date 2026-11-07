[CmdletBinding(DefaultParameterSetName = 'Thumbprint')]
param(
    [string]$Exe,
    [Parameter(ParameterSetName = 'Thumbprint', Mandatory = $true)]
    [string]$Thumbprint,
    [Parameter(ParameterSetName = 'Pfx', Mandatory = $true)]
    [string]$Pfx,
    [string]$TimestampUrl = 'http://timestamp.digicert.com',
    [switch]$KeepImported
)

$ErrorActionPreference = 'Stop'
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $Exe) {
    $Exe = Join-Path (Split-Path -Parent $scriptDir) '轻画板.exe'
}
if (-not (Test-Path -LiteralPath $Exe)) {
    throw "找不到待签名 EXE：$Exe"
}
$Exe = (Resolve-Path -LiteralPath $Exe).Path

$importedCert = $null
try {
    if ($Pfx) {
        if (-not (Test-Path -LiteralPath $Pfx)) {
            throw "找不到 PFX：$Pfx"
        }
        $Pfx = (Resolve-Path -LiteralPath $Pfx).Path
        $secure = Read-Host -AsSecureString "请输入 PFX 密码"
        Write-Host "正在导入证书到 CurrentUser\My ..."
        $importedCert = Import-PfxCertificate -FilePath $Pfx `
            -CertStoreLocation Cert:\CurrentUser\My -Password $secure
        $Thumbprint = $importedCert.Thumbprint
    }

    if (-not $Thumbprint) {
        throw "请通过 -Thumbprint 或 -Pfx 指定代码签名证书。"
    }
    $Thumbprint = ($Thumbprint -replace '\s', '').ToUpperInvariant()
    $cert = Get-ChildItem Cert:\CurrentUser\My |
        Where-Object { $_.Thumbprint -eq $Thumbprint } |
        Select-Object -First 1
    if (-not $cert) {
        throw "当前用户证书存储中找不到指纹 $Thumbprint。"
    }
    if (-not $cert.HasPrivateKey) {
        throw "证书没有私钥，无法签名。EV 证书请确认 token 已插入并解锁。"
    }

    $oids = @($cert.EnhancedKeyUsageList | ForEach-Object { $_.ObjectId })
    if ($oids -notcontains '1.3.6.1.5.5.7.3.3') {
        throw "这不是代码签名证书（缺少 Code Signing EKU）。"
    }

    Write-Host ""
    Write-Host "证书主题: $($cert.Subject)"
    Write-Host "签发者:    $($cert.Issuer)"
    Write-Host "有效期至:  $($cert.NotAfter)"
    if ($cert.Subject -notmatch 'HAVEGET') {
        Write-Warning "证书主题中没有 HAVEGET；SmartScreen 会显示证书里的实际名称。"
    }

    # 优先使用 signtool（Windows SDK）；找不到时回退到 PowerShell Authenticode。
    $signtool = $null
    $cmd = Get-Command signtool.exe -ErrorAction SilentlyContinue
    if ($cmd) {
        $signtool = $cmd.Source
    } else {
        $kitRoots = @(
            "${env:ProgramFiles(x86)}\Windows Kits\10\bin",
            "$env:ProgramFiles\Windows Kits\10\bin"
        ) | Where-Object { Test-Path -LiteralPath $_ }
        foreach ($root in $kitRoots) {
            $candidate = Get-ChildItem -LiteralPath $root -Recurse -Filter signtool.exe `
                -ErrorAction SilentlyContinue |
                Where-Object { $_.FullName -match '\\x64\\' } |
                Sort-Object FullName -Descending |
                Select-Object -First 1
            if ($candidate) {
                $signtool = $candidate.FullName
                break
            }
        }
    }

    if ($signtool) {
        Write-Host "使用 signtool: $signtool"
        & $signtool sign /sha1 $Thumbprint /fd SHA256 /td SHA256 /tr $TimestampUrl $Exe
        if ($LASTEXITCODE -ne 0) {
            throw "signtool 签名失败（退出码 $LASTEXITCODE）。请检查证书/时间戳网络。"
        }
        & $signtool verify /pa /v $Exe
        if ($LASTEXITCODE -ne 0) {
            throw "signtool 验证失败（退出码 $LASTEXITCODE）。"
        }
    } else {
        Write-Host "未找到 signtool，使用 PowerShell Set-AuthenticodeSignature。"
        $result = Set-AuthenticodeSignature -FilePath $Exe -Certificate $cert `
            -TimestampServer $TimestampUrl -HashAlgorithm SHA256
        $result | Format-List Status, StatusMessage
        if ($result.Status -ne 'Valid') {
            throw "签名验证失败：$($result.StatusMessage)"
        }
    }

    $sig = Get-AuthenticodeSignature -LiteralPath $Exe
    $sig | Format-List Status, StatusMessage
    $sig.SignerCertificate |
        Format-List Subject, Issuer, Thumbprint, NotAfter

    if ($sig.Status -ne 'Valid') {
        throw "签名状态不是 Valid，SmartScreen 仍可能提示：$($sig.Status)"
    }

    Write-Host ""
    Write-Host "签名成功: $Exe" -ForegroundColor Green
    Write-Host "发布者证书主题: $($sig.SignerCertificate.Subject)" -ForegroundColor Green
    Write-Host "下一步：在干净虚拟机中下载此 EXE，确认 SmartScreen 不再拦截。" -ForegroundColor Yellow
} finally {
    if ($importedCert -and -not $KeepImported) {
        $thumb = $importedCert.Thumbprint
        Remove-Item -LiteralPath "Cert:\CurrentUser\My\$thumb" -Force -ErrorAction SilentlyContinue
        Write-Host "已从当前用户证书存储移除本次导入的证书（私钥不会被导出）。"
    }
}
