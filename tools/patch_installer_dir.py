from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("    if (!fileExists(g_installDir)) {",
              "    DWORD dirAttr = GetFileAttributesW(g_installDir.c_str());\n    if (dirAttr == INVALID_FILE_ATTRIBUTES || !(dirAttr & FILE_ATTRIBUTE_DIRECTORY)) {")
p.write_text(s, encoding="utf-8", newline="\n")
print("dir check fixed")
