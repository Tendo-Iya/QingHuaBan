from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
old = '''    if (!writeResource(g_inst, IDR_APP, app)) {
        DWORD writeErr = GetLastError();
        g_error = L"无法写出应用程序文件（" + fileNameOf(app) + L"），错误码 " + std::to_wstring(writeErr) + L"。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    writeResource(g_inst, IDR_ROOT, rootCer);
    writeResource(g_inst, IDR_LEAF, leafCer);'''
new = '''    std::wstring appTmp = g_installDir + L"\\\\app_payload.tmp";
    if (!writeResource(g_inst, IDR_APP, appTmp)) {
        DWORD writeErr = GetLastError();
        g_error = L"无法写出临时文件（app_payload.tmp），错误码 " + std::to_wstring(writeErr) + L"。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    if (!MoveFileExW(appTmp.c_str(), app.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        DWORD moveErr = GetLastError();
        DeleteFileW(appTmp.c_str());
        g_error = L"无法写入应用程序文件（" + fileNameOf(app) + L"），错误码 " + std::to_wstring(moveErr) + L"。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    writeResource(g_inst, IDR_ROOT, rootCer);
    writeResource(g_inst, IDR_LEAF, leafCer);'''
if old not in s:
    raise SystemExit("app write block missing")
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("staging write patched")
