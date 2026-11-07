#include "qhb.h"

#include <dwmapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <windowsx.h>
#include <winternl.h>
#include <cmath>

#include <algorithm>
#include <string>
#include <vector>

using namespace qhb;

namespace {

constexpr int IDI_SETUP = 101;
constexpr int IDR_APP = 201;
constexpr int IDR_ROOT = 202;
constexpr int IDR_LEAF = 203;
constexpr wchar_t kAppName[] = L"HAVEGET白板";
constexpr wchar_t kRootThumb[] = L"4DF3722ABC51338BDB28219E73B35B8CA65EDF63";
constexpr wchar_t kLeafThumb[] = L"061F255A26646F137DC012A893E3D5DD91CB4BD8";
constexpr int BASE_W = 760;
constexpr int BASE_H = 500;

HINSTANCE g_inst = nullptr;
HWND g_hwnd = nullptr;
float g_sc = 1.0f;
int g_w = BASE_W, g_h = BASE_H;
std::unique_ptr<Bitmap> g_back;

enum Phase { Ready, Working, Done, Failed };
Phase g_phase = Ready;
float g_progress = 0;
std::wstring g_status = L"准备安装";
std::wstring g_error;
std::wstring g_envWarning;
std::wstring g_installDir;
bool g_desktop = true;
bool g_installCert = true;
bool g_certWarning = false;
int g_hot = 0;

std::unique_ptr<Font> fTitle, fHero, fUI, fSmall, fTiny;

void dbg(const std::wstring& s) {
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

float S(float v) { return v * g_sc; }
RectF client() { return RectF(0, 0, (float)g_w, (float)g_h); }
RectF cardRect() { return RectF(S(24), S(24), (float)g_w - S(48), (float)g_h - S(48)); }
RectF closeRect() { return RectF((float)g_w - S(70), S(38), S(32), S(32)); }
RectF installRect() { return RectF((float)g_w - S(206), (float)g_h - S(96), S(150), S(46)); }
RectF cancelRect() { return RectF((float)g_w - S(326), (float)g_h - S(96), S(108), S(46)); }
RectF launchRect() { return RectF((float)g_w - S(246), (float)g_h - S(96), S(190), S(46)); }
RectF doneCloseRect() { return RectF((float)g_w - S(326), (float)g_h - S(96), S(68), S(46)); }
RectF desktopRect() { return RectF(S(56), S(270), S(24), S(24)); }
RectF certRect() { return RectF(S(56), S(306), S(24), S(24)); }

void makeFonts() {
    auto mk = [&](float px, bool bold = false) {
        return std::make_unique<Font>(L"Microsoft YaHei UI", S(px),
                                      bold ? FontStyleBold : FontStyleRegular, UnitPixel);
    };
    fTitle = mk(31, true);
    fHero = mk(24, true);
    fUI = mk(14.0f);
    fSmall = mk(12.2f);
    fTiny = mk(11.0f);
}

void drawOrbs(Graphics& g) {
    auto orb = [&](float cx, float cy, float rad, uint32_t col, int alpha) {
        for (int i = 0; i < 6; ++i) {
            float rr = rad * (1.0f + i * 0.24f);
            SolidBrush b(RGBA(col, (BYTE)std::max(2, alpha / (i + 1))));
            g.FillEllipse(&b, cx - rr, cy - rr, rr * 2, rr * 2);
        }
    };
    orb((float)g_w * 0.83f, (float)g_h * 0.10f, S(240), 0x9FE2C7, 28);
    orb((float)g_w * 0.10f, (float)g_h * 0.86f, S(220), 0xFFE0C7, 20);
    orb((float)g_w * 0.55f, (float)g_h * 0.48f, S(260), 0xD8E9FF, 13);
}

void glass(Graphics& g, RectF r, float radius, BYTE alpha) {
    drawShadow(g, r, radius, S(14), S(6));
    GraphicsPath path;
    roundPath(path, r, radius);
    LinearGradientBrush bg(PointF(r.X, r.Y), PointF(r.X, r.GetBottom()),
                           Color(alpha, 255, 255, 255),
                           Color((BYTE)std::max(30, (int)alpha - 34), 238, 246, 242));
    g.FillPath(&bg, &path);
    Pen edge(Color(190, 255, 255, 255), S(1));
    g.DrawPath(&edge, &path);
    Pen high(Color(220, 255, 255, 255), S(1));
    g.DrawLine(&high, r.X + radius * 0.6f, r.Y + S(1), r.GetRight() - radius * 0.6f, r.Y + S(1));
}

void button(Graphics& g, RectF r, const std::wstring& text, bool primary, bool enabled, int hotId) {
    bool hover = enabled && g_hot == hotId;
    if (primary) {
        fillRoundRect(g, r, S(13), hover ? 0x0F8F6B : COL_ACCENT, 248);
    } else {
        fillRoundRect(g, r, S(13), 0xFFFFFF, hover ? 190 : 112);
        strokeRoundRect(g, r, S(13), 0xD2DDD6, S(1), 220);
    }
    drawTextAligned(g, text, r, fUI, primary ? 0xFFFFFF : (enabled ? 0x4A5D53 : 0xB6C2BB),
                    StringAlignmentCenter, StringAlignmentCenter, false);
}

void checkbox(Graphics& g, RectF r, bool on, bool enabled) {
    fillRoundRect(g, r, S(7), on ? COL_ACCENT : 0xFFFFFF, on ? 250 : 120);
    strokeRoundRect(g, r, S(7), on ? COL_ACCENT : 0xCBD7D0, S(1.2f), 230);
    if (on) drawCheckmark(g, RectF(r.X + S(4), r.Y + S(4), r.Width - S(8), r.Height - S(8)),
                          0xFFFFFF, S(1.8f));
}

void drawLogo(Graphics& g, RectF r) {
    drawShadow(g, r, S(15), S(12), S(5));
    GraphicsPath path;
    roundPath(path, r, S(15));
    LinearGradientBrush bg(PointF(r.X, r.Y), PointF(r.GetRight(), r.GetBottom()),
                           RGBA(0x2AC79A), RGBA(0x0C8C68));
    g.FillPath(&bg, &path);
    Pen edge(Color(190, 255, 255, 255), S(1.1f));
    g.DrawPath(&edge, &path);
    drawTextAligned(g, L"H", r, fHero, 0xFFFFFF, StringAlignmentCenter, StringAlignmentCenter, false);
}

std::wstring localAppData() {
    wchar_t buf[MAX_PATH] = {};
    if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, buf) != S_OK)
        return L".";
    return buf;
}

std::wstring startMenuDir() {
    wchar_t buf[MAX_PATH] = {};
    if (SHGetFolderPathW(nullptr, CSIDL_PROGRAMS, nullptr, SHGFP_TYPE_CURRENT, buf) != S_OK)
        return L".";
    return buf;
}

std::wstring desktopDir() {
    wchar_t buf[MAX_PATH] = {};
    if (SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, SHGFP_TYPE_CURRENT, buf) != S_OK)
        return L".";
    return buf;
}

std::wstring appData() {
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

bool writeResource(HINSTANCE inst, int id, const std::wstring& path) {
    HRSRC res = FindResourceW(inst, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!res) return false;
    HGLOBAL h = LoadResource(inst, res);
    if (!h) return false;
    void* ptr = LockResource(h);
    DWORD sz = SizeofResource(inst, res);
    if (!ptr || !sz) return false;
    HANDLE hf = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hf == INVALID_HANDLE_VALUE) {
        dbg(L"CreateFile failed path=[" + path + L"] err=" + std::to_wstring(GetLastError()));
        return false;
    }
    DWORD wrote = 0;
    BOOL ok = WriteFile(hf, ptr, sz, &wrote, nullptr);
    CloseHandle(hf);
    return ok && wrote == sz;
}

bool runConsole(const std::wstring& cmd) {
    std::wstring c = cmd;
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    if (!CreateProcessW(nullptr, c.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi))
        return false;
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return code == 0;
}

bool makeShortcut(const std::wstring& linkPath, const std::wstring& target, const std::wstring& workDir) {
    IShellLinkW* link = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                IID_IShellLinkW, (void**)&link)))
        return false;
    wchar_t win[MAX_PATH] = {};
    GetWindowsDirectoryW(win, MAX_PATH);
    std::wstring cmd = std::wstring(win) + L"\\System32\\cmd.exe";
    std::wstring args = L"/c \"\"" + target + L"\"\"";
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
}

void writeUninstaller() {
    std::wstring ps = g_installDir + L"\\uninstall.ps1";
    std::wstring text =
        L"$ErrorActionPreference='SilentlyContinue'\n"
        L"$dir=Split-Path -Parent $MyInvocation.MyCommand.Path\n"
        L"Remove-Item -LiteralPath (Join-Path $env:USERPROFILE 'Desktop\\HAVEGET白板.lnk') -Force\n"
        L"Remove-Item -LiteralPath (Join-Path $env:APPDATA 'Microsoft\\Windows\\Start Menu\\Programs\\HAVEGET白板.lnk') -Force\n"
        L"Remove-Item -LiteralPath 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\HAVEGET白板' -Recurse -Force\n"
        L"& certutil.exe -user -delstore TrustedPublisher '" + std::wstring(kLeafThumb) + L"'\n"
        L"& certutil.exe -user -delstore Root '" + std::wstring(kRootThumb) + L"'\n"
        L"Remove-Item -LiteralPath (Join-Path $dir 'HAVEGET白板.exe') -Force\n"
        L"Remove-Item -LiteralPath (Join-Path $dir 'HAVEGET-RootCA.cer') -Force\n"
        L"Remove-Item -LiteralPath (Join-Path $dir 'HAVEGET-CodeSigning.cer') -Force\n"
        L"Start-Process -WindowStyle Hidden -FilePath 'cmd.exe' -ArgumentList '/c',('timeout /t 1 >nul & rmdir /s /q \"' + $dir + '\"')\n";
    std::string utf8;
    int n = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0, nullptr, nullptr);
    utf8.resize((size_t)n);
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), utf8.data(), n, nullptr, nullptr);
    FILE* f = _wfopen(ps.c_str(), L"wb");
    if (f) {
        const unsigned char bom[3] = {0xEF, 0xBB, 0xBF};
        fwrite(bom, 1, 3, f);
        fwrite(utf8.data(), 1, utf8.size(), f);
        fclose(f);
    }
}

void registerUninstall() {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\HAVEGET白板",
                        0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS)
        return;
    auto set = [&](const wchar_t* name, const std::wstring& value) {
        RegSetValueExW(key, name, 0, REG_SZ, (const BYTE*)value.c_str(),
                       (DWORD)((value.size() + 1) * sizeof(wchar_t)));
    };
    std::wstring app = g_installDir + L"\\HAVEGET白板.exe";
    set(L"DisplayName", L"HAVEGET白板");
    set(L"DisplayVersion", L"1.8.2.4");
    set(L"Publisher", L"HAVEGET工作室");
    set(L"DisplayIcon", app + L",0");
    set(L"InstallLocation", g_installDir);
    set(L"UninstallString", L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
                            g_installDir + L"\\uninstall.ps1\"");
    DWORD one = 1;
    RegSetValueExW(key, L"NoModify", 0, REG_DWORD, (const BYTE*)&one, sizeof(one));
    RegSetValueExW(key, L"NoRepair", 0, REG_DWORD, (const BYTE*)&one, sizeof(one));
    RegCloseKey(key);
}

bool checkRuntimeEnvironment(std::wstring& err) {
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

void installNow() {
    g_phase = Working;
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
    g_installDir = chooseInstallDir();
    dbg(L"choose installDir=[" + g_installDir + L"]");
    if (g_installDir.empty()) {
        g_error = L"没有可写的安装目录。请关闭正在运行的 HAVEGET白板，或右键“以管理员身份运行”安装器。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    std::wstring app = g_installDir + L"\\HAVEGET白板.exe";
    std::wstring rootCer = g_installDir + L"\\HAVEGET-RootCA.cer";
    std::wstring leafCer = g_installDir + L"\\HAVEGET-CodeSigning.cer";

    g_progress = 0.16f;
    g_status = L"正在复制 HAVEGET白板…";
    InvalidateRect(g_hwnd, nullptr, FALSE);
    UpdateWindow(g_hwnd);
    if (!FindResourceW(g_inst, MAKEINTRESOURCEW(IDR_APP), RT_RCDATA)) {
        g_error = L"安装包缺少应用程序资源，请重新下载安装器。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    std::wstring appTmp = g_installDir + L"\\app_payload.tmp";
    dbg(L"appTmp=[" + appTmp + L"]");
    if (!writeResource(g_inst, IDR_APP, appTmp)) {
        DWORD writeErr = GetLastError();
        dbg(L"writeResource failed err=" + std::to_wstring(writeErr) + L" appTmp=[" + appTmp + L"]");
        g_error = L"无法写入安装文件（错误码 " + std::to_wstring(writeErr) + L"）。请以管理员身份运行安装器后重试。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    SetFileAttributesW(app.c_str(), FILE_ATTRIBUTE_NORMAL);
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
    }
    writeResource(g_inst, IDR_ROOT, rootCer);
    writeResource(g_inst, IDR_LEAF, leafCer);
    DeleteFileW((app + L":Zone.Identifier").c_str());

    if (g_installCert) {
        g_progress = 0.38f;
        g_status = L"正在安装证书，如出现安全警告请点“是”…";
        InvalidateRect(g_hwnd, nullptr, FALSE);
        UpdateWindow(g_hwnd);
        bool ok1 = runConsole(L"certutil.exe -user -addstore -f Root \"" + rootCer + L"\"");
        bool ok2 = runConsole(L"certutil.exe -user -addstore -f TrustedPublisher \"" + leafCer + L"\"");
        if (!ok1 || !ok2) g_certWarning = true;
    }

    g_progress = 0.70f;
    g_status = L"正在创建快捷方式…";
    InvalidateRect(g_hwnd, nullptr, FALSE);
    UpdateWindow(g_hwnd);
    makeShortcut(startMenuDir() + L"\\HAVEGET白板.lnk", app, g_installDir);
    if (g_desktop) makeShortcut(desktopDir() + L"\\HAVEGET白板.lnk", app, g_installDir);

    g_progress = 0.88f;
    g_status = L"正在注册卸载信息…";
    writeUninstaller();
    registerUninstall();

    g_progress = 1.0f;
    g_status = g_certWarning ? L"已安装；证书安装未完成，应用仍可运行。"
                             : L"HAVEGET白板 已安装完成。";
    g_phase = Done;
    InvalidateRect(g_hwnd, nullptr, FALSE);
}

void paint(Graphics& g) {
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    LinearGradientBrush bg(PointF(0, 0), PointF((float)g_w, (float)g_h),
                           RGBA(0xF8FBF9), RGBA(0xE7F0EB));
    g.FillRectangle(&bg, client());
    drawOrbs(g);

    RectF card = cardRect();
    glass(g, card, S(28), 242);

    RectF logo(S(56), S(48), S(58), S(58));
    drawLogo(g, logo);
    drawTextAligned(g, L"HAVEGET白板", RectF(S(130), S(46), S(360), S(38)), fTitle, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"液态玻璃白板 · 安装器", RectF(S(132), S(82), S(360), S(22)), fSmall, 0x5C6F65,
                    StringAlignmentNear, StringAlignmentCenter, false);

    RectF close = closeRect();
    if (g_hot == 1) fillRoundRect(g, close, S(9), 0xFFFFFF, 170);
    drawIcon(g, Icon::Close, RectF(close.X + S(8), close.Y + S(8), S(16), S(16)), 0x596C62, S(1.6f));

    Pen line(RGBA(0x5F746A, 36), S(1));
    g.DrawLine(&line, S(56), S(126), (float)g_w - S(56), S(126));

    if (g_phase == Ready) {
        drawTextAligned(g, L"安装到这台电脑", RectF(S(56), S(144), S(420), S(32)), fHero, 0x2C4036,
                        StringAlignmentNear, StringAlignmentCenter, false);
        RectF pathR(S(56), S(188), (float)g_w - S(112), S(46));
        fillRoundRect(g, pathR, S(13), 0xFFFFFF, 132);
        strokeRoundRect(g, pathR, S(13), 0xD4DFD8, S(1), 200);
        drawIcon(g, Icon::Folder, RectF(pathR.X + S(14), pathR.Y + S(13), S(20), S(20)), 0x52665C, S(1.6f));
        drawTextAligned(g, localAppData() + L"\\Programs\\HAVEGET白板",
                        RectF(pathR.X + S(44), pathR.Y, pathR.Width - S(58), pathR.Height),
                        fSmall, 0x596D62, StringAlignmentNear, StringAlignmentCenter, true);

        checkbox(g, desktopRect(), g_desktop, true);
        drawTextAligned(g, L"创建桌面快捷方式", RectF(S(90), S(270), S(360), S(24)), fUI, 0x40544A,
                        StringAlignmentNear, StringAlignmentCenter, false);
        checkbox(g, certRect(), g_installCert, true);
        drawTextAligned(g, L"安装 HAVEGET 证书（推荐）", RectF(S(90), S(306), S(360), S(24)), fUI, 0x40544A,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, L"首次会弹出一次系统证书确认；通常无需管理员权限。",
                        RectF(S(90), S(334), S(520), S(22)), fSmall, 0x6B7E74,
                        StringAlignmentNear, StringAlignmentCenter, false);

        button(g, installRect(), L"安装", true, true, 2);
        button(g, cancelRect(), L"取消", false, true, 3);
    } else if (g_phase == Working) {
        drawTextAligned(g, L"正在安装", RectF(S(56), S(154), S(420), S(34)), fHero, 0x2C4036,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, g_status, RectF(S(56), S(200), (float)g_w - S(112), S(26)), fUI, 0x45594F,
                        StringAlignmentNear, StringAlignmentCenter, true);
        RectF track(S(56), S(250), (float)g_w - S(112), S(8));
        fillRoundRect(g, track, S(4), 0xFFFFFF, 120);
        if (g_progress > 0.01f)
            fillRoundRect(g, RectF(track.X, track.Y, track.Width * g_progress, track.Height),
                          S(4), COL_ACCENT, 245);
        drawTextAligned(g, L"请稍候，不要关闭窗口。", RectF(S(56), S(280), (float)g_w - S(112), S(22)),
                        fSmall, 0x6B7E74, StringAlignmentNear, StringAlignmentCenter, false);
    } else if (g_phase == Done) {
        RectF ok(S(56), S(148), S(48), S(48));
        fillRoundRect(g, ok, S(24), COL_ACCENT_SOFT, 250);
        drawCheckmark(g, RectF(ok.X + S(12), ok.Y + S(12), S(24), S(24)), COL_ACCENT, S(2.2f));
        drawTextAligned(g, L"安装完成", RectF(S(120), S(150), S(420), S(36)), fHero, 0x2C4036,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, g_status, RectF(S(56), S(214), (float)g_w - S(112), S(26)), fUI, 0x45594F,
                        StringAlignmentNear, StringAlignmentCenter, true);
        drawTextAligned(g, L"以后可从桌面或开始菜单直接打开 HAVEGET白板。",
                        RectF(S(56), S(248), (float)g_w - S(112), S(22)), fSmall, 0x6B7E74,
                        StringAlignmentNear, StringAlignmentCenter, false);
        if (g_certWarning) {
            drawTextAligned(g, L"提示：证书未安装成功，首次启动仍可能出现 SmartScreen。",
                            RectF(S(56), S(276), (float)g_w - S(112), S(22)), fSmall, 0xB7772A,
                            StringAlignmentNear, StringAlignmentCenter, true);
        }
        button(g, launchRect(), L"启动 HAVEGET白板", true, true, 4);
        button(g, doneCloseRect(), L"关闭", false, true, 5);
    } else {
        drawTextAligned(g, L"安装未完成", RectF(S(56), S(154), S(420), S(34)), fHero, 0x7A3B3B,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, g_error.empty() ? L"发生未知错误。" : g_error,
                        RectF(S(56), S(206), (float)g_w - S(112), S(50)), fUI, 0x7A4B4B,
                        StringAlignmentNear, StringAlignmentCenter, true);
        button(g, installRect(), L"重试", true, true, 2);
        button(g, cancelRect(), L"关闭", false, true, 3);
    }

    drawTextAligned(g, L"© 2026 HAVEGET工作室  ·  All Rights Reserved.",
                    RectF(S(58), (float)g_h - S(52), S(430), S(18)), fTiny, 0x788A80,
                    StringAlignmentNear, StringAlignmentCenter, false);
}

void paintWindow(HDC hdc) {
    if (!g_back || g_back->GetWidth() != g_w || g_back->GetHeight() != g_h)
        g_back = std::make_unique<Bitmap>(g_w, g_h, PixelFormat32bppPARGB);
    Graphics g(g_back.get());
    paint(g);
    Graphics dst(hdc);
    dst.DrawImage(g_back.get(), 0, 0, g_w, g_h);
}

int hitTest(int x, int y) {
    auto in = [&](RectF r) { return r.Contains((float)x, (float)y); };
    if (g_phase != Working && in(closeRect())) return 1;
    if (g_phase == Ready) {
        if (in(desktopRect())) return 10;
        if (in(certRect())) return 11;
        if (in(installRect())) return 2;
        if (in(cancelRect())) return 3;
    } else if (g_phase == Done) {
        if (in(launchRect())) return 4;
        if (in(doneCloseRect())) return 5;
    } else if (g_phase == Failed) {
        if (in(installRect())) return 2;
        if (in(cancelRect())) return 3;
    }
    return 0;
}

void launchApp() {
    std::wstring app = g_installDir + L"\\HAVEGET白板.exe";
    if (!fileExists(app)) return;
    std::wstring cmd = L"\"" + app + L"\"";
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(app.c_str(), cmd.data(), nullptr, nullptr, FALSE,
                       CREATE_NEW_PROCESS_GROUP, nullptr, g_installDir.c_str(), &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            g_hwnd = hwnd;
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            paintWindow(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_MOUSEMOVE: {
            int old = g_hot;
            g_hot = hitTest(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            if (g_hot != old) InvalidateRect(hwnd, nullptr, FALSE);
            TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, hwnd, 0};
            TrackMouseEvent(&tme);
            return 0;
        }
        case WM_MOUSELEAVE:
            if (g_hot) { g_hot = 0; InvalidateRect(hwnd, nullptr, FALSE); }
            return 0;
        case WM_LBUTTONDOWN: {
            int hit = hitTest(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            if (hit == 1) {
                if (g_phase != Working) DestroyWindow(hwnd);
            } else if (hit == 10) {
                g_desktop = !g_desktop;
                InvalidateRect(hwnd, nullptr, FALSE);
            } else if (hit == 11) {
                g_installCert = !g_installCert;
                InvalidateRect(hwnd, nullptr, FALSE);
            } else if (hit == 2) {
                installNow();
            } else if (hit == 3) {
                if (g_phase == Ready || g_phase == Failed) DestroyWindow(hwnd);
                else if (g_phase == Done) DestroyWindow(hwnd);
            } else if (hit == 4) {
                launchApp();
            } else if (hit == 5) {
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE && g_phase != Working) DestroyWindow(hwnd);
            else if (wp == VK_RETURN && g_phase == Ready) installNow();
            return 0;
        case WM_NCHITTEST: {
            POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            ScreenToClient(hwnd, &pt);
            if (pt.y < (int)S(120) && !closeRect().Contains((float)pt.x, (float)pt.y))
                return HTCAPTION;
            return HTCLIENT;
        }
        case WM_CLOSE:
            if (g_phase != Working) DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
    g_inst = inst;
    SetProcessDpiAwarenessContext((DPI_AWARENESS_CONTEXT)-4);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    GdiplusStartupInput gsi;
    ULONG_PTR token = 0;
    if (GdiplusStartup(&token, &gsi, nullptr) != Ok) return 1;

    HDC screen = GetDC(nullptr);
    g_sc = (float)GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
    ReleaseDC(nullptr, screen);
    if (g_sc < 1.0f) g_sc = 1.0f;
    g_w = (int)std::round(BASE_W * g_sc);
    g_h = (int)std::round(BASE_H * g_sc);
    makeFonts();

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_SETUP), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    if (!wc.hIcon) wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    wc.lpszClassName = L"HAVEGETSetupWindow";
    if (!RegisterClassExW(&wc)) return 1;

    int px = (GetSystemMetrics(SM_CXSCREEN) - g_w) / 2;
    int py = (GetSystemMetrics(SM_CYSCREEN) - g_h) / 2;
    HWND hwnd = CreateWindowExW(WS_EX_APPWINDOW, wc.lpszClassName, L"HAVEGET白板 安装器",
                                WS_POPUP | WS_CLIPCHILDREN, px, py, g_w, g_h,
                                nullptr, nullptr, inst, nullptr);
    if (!hwnd) return 1;
    HRGN rgn = CreateRoundRectRgn(0, 0, g_w + 1, g_h + 1, (int)S(28), (int)S(28));
    SetWindowRgn(hwnd, rgn, TRUE);
    int pref = 2;
    DwmSetWindowAttribute(hwnd, 33, &pref, sizeof(pref));
    MARGINS margins{1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    GdiplusShutdown(token);
    CoUninitialize();
    return 0;
}
