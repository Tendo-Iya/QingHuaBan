from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")

old = '''bool makeShortcut(const std::wstring& linkPath, const std::wstring& target, const std::wstring& workDir) {
    IShellLinkW* link = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                IID_IShellLinkW, (void**)&link)))
        return false;
    link->SetPath(target.c_str());
    link->SetWorkingDirectory(workDir.c_str());
    link->SetIconLocation(target.c_str(), 0);
    link->SetDescription(L"HAVEGET白板 · 液态玻璃白板");
    IPersistFile* pf = nullptr;
    bool ok = false;
    if (SUCCEEDED(link->QueryInterface(IID_IPersistFile, (void**)&pf))) {
        ok = SUCCEEDED(pf->Save(linkPath.c_str(), TRUE));
        pf->Release();
    }
    link->Release();
    return ok;
}'''
new = '''bool makeShortcut(const std::wstring& linkPath, const std::wstring& target, const std::wstring& workDir) {
    IShellLinkW* link = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                IID_IShellLinkW, (void**)&link)))
        return false;
    wchar_t win[MAX_PATH] = {};
    GetWindowsDirectoryW(win, MAX_PATH);
    std::wstring cmd = std::wstring(win) + L"\\\\System32\\\\cmd.exe";
    std::wstring args = L"/c \\"\\"" + target + L"\\"\\"";
    link->SetPath(cmd.c_str());
    link->SetArguments(args.c_str());
    link->SetWorkingDirectory(workDir.c_str());
    link->SetIconLocation(target.c_str(), 0);
    link->SetShowCmd(SW_SHOWMINNOACTIVE);
    link->SetDescription(L"HAVEGET白板 · 液态玻璃白板");
    IPersistFile* pf = nullptr;
    bool ok = false;
    if (SUCCEEDED(link->QueryInterface(IID_IPersistFile, (void**)&pf))) {
        ok = SUCCEEDED(pf->Save(linkPath.c_str(), TRUE));
        pf->Release();
    }
    link->Release();
    return ok;
}'''
if old not in s: raise SystemExit("makeShortcut anchor missing")
s = s.replace(old, new)

old2 = '''void launchApp() {
    std::wstring app = g_installDir + L"\\\\HAVEGET白板.exe";
    if (fileExists(app))
        ShellExecuteW(g_hwnd, L"open", app.c_str(), nullptr, g_installDir.c_str(), SW_SHOWNORMAL);
}'''
new2 = '''void launchApp() {
    std::wstring app = g_installDir + L"\\\\HAVEGET白板.exe";
    if (!fileExists(app)) return;
    std::wstring cmd = L"\\"" + app + L"\\"";
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(app.c_str(), cmd.data(), nullptr, nullptr, FALSE,
                       CREATE_NEW_PROCESS_GROUP, nullptr, g_installDir.c_str(), &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}'''
if old2 not in s: raise SystemExit("launchApp anchor missing")
s = s.replace(old2, new2)

p.write_text(s, encoding="utf-8", newline="\n")
print("installer shortcut/launch patched")

# portable fallback direct launch
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\HAVEGET-白板-发布\portable-install.ps1")
s = p.read_text(encoding="utf-8-sig")
s = s.replace("Start-Process -FilePath $exe",
"""$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $exe
$psi.UseShellExecute = $false
[System.Diagnostics.Process]::Start($psi) | Out-Null""")
p.write_text(s, encoding="utf-8-sig", newline="\n")
print("portable fallback patched")
