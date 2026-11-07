from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (overlay_ == 1) drawPagesOverlay(g);
    else if (overlay_ == 2) drawHelpOverlay(g);
    else if (overlay_ == 3) drawSettings(g);""",
"""    if (overlay_ == 1) drawPagesOverlay(g);
    else if (overlay_ == 2) drawHelpOverlay(g);
    else if (overlay_ == 3) drawSettings(g);
    else if (overlay_ == 4) drawFunctionDialog(g);""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# overlay4 click branch before overlay3
s = s.replace("""    if (overlay_ == 3) {
        if (settingsClosing_ || settingsAnim_ < 0.96f) return;""",
"""    if (overlay_ == 4) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "fn:") || rid == "ov:scrim") handleRegion(rid);
        return;
    }
    if (overlay_ == 3) {
        if (settingsClosing_ || settingsAnim_ < 0.96f) return;""")

# dock open + fn handlers near page label
s = s.replace("""    if (id == "page:label") { overlay_ = 1; bgPanel_ = false; pageScrollToCurrent_ = true; invalidate(); return; }""",
"""    if (id == "page:label") { overlay_ = 1; bgPanel_ = false; pageScrollToCurrent_ = true; invalidate(); return; }
    if (id == "dock:function") {
        fnExpr_.clear();
        fnError_.clear();
        bgPanel_ = false;
        overlay_ = 4;
        invalidate();
        return;
    }
    if (id == "fn:insert") { insertFunctionGraph(); return; }
    if (id == "fn:close") { overlay_ = 0; invalidate(); return; }
    if (startsWith(id, "fn:example:")) {
        fnExpr_ = id.substr(std::string("fn:example:").size());
        fnError_.clear();
        invalidate();
        return;
    }""")

# keyboard overlay4 at top after splash/hEdit? add before hEdit
s = s.replace("""bool App::onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt) {
    if (splash_) { enterBoard(); return true; }
    if (hEdit_) return false;""",
"""bool App::onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt) {
    if (splash_) { enterBoard(); return true; }
    if (overlay_ == 4) {
        if (vk == VK_ESCAPE) { overlay_ = 0; invalidate(); return true; }
        if (vk == VK_RETURN) { insertFunctionGraph(); return true; }
        if (vk == VK_BACK) {
            if (!fnExpr_.empty()) {
                fnExpr_.pop_back();
                fnError_.clear();
                invalidate();
            }
            return true;
        }
        return true;
    }
    if (hEdit_) return false;""")

# onChar after onKeyUp
s = s.replace("""void App::onKeyUp(WPARAM vk) {
    if (vk == VK_SPACE) {
        spaceDown_ = false;
        updateCursor();
    }
}""",
"""void App::onKeyUp(WPARAM vk) {
    if (vk == VK_SPACE) {
        spaceDown_ = false;
        updateCursor();
    }
}

bool App::onChar(wchar_t ch) {
    if (overlay_ != 4) return false;
    if (ch >= 32 && ch < 127 && fnExpr_.size() < 120) {
        fnExpr_.push_back((char)ch);
        fnError_.clear();
        invalidate();
        return true;
    }
    return false;
}""")

# insert function graph implementation before onKeyDown or near page functions
anchor = "void App::onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt) {"
impl = r'''void App::insertFunctionGraph() {
    std::vector<std::pair<double, double>> samples;
    std::string error;
    if (!sampleExpression(fnExpr_, -8.0, 8.0, 700, samples, error)) {
        fnError_ = wide(error);
        invalidate();
        return;
    }
    double xmin = samples.front().first, xmax = samples.back().first;
    double ymin = samples.front().second, ymax = samples.front().second;
    for (auto& p : samples) {
        ymin = std::min(ymin, p.second);
        ymax = std::max(ymax, p.second);
    }
    double xspan = std::max(1.0e-6, xmax - xmin);
    double yspan = std::max(1.0e-6, ymax - ymin);
    float boxW = std::min(1050.0f, PAGE_W * 0.78f);
    float boxH = std::min(560.0f, PAGE_H * 0.62f);
    float sx = boxW / (float)xspan;
    float sy = boxH / (float)yspan;
    if (yspan < 1.0e-5) sy = sx;
    sy = clampf(sy, sx * 0.10f, sx * 3.0f);
    Pt center = screenToWorld(viewCenter());
    double mx = (xmin + xmax) * 0.5, my = (ymin + ymax) * 0.5;

    Item item;
    item.kind = ItemKind::Stroke;
    item.id = nextId_++;
    item.color = 0x2457D6;
    item.width = 3.0f;
    item.strokeTool = L"pen";
    item.points.reserve(samples.size());
    for (auto& p : samples) {
        float x = center.x + (float)((p.first - mx) * sx);
        float y = center.y - (float)((p.second - my) * sy);
        item.points.push_back({x, y});
    }
    if (item.points.size() < 2) {
        fnError_ = L"函数点太少，无法绘制";
        invalidate();
        return;
    }
    pushUndo();
    page().items.push_back(std::move(item));
    int index = (int)page().items.size() - 1;
    setTool(Tool::Select);
    sel_ = index;
    overlay_ = 0;
    page().touch();
    markDirty();
    showToast(L"已插入函数 y = " + wide(fnExpr_), false, 1800);
    invalidate();
}

'''
s = s.replace(anchor, impl + anchor, 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("function dialog behavior added")
