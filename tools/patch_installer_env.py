from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
if "#include <winternl.h>" not in s:
    s = s.replace("#include <windowsx.h>", "#include <windowsx.h>\n#include <winternl.h>")
s = s.replace("std::wstring g_error;", "std::wstring g_error;\nstd::wstring g_envWarning;")

anchor = "void installNow() {"
check = r'''bool checkRuntimeEnvironment(std::wstring& err) {
    SYSTEM_INFO si{};
    GetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64 &&
        si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_ARM64) {
        err = L"此安装包需要 64 位 Windows 10/11。当前系统不是 64 位。";
        return false;
    }
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    auto rtlGetVersion = ntdll ? (RtlGetVersionFn)GetProcAddress(ntdll, "RtlGetVersion") : nullptr;
    if (rtlGetVersion) {
        RTL_OSVERSIONINFOW vi{};
        vi.dwOSVersionInfoSize = sizeof(vi);
        if (rtlGetVersion(&vi) == 0 && vi.dwMajorVersion < 10) {
            err = L"此应用需要 Windows 10 或 Windows 11。当前系统版本过低。";
            return false;
        }
    }
    HMODULE ucrt = LoadLibraryW(L"ucrtbase.dll");
    if (!ucrt) {
        err = L"系统缺少 UCRT 运行库（ucrtbase.dll）。请先在 Windows 更新中安装最新系统更新。";
        return false;
    }
    FreeLibrary(ucrt);
    HMODULE gdi = LoadLibraryW(L"gdiplus.dll");
    if (!gdi) {
        err = L"系统缺少 GDI+ 图形组件（gdiplus.dll）。请先安装 Windows 更新后重试。";
        return false;
    }
    FreeLibrary(gdi);
    wchar_t win[MAX_PATH] = {};
    GetWindowsDirectoryW(win, MAX_PATH);
    std::wstring fonts = std::wstring(win) + L"\\Fonts\\";
    bool hasYahei = fileExists(fonts + L"msyh.ttc") || fileExists(fonts + L"msyhbd.ttc") ||
                    fileExists(fonts + L"msyhl.ttc");
    bool hasFallback = fileExists(fonts + L"segoeui.ttf");
    if (!hasYahei && !hasFallback) {
        g_envWarning = L"系统字体不完整，将使用系统回退字体。";
    } else if (!hasYahei) {
        g_envWarning = L"未找到微软雅黑，界面将使用系统回退字体。";
    }
    return true;
}

'''
s = s.replace(anchor, check + anchor, 1)
s = s.replace("""    g_phase = Working;
    g_progress = 0.04f;
    g_status = L"正在准备安装目录…";
    g_error.clear();
    g_certWarning = false;
    InvalidateRect(g_hwnd, nullptr, FALSE);
    UpdateWindow(g_hwnd);

    g_installDir = chooseInstallDir();""",
"""    g_phase = Working;
    g_progress = 0.04f;
    g_status = L"正在检查运行环境…";
    g_error.clear();
    g_envWarning.clear();
    g_certWarning = false;
    InvalidateRect(g_hwnd, nullptr, FALSE);
    UpdateWindow(g_hwnd);
    if (!checkRuntimeEnvironment(g_error)) {
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }

    g_progress = 0.10f;
    g_status = L"正在准备安装目录…";
    UpdateWindow(g_hwnd);
    g_installDir = chooseInstallDir();""")
p.write_text(s, encoding="utf-8", newline="\n")
print("installer runtime environment check")
