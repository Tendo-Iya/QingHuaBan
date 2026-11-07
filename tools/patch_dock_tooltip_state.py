from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    std::unique_ptr<Bitmap> pageContentBmp_;",
              "    std::unique_ptr<Bitmap> pageContentBmp_;\n    std::unique_ptr<Bitmap> dockDragBase_;")
s = s.replace("    bool onChar(wchar_t ch);", "    bool onChar(wchar_t ch);\n    void openFunctionDialog();")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\liquid.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""void App::startDockDrag(Pt screen) {""",
"""void App::startDockDrag(Pt screen) {
    dockDragBase_.reset();""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (hEdit_) commitText();
    if (hNameEdit_) commitAppName();
    clampCam();""",
"""    if (hEdit_) commitText();
    if (hNameEdit_) commitAppName();
    dockDragBase_.reset();
    clampCam();""")
# shortcut F + open function method
s = s.replace("""    if (hEdit_) return false;
    if (ctrl) {""",
"""    if (hEdit_) return false;
    if (!ctrl && vk == 'F') { openFunctionDialog(); return true; }
    if (ctrl) {""")
s = s.replace("""void App::insertFunctionGraph() {""",
"""void App::openFunctionDialog() {
    fnExpr_.clear();
    fnError_.clear();
    bgPanel_ = false;
    overlay_ = 4;
    invalidate();
}

void App::insertFunctionGraph() {""")
# tooltip timer/delay
s = s.replace("if (!id.empty()) SetTimer(hwnd_, 2, 450, nullptr);",
              "if (!id.empty()) SetTimer(hwnd_, 2, 60, nullptr);")
s = s.replace("""    } else if (id == 2) {
        KillTimer(hwnd_, 2);
        invalidate();
    }""",
"""    } else if (id == 2) {
        KillTimer(hwnd_, 2);
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
    }""")
s = s.replace("""        if (toolAnimStart_ != 0) {
            had = true;""",
"""        {
            ULONGLONG elapsed = now - hoverSince_;
            bool tip = !hoverId_.empty() && mouseInside_ && elapsed >= 45 && elapsed < 190;
            if (tip) had = true;
        }
        if (toolAnimStart_ != 0) {
            had = true;""")
p.write_text(s, encoding="utf-8", newline="\n")
print("dock cache/tooltip keys patched")
