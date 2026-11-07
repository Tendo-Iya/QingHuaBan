from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
old = '''    FILE* f = _wfopen(path.c_str(), L"wb");
    if (!f) return false;
    size_t n = fwrite(ptr, 1, sz, f);
    fclose(f);
    return n == sz;'''
new = '''    HANDLE hf = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hf == INVALID_HANDLE_VALUE) return false;
    DWORD wrote = 0;
    BOOL ok = WriteFile(hf, ptr, sz, &wrote, nullptr);
    CloseHandle(hf);
    return ok && wrote == sz;'''
if old not in s:
    raise SystemExit("writeResource anchor missing")
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("writeResource switched to Win32")
