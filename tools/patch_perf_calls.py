from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# invalidate + live helpers
s = s.replace("void App::invalidate() { InvalidateRect(hwnd_, nullptr, FALSE); }",
"""void App::invalidate() {
    baseDirty_ = true;
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void App::invalidateLive() { InvalidateRect(hwnd_, nullptr, FALSE); }

bool App::isLiveDrawing() const {
    if (splash_ || overlay_ != 0 || hEdit_) return false;
    if (drag_ == DragMode::Draw || drag_ == DragMode::Shape ||
        drag_ == DragMode::Laser || laserDown_ || !laser_.empty()) return true;
    if (tool_ == Tool::Eraser && mouseInside_) return true;
    return false;
}""")

# updateLaser
s = s.replace("""void App::updateLaser() {
    // Kept for API symmetry; trail upkeep happens in the timer.
    invalidate();
}""",
"""void App::updateLaser() {
    // Kept for API symmetry; trail upkeep happens in the timer.
    invalidateLive();
}""")

# mouse guards
s = s.replace("""void App::onMouseMove(int x, int y, WPARAM) {
    mouse_ = {x, y};""",
"""void App::onMouseMove(int x, int y, WPARAM) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    mouse_ = {x, y};""")

s = s.replace("""void App::onLButtonDown(int x, int y) {
    if (splash_) {""",
"""void App::onLButtonDown(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (splash_) {""")

s = s.replace("""void App::onLButtonUp(int x, int y) {
    if (drag_ == DragMode::None) return;""",
"""void App::onLButtonUp(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (drag_ == DragMode::None) return;""")

s = s.replace("""void App::onLButtonDblClk(int x, int y) {
    if (overlay_ == 3) {""",
"""void App::onLButtonDblClk(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (overlay_ == 3) {""")

# live invalidations
s = s.replace("""                liveStroke_.points.push_back(c);
                invalidate();""",
"""                liveStroke_.points.push_back(c);
                invalidateLive();""")
s = s.replace("""            invalidate();
        } else if (drag_ == DragMode::Erase) {""",
"""            invalidateLive();
        } else if (drag_ == DragMode::Erase) {""")
s = s.replace("""            invalidate();
        }
        return;
    }

    updateHoverTooltip();
    if (tool_ == Tool::Eraser) invalidate();""",
"""            invalidateLive();
        }
        return;
    }

    updateHoverTooltip();
    if (tool_ == Tool::Eraser) invalidateLive();""")

# laser timer
s = s.replace("""        if (laser_.empty() && !laserDown_) {
            KillTimer(hwnd_, 3);
        }
        invalidate();""",
"""        if (laser_.empty() && !laserDown_) {
            KillTimer(hwnd_, 3);
        }
        invalidateLive();""")

p.write_text(s, encoding="utf-8", newline="\n")
print("perf invalidations patched")
