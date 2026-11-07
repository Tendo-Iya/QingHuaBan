from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace('g_error = L"无法写出应用程序文件，请检查磁盘空间或权限。";',
              'DWORD writeErr = GetLastError();\n        g_error = L"无法写出应用程序文件（" + fileNameOf(app) + L"），错误码 " + std::to_wstring(writeErr) + L"。";')
p.write_text(s, encoding="utf-8", newline="\n")
print("write error detail added")
