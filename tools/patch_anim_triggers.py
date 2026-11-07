from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# splash timer faster
s = s.replace("SetTimer(hwnd_, 4, 33, nullptr);", "SetTimer(hwnd_, 4, 16, nullptr);")

# page fade on navigation
s = s.replace("""    current_ = index;
    sel_ = -1;
    fitView();
    if (closeOverlay) overlay_ = 0;
    invalidate();""",
"""    current_ = index;
    sel_ = -1;
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    if (closeOverlay) overlay_ = 0;
    invalidate();""")

s = s.replace("""    current_ = current_ + 1;
    sel_ = -1;
    fitView();
    showToast(L"已新建第 " + std::to_wstring(current_ + 1) + L" 页", false, 1400);""",
"""    current_ = current_ + 1;
    sel_ = -1;
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    showToast(L"已新建第 " + std::to_wstring(current_ + 1) + L" 页", false, 1400);""")

s = s.replace("""    current_ = current_ + 1;
    sel_ = -1;
    fitView();
    showToast(L"已复制页面", false, 1400);""",
"""    current_ = current_ + 1;
    sel_ = -1;
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    showToast(L"已复制页面", false, 1400);""")

s = s.replace("""    if (current_ >= (int)pages_.size()) current_ = (int)pages_.size() - 1;
    sel_ = -1;
    fitView();
    showToast(L"已删除页面", false, 1400);""",
"""    if (current_ >= (int)pages_.size()) current_ = (int)pages_.size() - 1;
    sel_ = -1;
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    showToast(L"已删除页面", false, 1400);""")

# newDocument fade
s = s.replace("""    clearImageCache();
    fitView();
    updateWindowTitle();
    invalidate();
    showToast(L"已新建空白白板", false, 1600);""",
"""    clearImageCache();
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    updateWindowTitle();
    invalidate();
    showToast(L"已新建空白白板", false, 1600);""")

# showToast slide
s = s.replace("""void App::showToast(const std::wstring& text, bool error, int ms) {
    toast_ = text;
    toastError_ = error;
    toastUntil_ = GetTickCount64() + (ULONGLONG)ms;
    SetTimer(hwnd_, 1, (UINT)ms + 30, nullptr);
    invalidate();""",
"""void App::showToast(const std::wstring& text, bool error, int ms) {
    toast_ = text;
    toastError_ = error;
    toastUntil_ = GetTickCount64() + (ULONGLONG)ms;
    toastAnimStart_ = GetTickCount64();
    SetTimer(hwnd_, 1, (UINT)ms + 30, nullptr);
    SetTimer(hwnd_, 5, 16, nullptr);
    invalidate();""")

# settings open
s = s.replace("""    if (id == "dock:settings" || id == "settings:open") {
        commitText();
        overlay_ = 3;
        bgPanel_ = false;
        invalidate();
        return;
    }""",
"""    if (id == "dock:settings" || id == "settings:open") {
        commitText();
        overlay_ = 3;
        settingsAnim_ = 0.0f;
        settingsClosing_ = false;
        settingsAnimStart_ = GetTickCount64();
        bgPanel_ = false;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
        return;
    }""")

# settings animated close before generic overlay close
s = s.replace("""    if (id == "ov:pages:close" || id == "ov:help:close" || id == "ov:scrim" || id == "settings:close") {
        commitAppName();
        overlay_ = 0;
        invalidate();
        return;
    }""",
"""    if ((id == "settings:close" || id == "ov:scrim") && overlay_ == 3) {
        commitAppName();
        settingsClosing_ = true;
        settingsAnimStart_ = GetTickCount64();
        SetTimer(hwnd_, 5, 16, nullptr);
        return;
    }
    if (id == "ov:pages:close" || id == "ov:help:close" || id == "ov:scrim" || id == "settings:close") {
        commitAppName();
        overlay_ = 0;
        invalidate();
        return;
    }""")

# Escape settings animated close
s = s.replace("""        if (overlay_) {
            if (overlay_ == 3) commitAppName();
            overlay_ = 0;
        }""",
"""        if (overlay_) {
            if (overlay_ == 3) {
                commitAppName();
                settingsClosing_ = true;
                settingsAnimStart_ = GetTickCount64();
                SetTimer(hwnd_, 5, 16, nullptr);
            } else {
                overlay_ = 0;
            }
        }""")

# ignore settings input during transition
s = s.replace("""    if (overlay_ == 3) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "settings:") || rid == "ov:scrim" || rid == "settings:close") {
            handleRegion(rid);
        }
        return;
    }""",
"""    if (overlay_ == 3) {
        if (settingsClosing_ || settingsAnim_ < 0.96f) return;
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "settings:") || rid == "ov:scrim" || rid == "settings:close") {
            handleRegion(rid);
        }
        return;
    }""")

p.write_text(s, encoding="utf-8", newline="\n")
print("animation triggers patched")
