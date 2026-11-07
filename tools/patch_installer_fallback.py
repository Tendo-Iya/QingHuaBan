from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
if "#include <vector>" not in s:
    s = s.replace("#include <string>", "#include <string>\n#include <vector>")

anchor = "bool writeResource(HINSTANCE inst, int id, const std::wstring& path) {"
helpers = r'''std::wstring appData() {
    wchar_t buf[MAX_PATH] = {};
    if (SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, buf) != S_OK)
        return L".";
    return buf;
}

bool directoryWritable(const std::wstring& dir) {
    std::wstring probe = dir + L"\\.hgwb_write_test";
    HANDLE h = CreateFileW(probe.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    CloseHandle(h);
    return true;
}

bool targetWritable(const std::wstring& dir) {
    std::wstring app = dir + L"\\HAVEGET白板.exe";
    DWORD attr = GetFileAttributesW(app.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) return true;
    HANDLE h = CreateFileW(app.c_str(), GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    CloseHandle(h);
    return true;
}

std::wstring chooseInstallDir() {
    std::wstring lad = localAppData();
    wchar_t tmp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, tmp);
    std::vector<std::wstring> candidates = {
        lad + L"\\Programs\\HAVEGET白板",
        lad + L"\\Programs\\HAVEGET白板-1.8.2.4",
        lad + L"\\HAVEGET白板",
        appData() + L"\\HAVEGET白板",
        std::wstring(tmp) + L"HAVEGET白板"
    };
    for (auto& dir : candidates) {
        std::wstring parent = dir;
        size_t slash = parent.find_last_of(L'\\');
        if (slash != std::wstring::npos) {
            parent.resize(slash);
            CreateDirectoryW(parent.c_str(), nullptr);
        }
        CreateDirectoryW(dir.c_str(), nullptr);
        DWORD attr = GetFileAttributesW(dir.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) continue;
        if (!directoryWritable(dir)) continue;
        if (!targetWritable(dir)) continue;
        return dir;
    }
    return L"";
}

'''
s = s.replace(anchor, helpers + anchor, 1)

s = s.replace("""    g_installDir = localAppData() + L"\\\\Programs\\\\HAVEGET白板";
    std::wstring programs = localAppData() + L"\\\\Programs";
    dbg(L"start localAppData=[" + localAppData() + L"] installDir=[" + g_installDir + L"]");
    CreateDirectoryW(programs.c_str(), nullptr);
    DWORD e1 = GetLastError();
    CreateDirectoryW(g_installDir.c_str(), nullptr);
    DWORD e2 = GetLastError();
    dbg(L"create programs err=" + std::to_wstring(e1) + L" create install err=" + std::to_wstring(e2));
    DWORD dirAttr = GetFileAttributesW(g_installDir.c_str());
    dbg(L"dirAttr=" + std::to_wstring(dirAttr) + L" lastErr=" + std::to_wstring(GetLastError()));
    if (dirAttr == INVALID_FILE_ATTRIBUTES || !(dirAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        g_error = L"无法创建安装目录。请关闭安装器，右键选择“以管理员身份运行”后重试。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }""",
"""    g_installDir = chooseInstallDir();
    dbg(L"choose installDir=[" + g_installDir + L"]");
    if (g_installDir.empty()) {
        g_error = L"没有可写的安装目录。请关闭正在运行的 HAVEGET白板，或右键“以管理员身份运行”安装器。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }""")
s = s.replace("""    if (!MoveFileExW(appTmp.c_str(), app.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        DWORD moveErr = GetLastError();
        DeleteFileW(appTmp.c_str());
        g_error = L"无法写入应用程序文件（" + fileNameOf(app) + L"），错误码 " + std::to_wstring(moveErr) + L"。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }""",
"""    SetFileAttributesW(app.c_str(), FILE_ATTRIBUTE_NORMAL);
    bool moved = MoveFileExW(appTmp.c_str(), app.c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE;
    if (!moved) {
        DWORD moveErr = GetLastError();
        DeleteFileW(app.c_str());
        moved = MoveFileExW(appTmp.c_str(), app.c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE;
        if (!moved) {
            DWORD moveErr2 = GetLastError();
            DeleteFileW(appTmp.c_str());
            g_error = L"无法写入应用程序文件（" + fileNameOf(app) + L"），错误码 " +
                      std::to_wstring(moveErr ? moveErr : moveErr2) +
                      L"。请关闭正在运行的 HAVEGET白板后重试。";
            g_phase = Failed;
            InvalidateRect(g_hwnd, nullptr, FALSE);
            return;
        }
    }""")
p.write_text(s, encoding="utf-8", newline="\n")
print("installer fallback writable directory")
