from pathlib import Path
pkg = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\轻画板-HAVEGET自签名版")

install = """@echo off
setlocal
cd /d "%~dp0"
echo ============================================
echo   QingHuaBan - HAVEGET Studio Self-Signed
echo ============================================
echo.
echo Installing the HAVEGET certificate for this user...
echo This is needed once per Windows account.
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File ".\\install.ps1"
if errorlevel 1 (
    echo.
    echo Certificate installation failed. See the message above.
    pause
    exit /b 1
)
exit /b 0
"""

uninstall = """@echo off
setlocal
cd /d "%~dp0"
echo Removing the HAVEGET self-signed certificate for this user...
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File ".\\uninstall.ps1"
echo.
pause
"""

(pkg / "install-and-run.cmd").write_bytes(install.replace("\n", "\r\n").encode("ascii"))
(pkg / "uninstall-cert.cmd").write_bytes(uninstall.replace("\n", "\r\n").encode("ascii"))
print("cmd files rewritten ASCII+CRLF")
