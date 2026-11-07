from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""void App::invalidateLiveRect(const RECT& r) {
    InvalidateRect(hwnd_, &r, FALSE);
    ULONGLONG now = GetTickCount64();
    if (now - lastLivePaint_ >= 8) {
        lastLivePaint_ = now;
        if (hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
    }
}""",
"""void App::invalidateLiveRect(const RECT& r) {
    InvalidateRect(hwnd_, &r, FALSE);
}""")
s = s.replace("""    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;
    SetCapture(hwnd_);
    invalidateLive();
}""",
"""    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;
    SetCapture(hwnd_);
    SetTimer(hwnd_, 6, 8, nullptr);
    Pt s = worldToScreen(liveStroke_.points[0]);
    float pad = liveStroke_.width * viewScale() * 0.5f + S(6);
    RECT nr;
    nr.left = std::max(0L, (LONG)std::floor(s.x - pad));
    nr.top = std::max(0L, (LONG)std::floor(s.y - pad));
    nr.right = std::min((LONG)W_, (LONG)std::ceil(s.x + pad));
    nr.bottom = std::min((LONG)H_, (LONG)std::ceil(s.y + pad));
    liveDirty_ = nr;
    liveDirtyValid_ = true;
    invalidateLiveRect(nr);
}""")
# kill timer 6 at finish
s = s.replace("""    drag_ = DragMode::None;
    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;""",
"""    drag_ = DragMode::None;
    KillTimer(hwnd_, 6);
    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;""")
# timer 6
s = s.replace("""    } else if (id == 5) {""",
"""    } else if (id == 6) {
        if (drag_ == DragMode::Draw && hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
        else KillTimer(hwnd_, 6);
    } else if (id == 5) {""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (drag_ == DragMode::Draw && !baseDirty_ && base_ &&
        base_->GetWidth() == W_ && base_->GetHeight() == H_) {""",
"""    if (drag_ == DragMode::Draw && back_ &&
        back_->GetWidth() == W_ && back_->GetHeight() == H_) {""")
s = s.replace("""        presentBitmap(hdc, back_.get(), W_, H_);
        liveDirtyValid_ = false;
        return;
    }
    if (drag_ == DragMode::Move && sel_ >= 0""",
"""        RECT rc{};
        if (dirty) rc = *dirty;
        rc.left = std::max(0L, rc.left);
        rc.top = std::max(0L, rc.top);
        rc.right = std::min((LONG)W_, rc.right);
        rc.bottom = std::min((LONG)H_, rc.bottom);
        bool fullRect = !dirty || (rc.left <= 0 && rc.top <= 0 && rc.right >= W_ && rc.bottom >= H_);
        if (!fullRect && rc.right > rc.left && rc.bottom > rc.top)
            presentBitmapRect(hdc, back_.get(), rc, W_, H_);
        else
            presentBitmap(hdc, back_.get(), W_, H_);
        liveDirtyValid_ = false;
        return;
    }
    if (drag_ == DragMode::Move && sel_ >= 0""")
p.write_text(s, encoding="utf-8", newline="\n")
print("decoupled ink paint timer and dirty present")
