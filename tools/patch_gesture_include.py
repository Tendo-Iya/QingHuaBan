from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("    dpi_ = GetDpiForWindow(hwnd_);",
"""    dpi_ = GetDpiForWindow(hwnd_);
    GESTURECONFIG gcZero{GID_ZOOM, 0, GC_ZOOM};
    SetGestureConfig(hwnd_, 0, 1, &gcZero, sizeof(gcZero));""")
s = s.replace("#include <cmath>", "#include <cmath>\n#include <cstdio>")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("#include <memory>", "#include <memory>\n#include <algorithm>")
p.write_text(s, encoding="utf-8", newline="\n")
print("gesture/includes")
