from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("(toastAnimStart_ != 0 && animNow - toastAnimStart_ < 180);",
              "(toastAnimStart_ != 0 && animNow - toastAnimStart_ < 180) || toolAnimStart_ != 0;")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (overlay_ == 1) pageScrollToCurrent_ = true;
    showToast(L"已新建第 " + std::to_wstring(current_ + 1) + L" 页", false, 1400);""",
"""    if (overlay_ == 1) pageScrollToCurrent_ = true;
    else {
        pageFadeStart_ = GetTickCount64();
        SetTimer(hwnd_, 5, 16, nullptr);
    }
    showToast(L"已新建第 " + std::to_wstring(current_ + 1) + L" 页", false, 1400);""")
# remove unconditional pageFade earlier in addPage
s = s.replace("""    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    if (overlay_ == 1) pageScrollToCurrent_ = true;""",
"""    fitView();
    if (overlay_ == 1) pageScrollToCurrent_ = true;""")
s = s.replace("""    dirty_ = true;
    updateWindowTitle();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    invalidate();
    showToast(L"页面顺序已调整", false, 1200);""",
"""    dirty_ = true;
    updateWindowTitle();
    invalidate();
    showToast(L"页面顺序已调整", false, 1200);""")
p.write_text(s, encoding="utf-8", newline="\n")
print("animating/fade adjusted")
