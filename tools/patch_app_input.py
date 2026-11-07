from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

s = s.replace("""    if (hEdit_) commitText();
    clampCam();
    invalidate();
}""",
"""    if (hEdit_) commitText();
    if (hNameEdit_) commitAppName();
    clampCam();
    invalidate();
}""")

s = s.replace("""void App::onDpiChanged(UINT dpi, const RECT* suggested) {
    commitText();""",
"""void App::onDpiChanged(UINT dpi, const RECT* suggested) {
    commitText();
    commitAppName();""")

old_down = """void App::onLButtonDown(int x, int y) {
    if (hEdit_) {
        std::string editRegion = regionAt(x, y);
        commitText();
        if (editRegion == "text:done") {
            invalidate();
            return;
        }
    }
    SetFocus(hwnd_);
    mouse_ = {x, y};
    mouseInside_ = true;

    if (overlay_) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "ov:")) handleRegion(rid);
        return;
    }
    if (bgPanel_) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "bg:")) {
            handleRegion(rid);
            return;
        }
        bgPanel_ = false;
        invalidate();
        return;
    }
    std::string rid = regionAt(x, y);
    if (!rid.empty()) {
        handleRegion(rid);
        return;
    }
    canvasMouseDown(x, y);
}"""
new_down = """void App::onLButtonDown(int x, int y) {
    if (splash_) {
        enterBoard();
        return;
    }
    mouse_ = {x, y};
    mouseInside_ = true;

    if (hNameEdit_) {
        std::string rid0 = regionAt(x, y);
        if (rid0 == "settings:name_save") {
            commitAppName();
            handleRegion(rid0);
            return;
        }
        commitAppName();
    }
    if (hEdit_) {
        std::string editRegion = regionAt(x, y);
        commitText();
        if (editRegion == "text:done") {
            invalidate();
            return;
        }
    }
    SetFocus(hwnd_);

    if (overlay_ == 3) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "settings:") || rid == "ov:scrim" || rid == "settings:close") {
            handleRegion(rid);
        }
        return;
    }
    if (overlay_) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "ov:")) handleRegion(rid);
        return;
    }
    if (bgPanel_) {
        bgPanel_ = false;
        invalidate();
        return;
    }
    std::string rid = regionAt(x, y);
    if (rid == "dock:grip") {
        startDockDrag({(float)x, (float)y});
        return;
    }
    if (!rid.empty()) {
        handleRegion(rid);
        return;
    }
    canvasMouseDown(x, y);
}"""
if old_down not in s:
    raise SystemExit("onLButtonDown anchor missing")
s = s.replace(old_down, new_down)

s = s.replace("""    } else if (drag_ == DragMode::Laser) {
        laserDown_ = false;
        lastLaserTime_ = GetTickCount64();
    }
    drag_ = DragMode::None;""",
"""    } else if (drag_ == DragMode::Laser) {
        laserDown_ = false;
        lastLaserTime_ = GetTickCount64();
    } else if (drag_ == DragMode::Dock) {
        saveConfig();
    }
    drag_ = DragMode::None;""")

s = s.replace("""    Pt sp{(float)x, (float)y};
    Pt wp = screenToWorld(sp);

    if (drag_ != DragMode::None) {""",
"""    Pt sp{(float)x, (float)y};
    Pt wp = screenToWorld(sp);

    if (drag_ == DragMode::Dock) {
        float cx = dockDragStartCenter_.x + (sp.x - dockDragStart_.x);
        float cy = dockDragStartCenter_.y + (sp.y - dockDragStart_.y);
        dockDefault_ = false;
        dockNormX_ = clampf(cx / std::max(1.0f, (float)W_), 0.0f, 1.0f);
        dockNormY_ = clampf(cy / std::max(1.0f, (float)H_), 0.0f, 1.0f);
        updateCursor();
        invalidate();
        return;
    }

    if (drag_ != DragMode::None) {""")

# Keyboard: splash skip + settings close + background tool already handled by earlier patch
s = s.replace("""bool App::onKeyDown(WPARAM vk, LPARAM) {
    commitText();""",
"""bool App::onKeyDown(WPARAM vk, LPARAM) {
    if (splash_) {
        enterBoard();
        return true;
    }
    commitText();""")

s = s.replace("""    if (vk == VK_ESCAPE) {
        if (overlay_) overlay_ = 0;
        else if (bgPanel_) bgPanel_ = false;""",
"""    if (vk == VK_ESCAPE) {
        if (overlay_) {
            if (overlay_ == 3) commitAppName();
            overlay_ = 0;
        }
        else if (bgPanel_) bgPanel_ = false;""")

# Fix settings color prefix length
s = s.replace('applyCanvasColor(hexColor(id.substr(21)), applyAllPages_);',
              'applyCanvasColor(hexColor(id.substr(22)), applyAllPages_);')

p.write_text(s, encoding="utf-8", newline="\n")
print("input patched")
