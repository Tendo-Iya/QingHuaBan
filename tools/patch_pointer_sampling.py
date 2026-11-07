from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    void invalidateLiveRect(const RECT& r);",
              "    void invalidateLiveRect(const RECT& r);\n    void addLiveSample(Pt world);")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
# Insert addLiveSample before startDraw
anchor = "void App::startDraw(Pt world) {"
helper = r'''void App::addLiveSample(Pt world) {
    if (drag_ != DragMode::Draw) return;
    Pt c{clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)};
    if (!liveStroke_.points.empty()) {
        Pt last = liveStroke_.points.back();
        if (std::hypot(c.x - last.x, c.y - last.y) <= 0.7f / viewScale()) return;
    }
    Pt prev = liveStroke_.points.empty() ? c : liveStroke_.points.back();
    liveStroke_.points.push_back(c);
    Pt a = worldToScreen(prev);
    Pt b = worldToScreen(c);
    float pad = liveStroke_.width * viewScale() * 0.5f + S(6);
    RECT nr;
    nr.left = std::max(0L, (LONG)std::floor(std::min(a.x, b.x) - pad));
    nr.top = std::max(0L, (LONG)std::floor(std::min(a.y, b.y) - pad));
    nr.right = std::min((LONG)W_, (LONG)std::ceil(std::max(a.x, b.x) + pad));
    nr.bottom = std::min((LONG)H_, (LONG)std::ceil(std::max(a.y, b.y) + pad));
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
}

'''
s = s.replace(anchor, helper + anchor, 1)
# Replace onMouseMove draw point block
old = """        if (drag_ == DragMode::Draw) {
            Pt c{clampf(wp.x, 1, PAGE_W - 1), clampf(wp.y, 1, PAGE_H - 1)};
            if (liveStroke_.points.empty() ||
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
            }
        } else if (drag_ == DragMode::Shape) {"""
new = """        if (drag_ == DragMode::Draw) {
            addLiveSample(wp);
        } else if (drag_ == DragMode::Shape) {"""
if old not in s: raise SystemExit("draw block not found")
s = s.replace(old, new, 1)
# timer6 sample current pointer before paint
s = s.replace("""    } else if (id == 6) {
        if (drag_ == DragMode::Draw && hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
        else KillTimer(hwnd_, 6);""",
"""    } else if (id == 6) {
        if (drag_ == DragMode::Draw && hwnd_ && IsWindowVisible(hwnd_)) {
            POINT pt{};
            bool got = false;
            if (pointerActive_) {
                POINTER_INFO pi{};
                if (GetPointerInfo(activePointerId_, &pi)) {
                    pt.x = pi.ptPixelLocation.x;
                    pt.y = pi.ptPixelLocation.y;
                    got = true;
                }
            } else if (GetCursorPos(&pt)) {
                got = true;
            }
            if (got) {
                ScreenToClient(hwnd_, &pt);
                addLiveSample(screenToWorld({(float)pt.x, (float)pt.y}));
            }
            UpdateWindow(hwnd_);
        } else {
            KillTimer(hwnd_, 6);
        }""")
p.write_text(s, encoding="utf-8", newline="\n")
print("trajectory pointer sampling")
