from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

s = s.replace("""void App::handleRegion(const std::string& id) {
    if (id.empty() || id == "ov:panel" || id == "ov:scrim:keep") return;
    commitText();

    if (id == "tool:select")""",
"""void App::handleRegion(const std::string& id) {
    if (id.empty() || id == "ov:panel" || id == "ov:scrim:keep") return;
    commitText();
    if (hNameEdit_ && id != "settings:name_save") commitAppName();

    if (id == "tool:select")""")

s = s.replace("""    if (id == "tool:hand" || id == "rail:hand") { setTool(Tool::Hand); return; }
    if (id == "rail:image") { insertImageDialog(); return; }
    if (id == "rail:bg") {
        bgPanel_ = !bgPanel_;
        overlay_ = 0;
        invalidate();
        return;
    }
    if (id == "header:undo") { undo(); return; }
    if (id == "header:redo") { redo(); return; }
    if (id == "header:open") { openDocumentDialog(); return; }
    if (id == "header:save") { saveDocument(false); return; }
    if (id == "header:export") { exportPng(); return; }
    if (id == "header:fullscreen") { toggleFullscreen(); return; }
    if (id == "header:help") { overlay_ = 2; bgPanel_ = false; invalidate(); return; }""",
"""    if (id == "tool:hand" || id == "rail:hand") { setTool(Tool::Hand); return; }
    if (id == "tool:background" || id == "rail:bg") { setTool(Tool::Background); return; }
    if (id == "rail:image" || id == "dock:image") { insertImageDialog(); return; }
    if (id == "header:undo" || id == "dock:undo") { undo(); return; }
    if (id == "header:redo" || id == "dock:redo") { redo(); return; }
    if (id == "header:open" || id == "dock:open") { openDocumentDialog(); return; }
    if (id == "header:save" || id == "dock:save") { saveDocument(false); return; }
    if (id == "header:export" || id == "dock:export") { exportPng(); return; }
    if (id == "header:fullscreen" || id == "dock:fullscreen") { toggleFullscreen(); return; }
    if (id == "header:help" || id == "dock:help") { overlay_ = 2; bgPanel_ = false; invalidate(); return; }
    if (id == "dock:settings" || id == "settings:open") {
        commitText();
        overlay_ = 3;
        bgPanel_ = false;
        invalidate();
        return;
    }""")

s = s.replace("""    if (id == "ov:pages:close" || id == "ov:help:close" || id == "ov:scrim") {
        overlay_ = 0;
        invalidate();
        return;
    }""",
"""    if (id == "ov:pages:close" || id == "ov:help:close" || id == "ov:scrim" || id == "settings:close") {
        commitAppName();
        overlay_ = 0;
        invalidate();
        return;
    }""")

insert = r'''
    if (id == "settings:version") {
        ULONGLONG now = GetTickCount64();
        if (now - versionClickLast_ > 2200) versionClicks_ = 0;
        versionClickLast_ = now;
        ++versionClicks_;
        if (versionClicks_ >= 8) {
            versionClicks_ = 0;
            devMode_ = true;
            saveConfig();
            showToast(L"开发者模式已启用", false, 1800);
            invalidate();
        } else if (versionClicks_ >= 5) {
            showToast(L"再点击 " + std::to_wstring(8 - versionClicks_) + L" 次启用开发者模式", false, 900);
        }
        return;
    }
    if (id == "settings:dock_reset") { resetDockPosition(); return; }
    if (id == "settings:apply_all") { applyAllPages_ = !applyAllPages_; invalidate(); return; }
    if (id == "settings:launcher_bg") { chooseLauncherBackground(); return; }
    if (id == "settings:launcher_bg_clear") { clearLauncherBackground(); return; }
    if (id == "settings:logo") { chooseCustomLogo(); return; }
    if (id == "settings:logo_clear") { clearCustomLogo(); return; }
    if (id == "settings:name_edit") { beginAppNameEdit(); return; }
    if (id == "settings:name_save") { commitAppName(); return; }
    if (id == "settings:dev_off") { devMode_ = false; saveConfig(); invalidate(); return; }
    if (id == "settings:reset_look") {
        appName_ = APP_NAME;
        clearLauncherBackground();
        clearCustomLogo();
        saveConfig();
        updateWindowTitle();
        invalidate();
        showToast(L"外观已恢复默认", false, 1600);
        return;
    }
    if (startsWith(id, "settings:canvas_color:")) {
        applyCanvasColor(hexColor(id.substr(21)), applyAllPages_);
        return;
    }
    if (id == "settings:canvas_custom" || id == "canvas:color_custom") {
        uint32_t c = chooseColorDialog(hwnd_, page().paper);
        applyCanvasColor(c, applyAllPages_ && id == "settings:canvas_custom");
        return;
    }
    if (startsWith(id, "canvas:color:")) {
        applyCanvasColor(hexColor(id.substr(13)), false);
        return;
    }
    if (startsWith(id, "canvas:pattern:") || startsWith(id, "bg:pattern:")) {
        size_t off = startsWith(id, "canvas:pattern:") ? 15 : 11;
        std::wstring pat = wide(id.substr(off));
        if (pat != page().pattern) {
            pushUndo();
            page().pattern = pat;
            page().touch();
            markDirty();
        }
        invalidate();
        return;
    }
'''

anchor = """    if (startsWith(id, "bg:paper:")) {
        uint32_t c = hexColor(id.substr(9));
        if (c != page().paper) {
            pushUndo();
            page().paper = c;
            page().touch();
            markDirty();
        }
        invalidate();
        return;
    }
}"""
repl = """    if (startsWith(id, "bg:paper:")) {
        uint32_t c = hexColor(id.substr(9));
        if (c != page().paper) {
            pushUndo();
            page().paper = c;
            page().touch();
            markDirty();
        }
        invalidate();
        return;
    }
""" + insert + "\n}"
if anchor not in s:
    raise SystemExit("handleRegion anchor missing")
s = s.replace(anchor, repl)
p.write_text(s, encoding="utf-8", newline="\n")
print("handleRegion patched")
