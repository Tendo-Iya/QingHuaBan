from pathlib import Path

# app resources
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\assets\app.rc")
s = p.read_text(encoding="utf-8")
s = s.replace("FILEVERSION 1,0,0,0", "FILEVERSION 1,2,0,1")
s = s.replace("PRODUCTVERSION 1,0,0,0", "PRODUCTVERSION 1,2,0,1")
s = s.replace('VALUE "FileVersion", "1.0.0.0"', 'VALUE "FileVersion", "1.2.0.1"')
s = s.replace('VALUE "ProductVersion", "1.0.0.0"', 'VALUE "ProductVersion", "1.2.0.1"')
p.write_text(s, encoding="utf-8", newline="\n")

# installer resources
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.rc")
s = p.read_text(encoding="utf-8")
s = s.replace("FILEVERSION 1,0,0,0", "FILEVERSION 1,2,0,1")
s = s.replace("PRODUCTVERSION 1,0,0,0", "PRODUCTVERSION 1,2,0,1")
s = s.replace('VALUE "FileVersion", "1.0.0.0"', 'VALUE "FileVersion", "1.2.0.1"')
s = s.replace('VALUE "ProductVersion", "1.0.0.0"', 'VALUE "ProductVersion", "1.2.0.1"')
p.write_text(s, encoding="utf-8", newline="\n")

# installer display version
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace('set(L"DisplayVersion", L"1.0.0");', 'set(L"DisplayVersion", L"1.2.0.1");')
p.write_text(s, encoding="utf-8", newline="\n")

# usage version
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\HAVEGET-白板-发布\使用说明.txt")
s = p.read_text(encoding="utf-8")
s = s.replace("HAVEGET白板 v1.0  ·  HAVEGET工作室", "HAVEGET白板 v1.2.0.1  ·  HAVEGET工作室")
p.write_text(s, encoding="utf-8", newline="\n")
print("version 1.2.0.1 patched")
