param(
    [string]$OutDir = (Join-Path $PSScriptRoot '..\selfsigned'),
    [switch]$NoTimestamp
)
$ErrorActionPreference = 'Stop'
$OutDir = [System.IO.Path]::GetFullPath($OutDir)
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$exe = Join-Path $OutDir '轻画板.exe'
if (-not (Test-Path -LiteralPath $exe)) {
    throw "缺少待签名文件：$exe"
}

Write-Host '创建 HAVEGET 根证书 ...'
$root = New-SelfSignedCertificate `
    -Type Custom `
    -Subject 'CN=HAVEGET Studio Root CA, O=HAVEGET工作室' `
    -KeyUsage CertSign, CRLSign `
    -KeyLength 4096 `
    -HashAlgorithm SHA256 `
    -NotAfter (Get-Date).AddYears(15) `
    -CertStoreLocation Cert:\CurrentUser\My `
    -TextExtension @('2.5.29.19={text}CA=true&pathlength=1')

Write-Host '创建 HAVEGET 代码签名证书 ...'
$leaf = New-SelfSignedCertificate `
    -Type CodeSigningCert `
    -Subject 'CN=HAVEGET工作室, O=HAVEGET工作室' `
    -Signer $root `
    -KeyUsage DigitalSignature `
    -KeyLength 3072 `
    -HashAlgorithm SHA256 `
    -NotAfter (Get-Date).AddYears(10) `
    -CertStoreLocation Cert:\CurrentUser\My `
    -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3')

$rootCer = Join-Path $OutDir 'HAVEGET-RootCA.cer'
$leafCer = Join-Path $OutDir 'HAVEGET-CodeSigning.cer'
Export-Certificate -Cert $root -FilePath $rootCer -Type CERT | Out-Null
Export-Certificate -Cert $leaf -FilePath $leafCer -Type CERT | Out-Null
Write-Host "根证书: $($root.Thumbprint)"
Write-Host "代码签名证书: $($leaf.Thumbprint)"

Write-Host '对 EXE 签名 ...'
$signed = $null
$timestampUsed = $false
if (-not $NoTimestamp) {
    try {
        $signed = Set-AuthenticodeSignature -FilePath $exe -Certificate $leaf `
            -TimestampServer 'http://timestamp.digicert.com' -HashAlgorithm SHA256
        if ($signed.TimeStamperCertificate) {
            $timestampUsed = $true
            Write-Host '时间戳签名成功。'
        } else {
            Write-Warning '未获得时间戳证书。'
        }
    } catch {
        Write-Warning "带时间戳签名失败：$($_.Exception.Message)"
    }
}
if (-not $signed -or (-not $signed.SignerCertificate)) {
    Write-Host '不带时间戳签名 ...'
    $signed = Set-AuthenticodeSignature -FilePath $exe -Certificate $leaf -HashAlgorithm SHA256
}

Write-Host '安装当前用户信任证书 ...'
& certutil.exe -user -addstore -f Root $rootCer | Out-Null
if ($LASTEXITCODE -ne 0) { throw '导入根证书失败' }
& certutil.exe -user -addstore -f TrustedPublisher $leafCer | Out-Null
if ($LASTEXITCODE -ne 0) { throw '导入受信任发布者失败' }

$sig = Get-AuthenticodeSignature -LiteralPath $exe
Write-Host '--- 签名验证 ---'
$sig | Format-List Status, StatusMessage, SignerCertificate, TimeStamperCertificate

@(
    "RootSubject=$($root.Subject)",
    "RootThumbprint=$($root.Thumbprint)",
    "LeafSubject=$($leaf.Subject)",
    "LeafThumbprint=$($leaf.Thumbprint)",
    "TimestampUsed=$timestampUsed",
    "SignatureStatus=$($sig.Status)"
) | Set-Content -LiteralPath (Join-Path $OutDir 'cert-info.txt') -Encoding UTF8

if ($sig.Status -ne 'Valid') {
    Write-Warning "签名状态不是 Valid：$($sig.Status)。证书信任未生效或时间戳/证书链有问题。"
    exit 2
}
Write-Host '签名成功且证书链受信任。' -ForegroundColor Green
