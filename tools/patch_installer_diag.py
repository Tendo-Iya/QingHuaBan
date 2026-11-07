from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
old = '''    g_installDir = localAppData() + L"\\\\Programs\\\\HAVEGET白板";
    SHCreateDirectoryExW(nullptr, g_installDir.c_str(), nullptr);
    std::wstring app = g_installDir + L"\\\\HAVEGET白板.exe";'''
new = '''    g_installDir = localAppData() + L"\\\\Programs\\\\HAVEGET白板";
    std::wstring programs = localAppData() + L"\\\\Programs";
    CreateDirectoryW(programs.c_str(), nullptr);
    CreateDirectoryW(g_installDir.c_str(), nullptr);
    if (!fileExists(g_installDir)) {
        g_error = L"无法创建安装目录，请检查当前用户对 AppData\\\\Local\\\\Programs 的写入权限。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    std::wstring app = g_installDir + L"\\\\HAVEGET白板.exe";'''
if old not in s:
    raise SystemExit("install dir anchor missing")
s = s.replace(old, new)

old2 = '''    if (!writeResource(g_inst, IDR_APP, app)) {
        g_error = L"无法写出应用程序文件。";'''
new2 = '''    if (!FindResourceW(g_inst, MAKEINTRESOURCEW(IDR_APP), RT_RCDATA)) {
        g_error = L"安装包缺少应用程序资源，请重新下载安装器。";
        g_phase = Failed;
        InvalidateRect(g_hwnd, nullptr, FALSE);
        return;
    }
    if (!writeResource(g_inst, IDR_APP, app)) {
        g_error = L"无法写出应用程序文件，请检查磁盘空间或权限。";'''
if old2 not in s:
    raise SystemExit("write resource anchor missing")
s = s.replace(old2, new2)
p.write_text(s, encoding="utf-8", newline="\n")
print("installer diagnostics patched")
