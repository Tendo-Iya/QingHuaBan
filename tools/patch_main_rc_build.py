from pathlib import Path

# main.cpp
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\main.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace('SetCurrentProcessExplicitAppUserModelID(L"QingHuaBan.Whiteboard.1");',
              'SetCurrentProcessExplicitAppUserModelID(L"HAVEGET.Whiteboard.1");')
s = s.replace('wc.lpszClassName = L"QingHuaBanWindow";', 'wc.lpszClassName = L"HAVEGETWhiteboardWindow";')
s = s.replace('CreateWindowExW(0, wc.lpszClassName, L"轻画板", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,',
              'CreateWindowExW(0, wc.lpszClassName, L"HAVEGET白板", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,')
s = s.replace("""        case App::WM_APP_COMMIT_TEXT:
            if (g_app) g_app->commitText();
            return 0;""",
"""        case App::WM_APP_COMMIT_TEXT:
            if (g_app) g_app->commitText();
            return 0;
        case App::WM_APP_COMMIT_NAME:
            if (g_app) g_app->commitAppNamePublic();
            return 0;""")
# We have finishAppNameEdit public; use it instead
s = s.replace("g_app->commitAppNamePublic();", "g_app->finishAppNameEdit();")
p.write_text(s, encoding="utf-8", newline="\n")

# app.rc
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\assets\app.rc")
s = p.read_text(encoding="utf-8")
s = s.replace('VALUE "FileDescription", "轻画板 - 即点即写的白板"',
              'VALUE "FileDescription", "HAVEGET白板 - 液态玻璃白板"')
s = s.replace('VALUE "InternalName", "QingHuaBan"', 'VALUE "InternalName", "HAVEGETWhiteboard"')
s = s.replace('VALUE "OriginalFilename", "轻画板.exe"', 'VALUE "OriginalFilename", "HAVEGET白板.exe"')
s = s.replace('VALUE "ProductName", "轻画板"', 'VALUE "ProductName", "HAVEGET白板"')
p.write_text(s, encoding="utf-8", newline="\n")

# build.ps1
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\build.ps1")
s = p.read_text(encoding="utf-8-sig")
s = s.replace("foreach ($name in @('main', 'app', 'render', 'util')) {",
              "foreach ($name in @('main', 'app', 'render', 'util', 'liquid', 'ui')) {")
s = s.replace("$out = Join-Path $root 'QingHuaBan.exe'", "$out = Join-Path $root 'HAVEGET白板.exe'")
s = s.replace("Copy-Item -LiteralPath $out -Destination (Join-Path (Split-Path -Parent $root) '轻画板.exe') -Force",
              "Copy-Item -LiteralPath $out -Destination (Join-Path (Split-Path -Parent $root) 'HAVEGET白板.exe') -Force")
p.write_text(s, encoding="utf-8-sig", newline="\n")
print("main/rc/build patched")
