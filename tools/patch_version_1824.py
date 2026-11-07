from pathlib import Path

# Source version
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace('constexpr wchar_t APP_VERSION[] = L"1.2.0.1";',
              'constexpr wchar_t APP_VERSION[] = L"1.8.2.4";')
p.write_text(s, encoding="utf-8", newline="\n")

# App resources
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\assets\app.rc")
s = p.read_text(encoding="utf-8")
s = s.replace("FILEVERSION 1,2,0,1", "FILEVERSION 1,8,2,4")
s = s.replace("PRODUCTVERSION 1,2,0,1", "PRODUCTVERSION 1,8,2,4")
s = s.replace('"1.2.0.1"', '"1.8.2.4"')
p.write_text(s, encoding="utf-8", newline="\n")

# Installer resources / display version
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.rc")
s = p.read_text(encoding="utf-8")
s = s.replace("FILEVERSION 1,2,0,1", "FILEVERSION 1,8,2,4")
s = s.replace("PRODUCTVERSION 1,2,0,1", "PRODUCTVERSION 1,8,2,4")
s = s.replace('"1.2.0.1"', '"1.8.2.4"')
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace('set(L"DisplayVersion", L"1.2.0.1");', 'set(L"DisplayVersion", L"1.8.2.4");')
p.write_text(s, encoding="utf-8", newline="\n")

# Build copy/root versioned filename
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\build.ps1")
s = p.read_text(encoding="utf-8-sig")
s = s.replace("HAVEGET白板-v1.2.0.1.exe", "HAVEGET白板-v1.8.2.4.exe")
p.write_text(s, encoding="utf-8-sig", newline="\n")

# Developer names in developer options
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""        drawTextAligned(g, L"自定义启动器背景、左上角 Logo 和名称",
                        RectF(x, r.Y + S(316), r.Width - S(56), S(20)), fCapsule_, 0x5C6F65,
                        StringAlignmentNear, StringAlignmentCenter, false);
""",
"""        drawTextAligned(g, L"自定义启动器背景、左上角 Logo 和名称",
                        RectF(x, r.Y + S(316), r.Width - S(56), S(20)), fCapsule_, 0x5C6F65,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, L"开发者名单：田浚哲 · 吴晨曦",
                        RectF(x, r.Y + S(338), r.Width - S(56), S(18)), fCapsule_, 0x45594F,
                        StringAlignmentNear, StringAlignmentCenter, false);
""")
p.write_text(s, encoding="utf-8", newline="\n")
print("version 1.8.2.4 and developer names patched")
