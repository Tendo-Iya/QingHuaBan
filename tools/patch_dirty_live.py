from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    void paint(HDC hdc);", "    void paint(HDC hdc, const RECT* dirty = nullptr);")
s = s.replace("    void invalidateLive();", "    void invalidateLive();\n    void invalidateLiveRect(const RECT& r);")
s = s.replace("    bool baseDirty_ = true;", "    bool baseDirty_ = true;\n    bool liveDirtyValid_ = false;\n    RECT liveDirty_{};")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\main.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("if (g_app) g_app->paint(hdc);", "if (g_app) g_app->paint(hdc, &ps.rcPaint);")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
# add invalidateLiveRect after invalidateLive definition
s = s.replace("""void App::invalidateLive() {
    InvalidateRect(hwnd_, nullptr, FALSE);
    ULONGLONG now = GetTickCount64();
    if (now - lastLivePaint_ >= 10) {
        lastLivePaint_ = now;
        if (hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
    }
}""",
"""void App::invalidateLive() {
    InvalidateRect(hwnd_, nullptr, FALSE);
    ULONGLONG now = GetTickCount64();
    if (now - lastLivePaint_ >= 10) {
        lastLivePaint_ = now;
        if (hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
    }
}

void App::invalidateLiveRect(const RECT& r) {
    InvalidateRect(hwnd_, &r, FALSE);
    ULONGLONG now = GetTickCount64();
    if (now - lastLivePaint_ >= 8) {
        lastLivePaint_ = now;
        if (hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
    }
}""")
# dirty tracking on draw move
s = s.replace("""            if (liveStroke_.points.empty() ||
                std::hypot(c.x - liveStroke_.points.back().x, c.y - liveStroke_.points.back().y) >
                    1.0f / viewScale()) {
                liveStroke_.points.push_back(c);
                invalidateLive();
            }""",
"""            if (liveStroke_.points.empty() ||
                std::hypot(c.x - liveStroke_.points.back().x, c.y - liveStroke_.points.back().y) >
                    1.0f / viewScale()) {
                Pt prev = liveStroke_.points.empty() ? c : liveStroke_.points.back();
                liveStroke_.points.push_back(c);
                Pt a = worldToScreen(prev);
                Pt b = worldToScreen(c);
                float pad = liveStroke_.width * viewScale() * 0.5f + S(6);
                RECT nr;
                nr.left = (LONG)std::floor(std::min(a.x, b.x) - pad);
                nr.top = (LONG)std::floor(std::min(a.y, b.y) - pad);
                nr.right = (LONG)std::ceil(std::max(a.x, b.x) + pad);
                nr.bottom = (LONG)std::ceil(std::max(a.y, b.y) + pad);
                nr.left = std::max(0L, nr.left);
                nr.top = std::max(0L, nr.top);
                nr.right = std::min((LONG)W_, nr.right);
                nr.bottom = std::min((LONG)H_, nr.bottom);
                if (liveDirtyValid_) {
                    liveDirty_.left = std::min(liveDirty_.left, nr.left);
                    liveDirty_.top = std::min(liveDirty_.top, nr.top);
                    liveDirty_.right = std::max(liveDirty_.right, nr.right);
                    liveDirty_.bottom = std::max(liveDirty_.bottom, nr.bottom);
                } else {
                    liveDirty_ = nr;
                    liveDirtyValid_ = true;
                }
                invalidateLiveRect(liveDirty_);
            }""")
# reset dirty at start draw
s = s.replace("""    liveStroke_.points.push_back({clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)});
    SetCapture(hwnd_);
    invalidateLive();""",
"""    liveStroke_.points.push_back({clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)});
    liveDirtyValid_ = false;
    SetCapture(hwnd_);
    invalidateLive();""")
# reset at finish drag
s = s.replace("""    drag_ = DragMode::None;
    moveBase_.reset();""",
"""    drag_ = DragMode::None;
    liveDirtyValid_ = false;
    moveBase_.reset();""")
p.write_text(s, encoding="utf-8", newline="\n")
print("dirty live invalidation")
