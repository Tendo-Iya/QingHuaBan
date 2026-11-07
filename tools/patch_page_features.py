from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    bool pageScrollDrag_ = false;", "    bool pageScrollDrag_ = false;\n    bool pageScrollToCurrent_ = false;")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# setTool transition
s = s.replace("""void App::setTool(Tool t) {
    tool_ = t;
    bgPanel_ = false;
    sel_ = -1;
    invalidate();
}""",
"""void App::setTool(Tool t) {
    bool changed = tool_ != t;
    tool_ = t;
    bgPanel_ = false;
    sel_ = -1;
    if (changed) {
        toolAnim_ = 0.0f;
        toolAnimStart_ = GetTickCount64();
        SetTimer(hwnd_, 5, 16, nullptr);
    }
    invalidate();
}""")

# remove new document handler
s = s.replace('    if (id == "dock:new") { newDocument(); return; }\n', '')

# page navigation auto-scroll
s = s.replace('if (id == "page:label") { overlay_ = 1; bgPanel_ = false; pageScroll_ = 0; invalidate(); return; }',
              'if (id == "page:label") { overlay_ = 1; bgPanel_ = false; pageScrollToCurrent_ = true; invalidate(); return; }')

# addPage: auto scroll + fade
s = s.replace("""    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    showToast(L"已新建第 " + std::to_wstring(current_ + 1) + L" 页", false, 1400);""",
"""    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    if (overlay_ == 1) pageScrollToCurrent_ = true;
    showToast(L"已新建第 " + std::to_wstring(current_ + 1) + L" 页", false, 1400);""")

# movePage implementation after addPage
anchor = """void App::duplicatePage() {"""
move = r'''void App::movePage(int from, int to) {
    if (from < 0 || to < 0 || from >= pageCount() || to >= pageCount() || from == to) return;
    pushUndo();
    Page p = pages_[from];
    pages_.erase(pages_.begin() + from);
    pages_.insert(pages_.begin() + to, p);
    if (current_ == from) current_ = to;
    else if (from < current_ && to >= current_) --current_;
    else if (from > current_ && to <= current_) ++current_;
    sel_ = -1;
    dirty_ = true;
    updateWindowTitle();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    invalidate();
    showToast(L"页面顺序已调整", false, 1200);
}

void App::duplicatePage() {'''
if anchor not in s: raise SystemExit("duplicate anchor missing")
s = s.replace(anchor, move, 1)

# wheel: guard UI/settings and page scroll clamp
s = s.replace("""void App::onMouseWheel(int x, int y, int delta, bool ctrl, bool shift) {
    if (overlay_ == 1) {
        pageScroll_ -= (float)delta / 120.0f * S(90);
        if (pageScroll_ < 0) pageScroll_ = 0;
        invalidate();
        return;
    }
    if (overlay_ == 2) return;
    Pt sp{(float)x, (float)y};""",
"""void App::onMouseWheel(int x, int y, int delta, bool ctrl, bool shift) {
    if (overlay_ == 1) {
        pageScroll_ -= (float)delta / 120.0f * S(90);
        pageScroll_ = std::max(0.0f, std::min(pageScroll_, std::max(0.0f, pageScrollMax_)));
        invalidate();
        return;
    }
    if (overlay_ || splash_) return;
    if (dockRect().Contains((float)x, (float)y)) return;
    Pt sp{(float)x, (float)y};""")

# overlay1 handling on left down before generic overlay
s = s.replace("""    if (overlay_) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "ov:")) handleRegion(rid);
        return;
    }
    if (bgPanel_) {""",
"""    if (overlay_ == 1) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "ov:pages:page:")) {
            int idx = std::atoi(rid.c_str() + 14);
            pageDragFrom_ = idx;
            pageDragTo_ = idx;
            pageDragActive_ = false;
            pageDragStart_ = {(float)x, (float)y};
            SetCapture(hwnd_);
            return;
        }
        if (rid == "ov:pages:scroll") {
            pageScrollDrag_ = true;
            pageScrollDragStartY_ = (float)y;
            pageScrollDragStart_ = pageScroll_;
            SetCapture(hwnd_);
            return;
        }
        if (startsWith(rid, "ov:")) handleRegion(rid);
        return;
    }
    if (overlay_) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "ov:")) handleRegion(rid);
        return;
    }
    if (bgPanel_) {""")

# page drag handling on mouse move before drag modes
s = s.replace("""    Pt sp{(float)x, (float)y};
    Pt wp = screenToWorld(sp);

    if (drag_ == DragMode::Dock) {""",
"""    Pt sp{(float)x, (float)y};
    Pt wp = screenToWorld(sp);

    if (overlay_ == 1 && pageDragFrom_ >= 0) {
        if (!pageDragActive_ &&
            std::hypot(sp.x - pageDragStart_.x, sp.y - pageDragStart_.y) > S(5)) {
            pageDragActive_ = true;
        }
        if (pageDragActive_) {
            std::string rid = regionAt(x, y);
            if (startsWith(rid, "ov:pages:page:")) {
                pageDragTo_ = std::atoi(rid.c_str() + 14);
            } else {
                pageDragTo_ = -1;
            }
            invalidate();
        }
        return;
    }
    if (overlay_ == 1 && pageScrollDrag_) {
        float denom = std::max(S(1), pageScrollTrackH_ - pageScrollThumbH_);
        pageScroll_ = pageScrollDragStart_ +
                      (sp.y - pageScrollDragStartY_) * pageScrollMax_ / denom;
        pageScroll_ = std::max(0.0f, std::min(pageScroll_, pageScrollMax_));
        invalidate();
        return;
    }

    if (drag_ == DragMode::Dock) {""")

# page drag finish on left up
s = s.replace("""void App::onLButtonUp(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (drag_ == DragMode::None) return;""",
"""void App::onLButtonUp(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (overlay_ == 1 && pageDragFrom_ >= 0) {
        int from = pageDragFrom_;
        int to = pageDragTo_;
        bool active = pageDragActive_;
        pageDragFrom_ = pageDragTo_ = -1;
        pageDragActive_ = false;
        if (GetCapture() == hwnd_) ReleaseCapture();
        if (active && to >= 0 && to != from) movePage(from, to);
        else if (!active && from >= 0) {
            switchPage(from);
            overlay_ = 0;
        }
        invalidate();
        return;
    }
    if (overlay_ == 1 && pageScrollDrag_) {
        pageScrollDrag_ = false;
        if (GetCapture() == hwnd_) ReleaseCapture();
        invalidate();
        return;
    }
    if (drag_ == DragMode::None) return;""")

# remove Ctrl+N
s = s.replace("        if (vk == 'N') { newDocument(); return true; }\n", "")

# tool anim timer
s = s.replace("""        if (toastAnimStart_ != 0 && now - toastAnimStart_ < 180) had = true;
        if (had) invalidate();""",
"""        if (toolAnimStart_ != 0) {
            had = true;
            float t = std::min(1.0f, (float)(now - toolAnimStart_) / 140.0f);
            toolAnim_ = easeOutCubic(t);
            if (t >= 1.0f) {
                toolAnim_ = 1.0f;
                toolAnimStart_ = 0;
            }
        }
        if (toastAnimStart_ != 0 && now - toastAnimStart_ < 180) had = true;
        if (had) invalidate();""")

p.write_text(s, encoding="utf-8", newline="\n")
print("page reorder/scroll/new removal/tool anim patched")
