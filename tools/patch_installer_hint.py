from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace('g_error = L"无法创建安装目录，请检查当前用户对 AppData\\\\Local\\\\Programs 的写入权限。";',
              'g_error = L"无法创建安装目录。请关闭安装器，右键选择“以管理员身份运行”后重试。";')
s = s.replace('g_error = L"无法写出临时文件（app_payload.tmp），错误码 " + std::to_wstring(writeErr) + L"。";',
              'g_error = L"无法写入安装文件（错误码 " + std::to_wstring(writeErr) + L"）。请以管理员身份运行安装器后重试。";')
s = s.replace('drawTextAligned(g, L"首次会弹出一次系统证书确认；不需要管理员权限。",',
              'drawTextAligned(g, L"首次会弹出一次系统证书确认；通常无需管理员权限。",')
p.write_text(s, encoding="utf-8", newline="\n")
print("installer hints patched")
