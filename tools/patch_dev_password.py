from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    bool devMode_ = false;",
"""    bool devMode_ = false;
    bool devUnlocked_ = false;
    std::string devPass_;
    std::wstring devPassError_;""")
s = s.replace("    bool onGesture(LPARAM lp);",
"    bool onGesture(LPARAM lp);\n    void verifyDevPassword();")
s = s.replace("    void drawFunctionDialog(Graphics& g);",
"    void drawFunctionDialog(Graphics& g);\n    void drawDevPasswordDialog(Graphics& g);")
s = s.replace("    int overlay_ = 0;  // 0 none, 1 pages, 2 help, 3 settings",
              "    int overlay_ = 0;  // 0 none, 1 pages, 2 help, 3 settings, 4 function, 6 dev password")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# 8th click opens password prompt
s = s.replace("""        if (versionClicks_ >= 8) {
            versionClicks_ = 0;
            devMode_ = true;
            saveConfig();
            showToast(L"开发者模式已启用", false, 1800);
            invalidate();
        } else if (versionClicks_ >= 5) {""",
"""        if (versionClicks_ >= 8) {
            versionClicks_ = 0;
            devPass_.clear();
            devPassError_.clear();
            overlay_ = 6;
            invalidate();
        } else if (versionClicks_ >= 5) {""")

# dev off locks
s = s.replace('if (id == "settings:dev_off") { devMode_ = false; saveConfig(); invalidate(); return; }',
              'if (id == "settings:dev_off") { devMode_ = false; devUnlocked_ = false; saveConfig(); invalidate(); return; }')

# overlay6 mouse branch before overlay4
s = s.replace("""    if (overlay_ == 4) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "fn:") || rid == "ov:scrim") handleRegion(rid);
        return;
    }""",
"""    if (overlay_ == 6) {
        std::string rid = regionAt(x, y);
        if (rid == "devpass:ok" || rid == "devpass:cancel" || rid == "ov:scrim") handleRegion(rid);
        return;
    }
    if (overlay_ == 4) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "fn:") || rid == "ov:scrim") handleRegion(rid);
        return;
    }""")

# overlay6 keyboard before overlay4
s = s.replace("""    if (overlay_ == 4) {
        if (vk == VK_ESCAPE) { overlay_ = 0; invalidate(); return true; }""",
"""    if (overlay_ == 6) {
        if (vk == VK_ESCAPE) { overlay_ = 3; devPass_.clear(); devPassError_.clear(); invalidate(); return true; }
        if (vk == VK_RETURN) { verifyDevPassword(); return true; }
        if (vk == VK_BACK) {
            if (!devPass_.empty()) { devPass_.pop_back(); devPassError_.clear(); invalidate(); }
            return true;
        }
        return true;
    }
    if (overlay_ == 4) {
        if (vk == VK_ESCAPE) { overlay_ = 0; invalidate(); return true; }""")

# onChar overlay6
s = s.replace("""bool App::onChar(wchar_t ch) {
    if (overlay_ != 4) return false;""",
"""bool App::onChar(wchar_t ch) {
    if (overlay_ == 6) {
        if (ch >= 32 && ch < 127 && devPass_.size() < 64) {
            devPass_.push_back((char)ch);
            devPassError_.clear();
            invalidate();
            return true;
        }
        return false;
    }
    if (overlay_ != 4) return false;""")

# handleRegion devpass before fn insert
s = s.replace("""    if (id == "fn:insert") { submitFunctionDialog(); return; }""",
"""    if (id == "devpass:ok") { verifyDevPassword(); return; }
    if (id == "devpass:cancel") {
        overlay_ = 3;
        devPass_.clear();
        devPassError_.clear();
        invalidate();
        return;
    }
    if (id == "fn:insert") { submitFunctionDialog(); return; }""")

# verify function before openFunctionDialog
anchor = "void App::openFunctionDialog() {"
impl = r'''void App::verifyDevPassword() {
    if (devPass_ == "HAVEGETNEVERDIE") {
        devMode_ = true;
        devUnlocked_ = true;
        saveConfig();
        overlay_ = 3;
        devPass_.clear();
        devPassError_.clear();
        showToast(L"开发者选项已解锁", false, 1600);
        invalidate();
    } else {
        devPassError_ = L"密码错误，请重试";
        devPass_.clear();
        invalidate();
    }
}

'''
s = s.replace(anchor, impl + anchor, 1)
p.write_text(s, encoding="utf-8", newline="\n")

# settings show dev only unlocked
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("    if (devMode_) {", "    if (devMode_ && devUnlocked_) {")
p.write_text(s, encoding="utf-8", newline="\n")

# draw overlay6
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    else if (overlay_ == 4) drawFunctionDialog(g);""",
"""    else if (overlay_ == 4) drawFunctionDialog(g);
    else if (overlay_ == 6) drawDevPasswordDialog(g);""")
p.write_text(s, encoding="utf-8", newline="\n")
print("developer password gate")
