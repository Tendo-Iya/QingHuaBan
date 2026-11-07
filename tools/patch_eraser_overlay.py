from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    void addLiveSample(Pt world);",
              "    void addLiveSample(Pt world);\n    void addEraserSample(Pt world);\n    bool eraseGeometry(Pt world);")
s = s.replace("    bool liveDirtyValid_ = false;",
              "    bool liveDirtyValid_ = false;\n    std::vector<Pt> eraserPath_;\n    size_t eraserDrawn_ = 0;")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# start eraser overlay
s = s.replace("""void App::startErase(Pt world) {
    drag_ = DragMode::Erase;
    pendingSnapshot_ = snapshot();
    pendingSnapshotValid_ = true;
    eraserChanged_ = false;
    dragLastWorld_ = world;
    eraseAt(world);
    SetCapture(hwnd_);
    invalidate();
}""",
"""void App::startErase(Pt world) {
    drag_ = DragMode::Erase;
    pendingSnapshot_ = snapshot();
    pendingSnapshotValid_ = true;
    eraserChanged_ = false;
    eraserPath_.clear();
    eraserPath_.push_back(world);
    eraserDrawn_ = 0;
    dragLastWorld_ = world;
    SetCapture(hwnd_);
    SetTimer(hwnd_, 6, 8, nullptr);
}

void App::addEraserSample(Pt world) {
    if (drag_ != DragMode::Erase) return;
    Pt c{clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)};
    if (!eraserPath_.empty()) {
        Pt last = eraserPath_.back();
        float dist = std::hypot(c.x - last.x, c.y - last.y);
        if (dist < 0.4f / viewScale()) return;
        int steps = std::max(1, (int)std::ceil(dist / std::max(2.0f, eraserSize_ * 0.35f)));
        for (int i = 1; i <= steps; ++i) {
            float t = (float)i / steps;
            eraserPath_.push_back({last.x + (c.x - last.x) * t,
                                   last.y + (c.y - last.y) * t});
        }
    } else {
        eraserPath_.push_back(c);
    }
    dragLastWorld_ = c;
}""")

# rename eraseAt to eraseGeometry and remove side effects
s = s.replace("void App::eraseAt(Pt world) {", "bool App::eraseGeometry(Pt world) {")
# after loop changed check and end: replace side-effect tail
s = s.replace("""    if (!changed) return;
    if (!eraserChanged_ && pendingSnapshotValid_) {
        pushUndoSnapshot(pendingSnapshot_);
        eraserChanged_ = true;
    }
    page().items = std::move(out);
    page().touch();
    sel_ = -1;
    markDirty();
    invalidate();
}""",
"""    if (!changed) return false;
    page().items = std::move(out);
    sel_ = -1;
    return true;
}""")

# replace old drag Erase handling with path append
s = s.replace("""        } else if (drag_ == DragMode::Erase) {
            float dist = std::hypot(wp.x - dragLastWorld_.x, wp.y - dragLastWorld_.y);
            int steps = std::max(1, (int)std::ceil(dist / std::max(4.0f, eraserSize_ * 0.25f)));
            for (int i = 1; i <= steps; ++i) {
                float t = (float)i / steps;
                eraseAt({dragLastWorld_.x + (wp.x - dragLastWorld_.x) * t,
                         dragLastWorld_.y + (wp.y - dragLastWorld_.y) * t});
            }
            dragLastWorld_ = wp;
            invalidate();
        } else if (drag_ == DragMode::Move) {""",
"""        } else if (drag_ == DragMode::Erase) {
            addEraserSample(wp);
        } else if (drag_ == DragMode::Move) {""")

# finish eraser commit
s = s.replace("""    } else if (drag_ == DragMode::Erase) {
        if (eraserChanged_) {
            markDirty();
            page().touch();
        }
        pendingSnapshotValid_ = false;
        eraserChanged_ = false;
    } else if (drag_ == DragMode::Move || drag_ == DragMode::Resize) {""",
"""    } else if (drag_ == DragMode::Erase) {
        KillTimer(hwnd_, 6);
        bool changed = false;
        for (const Pt& p : eraserPath_) {
            if (eraseGeometry(p)) changed = true;
        }
        if (changed) {
            if (pendingSnapshotValid_) pushUndoSnapshot(pendingSnapshot_);
            page().touch();
            markDirty();
            eraserChanged_ = true;
        }
        eraserPath_.clear();
        eraserDrawn_ = 0;
        pendingSnapshotValid_ = false;
    } else if (drag_ == DragMode::Move || drag_ == DragMode::Resize) {""")

# timer6 supports eraser and samples
s = s.replace("""    } else if (id == 6) {
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
        }""",
"""    } else if (id == 6) {
        bool draw = drag_ == DragMode::Draw;
        bool erase = drag_ == DragMode::Erase;
        if ((draw || erase) && hwnd_ && IsWindowVisible(hwnd_)) {
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
                Pt w = screenToWorld({(float)pt.x, (float)pt.y});
                if (draw) addLiveSample(w);
                else addEraserSample(w);
            }
            UpdateWindow(hwnd_);
        } else {
            KillTimer(hwnd_, 6);
        }""")
p.write_text(s, encoding="utf-8", newline="\n")
print("live eraser overlay + batched commit")
