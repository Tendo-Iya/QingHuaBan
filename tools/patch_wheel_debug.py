from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (overlay_ == 1) {
        float maxS = std::max(0.0f, pageScrollMax_);
        float base = pageScrollAnimating_ ? pageScrollTo_ : pageScroll_;""",
"""    if (overlay_ == 1) {
        FILE* df = fopen("C:\\\\Users\\\\15797\\\\Documents\\\\deepseek-harness\\\\default-workspace\\\\QingHuaBan\\\\build\\\\wheel_debug.txt", "a");
        if (df) { fprintf(df, "delta=%d max=%.1f scroll=%.1f anim=%d\\n", delta, pageScrollMax_, pageScroll_, pageScrollAnimating_ ? 1 : 0); fclose(df); }
        float maxS = std::max(0.0f, pageScrollMax_);
        float base = pageScrollAnimating_ ? pageScrollTo_ : pageScroll_;""")
s = s.replace("""        pageScrollAnimStart_ = GetTickCount64();
        pageScrollAnimating_ = true;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
        return;
    }
    if (overlay_ || splash_) return;""",
"""        pageScrollAnimStart_ = GetTickCount64();
        pageScrollAnimating_ = true;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
        FILE* df2 = fopen("C:\\\\Users\\\\15797\\\\Documents\\\\deepseek-harness\\\\default-workspace\\\\QingHuaBan\\\\build\\\\wheel_debug.txt", "a");
        if (df2) { fprintf(df2, "  -> from=%.1f to=%.1f\\n", pageScrollFrom_, pageScrollTo_); fclose(df2); }
        return;
    }
    if (overlay_ || splash_) return;""")
p.write_text(s, encoding="utf-8", newline="\n")
print("wheel debug patched")
