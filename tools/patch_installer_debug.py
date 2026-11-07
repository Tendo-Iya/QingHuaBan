from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
# insert debug helper after constants / globals
anchor = "float S(float v) { return v * g_sc; }"
dbg = r'''void dbg(const std::wstring& s) {
    wchar_t tmp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, tmp);
    std::wstring path = std::wstring(tmp) + L"hav_setup_debug.txt";
    HANDLE h = CreateFileW(path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    std::string line = utf8(s) + "\r\n";
    DWORD wrote = 0;
    WriteFile(h, line.data(), (DWORD)line.size(), &wrote, nullptr);
    CloseHandle(h);
}

float S(float v) { return v * g_sc; }'''
if anchor not in s:
    raise SystemExit("S anchor missing")
s = s.replace(anchor, dbg, 1)

# log path and create errors
old = '''    g_installDir = localAppData() + L"\\\\Programs\\\\HAVEGET白板";
    std::wstring programs = localAppData() + L"\\\\Programs";
    CreateDirectoryW(programs.c_str(), nullptr);
    CreateDirectoryW(g_installDir.c_str(), nullptr);
    DWORD dirAttr = GetFileAttributesW(g_installDir.c_str());'''
new = '''    g_installDir = localAppData() + L"\\\\Programs\\\\HAVEGET白板";
    std::wstring programs = localAppData() + L"\\\\Programs";
    dbg(L"start localAppData=[" + localAppData() + L"] installDir=[" + g_installDir + L"]");
    CreateDirectoryW(programs.c_str(), nullptr);
    DWORD e1 = GetLastError();
    CreateDirectoryW(g_installDir.c_str(), nullptr);
    DWORD e2 = GetLastError();
    dbg(L"create programs err=" + std::to_wstring(e1) + L" create install err=" + std::to_wstring(e2));
    DWORD dirAttr = GetFileAttributesW(g_installDir.c_str());
    dbg(L"dirAttr=" + std::to_wstring(dirAttr) + L" lastErr=" + std::to_wstring(GetLastError()));'''
if old not in s:
    raise SystemExit("dir block missing")
s = s.replace(old, new)

# log after writeResource failure and temporary path
old2 = '''    std::wstring appTmp = g_installDir + L"\\\\app_payload.tmp";
    if (!writeResource(g_inst, IDR_APP, appTmp)) {
        DWORD writeErr = GetLastError();
        g_error = L"无法写出临时文件（app_payload.tmp），错误码 " + std::to_wstring(writeErr) + L"。";'''
new2 = '''    std::wstring appTmp = g_installDir + L"\\\\app_payload.tmp";
    dbg(L"appTmp=[" + appTmp + L"]");
    if (!writeResource(g_inst, IDR_APP, appTmp)) {
        DWORD writeErr = GetLastError();
        dbg(L"writeResource failed err=" + std::to_wstring(writeErr) + L" appTmp=[" + appTmp + L"]");
        g_error = L"无法写出临时文件（app_payload.tmp），错误码 " + std::to_wstring(writeErr) + L"。";'''
if old2 not in s:
    raise SystemExit("appTmp block missing")
s = s.replace(old2, new2)

# log inside writeResource
old3 = '''    HANDLE hf = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hf == INVALID_HANDLE_VALUE) return false;'''
new3 = '''    HANDLE hf = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hf == INVALID_HANDLE_VALUE) {
        dbg(L"CreateFile failed path=[" + path + L"] err=" + std::to_wstring(GetLastError()));
        return false;
    }'''
if old3 not in s:
    raise SystemExit("CreateFile block missing")
s = s.replace(old3, new3)
p.write_text(s, encoding="utf-8", newline="\n")
print("debug logging added")
