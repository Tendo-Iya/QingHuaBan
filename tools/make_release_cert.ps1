$ErrorActionPreference = 'Stop'
$rootDir = 'C:\Users\15797\Documents\deepseek-harness\default-workspace'
$release = Join-Path $rootDir 'HAVEGET-白板-发布'
New-Item -ItemType Directory -Force -Path $release | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $release 'certs') | Out-Null

$appSrc = Join-Path $rootDir 'QingHuaBan\HAVEGET-Whiteboard.exe'
$appOut = Join-Path $release 'HAVEGET白板.exe'
Get-Process HAVEGET-Whiteboard,HAVEGET白板 -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500
Copy-Item -LiteralPath $appSrc -Destination $appOut -Force

Write-Host '创建 HAVEGET 根证书 ...'
$root = New-SelfSignedCertificate `
    -Type Custom `
    -Subject 'CN=HAVEGET Studio Root CA, O=HAVEGET工作室' `
    -KeyUsage CertSign, CRLSign `
    -KeyLength 4096 `
    -HashAlgorithm SHA256 `
    -NotAfter (Get-Date).AddYears(15) `
    -CertStoreLocation Cert:\CurrentUser\My `
    -TextExtension @("2.5.29.19={text}CA=true`&pathlength=1")

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

$rootCer = Join-Path $release 'certs\HAVEGET-RootCA.cer'
$leafCer = Join-Path $release 'certs\HAVEGET-CodeSigning.cer'
Export-Certificate -Cert $root -FilePath $rootCer -Type CERT | Out-Null
Export-Certificate -Cert $leaf -FilePath $leafCer -Type CERT | Out-Null
Write-Host "根证书: $($root.Thumbprint)"
Write-Host "代码签名证书: $($leaf.Thumbprint)"

Write-Host '对 HAVEGET白板.exe 签名 ...'
$signed = $null
$timestampUsed = $false
try {
    $signed = Set-AuthenticodeSignature -FilePath $appOut -Certificate $leaf `
        -TimestampServer 'http://timestamp.digicert.com' -HashAlgorithm SHA256
    if ($signed.TimeStamperCertificate) { $timestampUsed = $true; Write-Host '时间戳签名成功。' }
} catch {
    Write-Warning "带时间戳签名失败：$($_.Exception.Message)"
}
if (-not $signed -or -not $signed.SignerCertificate) {
    Write-Host '不带时间戳签名 ...'
    $signed = Set-AuthenticodeSignature -FilePath $appOut -Certificate $leaf -HashAlgorithm SHA256
}

@(
    "RootSubject=$($root.Subject)",
    "RootThumbprint=$($root.Thumbprint)",
    "LeafSubject=$($leaf.Subject)",
    "LeafThumbprint=$($leaf.Thumbprint)",
    "TimestampUsed=$timestampUsed",
    "App=$appOut"
) | Set-Content -LiteralPath (Join-Path $release 'cert-info.txt') -Encoding UTF8

Get-AuthenticodeSignature -LiteralPath $appOut |
    Format-List Status, StatusMessage, SignerCertificate, TimeStamperCertificate
Write-Host '签名生成完成（尚未安装新根证书信任）。'
