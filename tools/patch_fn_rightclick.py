from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# function expression and smaller fit
s = s.replace("""    float boxW = std::min(1050.0f, PAGE_W * 0.78f);
    float boxH = std::min(560.0f, PAGE_H * 0.62f);""",
"""    float boxW = std::min(780.0f, PAGE_W * 0.52f);
    float boxH = std::min(380.0f, PAGE_H * 0.42f);""")
s = s.replace("""    item.functionGraph = true;
    item.showAxes = fnShowAxes_;""",
"""    item.functionGraph = true;
    item.showAxes = fnShowAxes_;
    item.funcExpr = fnExpr_;""")
s = s.replace("""    pushUndo();
    page().items.push_back(std::move(item));
    int index = (int)page().items.size() - 1;
    setTool(Tool::Select);
    sel_ = index;
    overlay_ = 0;
    page().touch();
    markDirty();
    showToast(L"已插入函数 y = " + wide(fnExpr_), false, 1800);
    invalidate();
}""",
"""    fitFunctionToPage(item);
    int index = -1;
    if (editingFunction_ >= 0 && editingFunction_ < (int)page().items.size()) {
        pushUndo();
        index = editingFunction_;
        page().items[index] = std::move(item);
        editingFunction_ = -1;
    } else {
        pushUndo();
        page().items.push_back(std::move(item));
        index = (int)page().items.size() - 1;
    }
    setTool(Tool::Select);
    sel_ = index;
    pointSel_.clear();
    overlay_ = 0;
    page().touch();
    markDirty();
    showToast(L"已插入函数 y = " + wide(fnExpr_), false, 1800);
    invalidate();
}""")

# Add helpers before insertFunctionGraph
anchor = "void App::insertFunctionGraph() {"
helpers = r'''void App::fitFunctionToPage(Item& item) {
    if (!item.functionGraph) return;
    const float margin = 24.0f;
    for (int pass = 0; pass < 3; ++pass) {
        RectF b = item.bounds();
        if (b.Width <= 0 || b.Height <= 0) return;
        float sx = (PAGE_W - margin * 2) / b.Width;
        float sy = (PAGE_H - margin * 2) / b.Height;
        float sc = std::min(1.0f, std::min(sx, sy));
        if (sc < 0.999f) {
            Pt c{b.X + b.Width / 2, b.Y + b.Height / 2};
            for (auto& p : item.points) {
                p.x = c.x + (p.x - c.x) * sc;
                p.y = c.y + (p.y - c.y) * sc;
            }
            item.originX = c.x + (item.originX - c.x) * sc;
            item.originY = c.y + (item.originY - c.y) * sc;
            item.gLeft = c.x + (item.gLeft - c.x) * sc;
            item.gRight = c.x + (item.gRight - c.x) * sc;
            item.gTop = c.y + (item.gTop - c.y) * sc;
            item.gBottom = c.y + (item.gBottom - c.y) * sc;
            item.unitX *= sc;
            item.unitY *= sc;
            continue;
        }
        b = item.bounds();
        float dx = 0, dy = 0;
        if (b.X < margin) dx = margin - b.X;
        else if (b.GetRight() > PAGE_W - margin) dx = PAGE_W - margin - b.GetRight();
        if (b.Y < margin) dy = margin - b.Y;
        else if (b.GetBottom() > PAGE_H - margin) dy = PAGE_H - margin - b.GetBottom();
        if (std::fabs(dx) > 0.01f || std::fabs(dy) > 0.01f) {
            for (auto& p : item.points) { p.x += dx; p.y += dy; }
            item.originX += dx; item.originY += dy;
            item.gLeft += dx; item.gRight += dx; item.gTop += dy; item.gBottom += dy;
        }
        break;
    }
}

void App::scaleSelectedFunction(float factor, Pt screenAnchor) {
    if (sel_ < 0 || sel_ >= (int)page().items.size()) return;
    Item& it = page().items[sel_];
    if (!it.functionGraph) return;
    if (!gestureUndoPushed_) {
        pushUndo();
        gestureUndoPushed_ = true;
    }
    Pt w = screenToWorld(screenAnchor);
    factor = clampf(factor, 0.6f, 1.7f);
    for (auto& p : it.points) {
        p.x = w.x + (p.x - w.x) * factor;
        p.y = w.y + (p.y - w.y) * factor;
    }
    it.originX = w.x + (it.originX - w.x) * factor;
    it.originY = w.y + (it.originY - w.y) * factor;
    it.gLeft = w.x + (it.gLeft - w.x) * factor;
    it.gRight = w.x + (it.gRight - w.x) * factor;
    it.gTop = w.y + (it.gTop - w.y) * factor;
    it.gBottom = w.y + (it.gBottom - w.y) * factor;
    it.unitX *= factor;
    it.unitY *= factor;
    fitFunctionToPage(it);
    page().touch();
    markDirty();
    invalidate();
}

bool App::onGesture(LPARAM lp) {
    GESTUREINFO gi{};
    gi.cbSize = sizeof(gi);
    if (!GetGestureInfo((HGESTUREINFO)lp, &gi)) return false;
    bool handled = false;
    if (gi.dwID == GID_ZOOM) {
        handled = true;
        if (gi.dwFlags & GF_BEGIN) {
            gestureLastZoom_ = (double)gi.ullArguments;
            gestureUndoPushed_ = false;
        } else if (gestureLastZoom_ > 0 && gi.ullArguments > 0) {
            double factor = (double)gi.ullArguments / gestureLastZoom_;
            gestureLastZoom_ = (double)gi.ullArguments;
            POINT pt = gi.ptsLocation;
            ScreenToClient(hwnd_, &pt);
            scaleSelectedFunction((float)factor, {(float)pt.x, (float)pt.y});
        }
    }
    CloseGestureInfoHandle((HGESTUREINFO)lp);
    return handled;
}

void App::submitFunctionDialog() {
    if (fnPointMode_) insertPointFromInput();
    else insertFunctionGraph();
}

void App::insertPointFromInput() {
    std::string t = fnExpr_;
    for (char& c : t) {
        if (c == '(' || c == ')' || c == '[' || c == ']' || c == ' ') c = ' ';
    }
    size_t comma = t.find(',');
    if (comma == std::string::npos) comma = t.find(';');
    if (comma == std::string::npos) {
        fnError_ = L"请输入两个坐标，例如 (2,3)";
        invalidate();
        return;
    }
    try {
        double gx = std::stod(t.substr(0, comma));
        double gy = std::stod(t.substr(comma + 1));
        Pt world;
        if (fnPointGraph_ >= 0 && fnPointGraph_ < (int)page().items.size() &&
            page().items[fnPointGraph_].functionGraph) {
            Item& f = page().items[fnPointGraph_];
            world = {f.originX + (float)(gx * f.unitX), f.originY - (float)(gy * f.unitY)};
        } else {
            Pt c = screenToWorld(viewCenter());
            world = {c.x + (float)(gx * 40), c.y - (float)(gy * 40)};
        }
        Item p;
        p.kind = ItemKind::Shape;
        p.shape = ShapeKind::Ellipse;
        p.filled = true;
        p.width = 2.0f;
        p.color = 0xC0392B;
        p.x1 = world.x - 5.5f; p.y1 = world.y - 5.5f;
        p.x2 = world.x + 5.5f; p.y2 = world.y + 5.5f;
        p.isPoint = true;
        char buf[80];
        snprintf(buf, sizeof(buf), "(%.4g, %.4g)", gx, gy);
        p.pointText = buf;
        p.id = nextId_++;
        pushUndo();
        page().items.push_back(std::move(p));
        int idx = (int)page().items.size() - 1;
        setTool(Tool::Select);
        sel_ = idx;
        pointSel_.clear();
        pointSel_.push_back(idx);
        overlay_ = 0;
        page().touch();
        markDirty();
        invalidate();
    } catch (...) {
        fnError_ = L"坐标格式不正确，例如 (2,3)";
        invalidate();
    }
}

void App::connectSelectedPoints(int mode, int direction) {
    if (pointSel_.size() != 2) return;
    auto center = [&](int idx) {
        RectF b = page().items[idx].bounds();
        return Pt{b.X + b.Width / 2, b.Y + b.Height / 2};
    };
    int i1 = pointSel_[0], i2 = pointSel_[1];
    if (i1 < 0 || i2 < 0 || i1 >= (int)page().items.size() || i2 >= (int)page().items.size()) return;
    Pt a = center(i1), b = center(i2);
    Pt p1 = a, p2 = b;
    if (mode != 2) {
        float dx = b.x - a.x, dy = b.y - a.y;
        if (std::fabs(dx) + std::fabs(dy) < 1e-4f) return;
        float tmin = -1e9f, tmax = 1e9f;
        auto clipAxis = [&](float p0, float d, float lo, float hi) {
            if (std::fabs(d) < 1e-6f) return p0 >= lo && p0 <= hi;
            float t1 = (lo - p0) / d, t2 = (hi - p0) / d;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            return tmin <= tmax;
        };
        if (!clipAxis(a.x, dx, 0, PAGE_W) || !clipAxis(a.y, dy, 0, PAGE_H)) return;
        if (mode == 0) {
            p1 = {a.x + dx * tmin, a.y + dy * tmin};
            p2 = {a.x + dx * tmax, a.y + dy * tmax};
        } else {
            Pt start = direction == 0 ? a : b;
            Pt dir = direction == 0 ? Pt{dx, dy} : Pt{-dx, -dy};
            float tEnd = 0;
            if (std::fabs(dir.x) > 1e-6f) {
                float tx = dir.x > 0 ? (PAGE_W - start.x) / dir.x : (0 - start.x) / dir.x;
                tEnd = std::max(tEnd, tx);
            }
            if (std::fabs(dir.y) > 1e-6f) {
                float ty = dir.y > 0 ? (PAGE_H - start.y) / dir.y : (0 - start.y) / dir.y;
                tEnd = std::max(tEnd, ty);
            }
            p1 = start;
            p2 = {start.x + dir.x * tEnd, start.y + dir.y * tEnd};
        }
    }
    Item line;
    line.kind = ItemKind::Shape;
    line.shape = ShapeKind::Line;
    line.color = 0x3B4555;
    line.width = 2.0f;
    line.x1 = p1.x; line.y1 = p1.y; line.x2 = p2.x; line.y2 = p2.y;
    line.id = nextId_++;
    pushUndo();
    page().items.push_back(std::move(line));
    int idx = (int)page().items.size() - 1;
    setTool(Tool::Select);
    sel_ = idx;
    pointSel_.clear();
    page().touch();
    markDirty();
    invalidate();
}

'''
s = s.replace(anchor, helpers + anchor, 1)

# Enter calls submit
s = s.replace("if (vk == VK_RETURN) { insertFunctionGraph(); return true; }",
              "if (vk == VK_RETURN) { submitFunctionDialog(); return true; }")
# handle fn insert submit
s = s.replace('if (id == "fn:insert") { insertFunctionGraph(); return; }',
              'if (id == "fn:insert") { submitFunctionDialog(); return; }')
# reset modes on open
s = s.replace("""void App::openFunctionDialog() {
    fnExpr_.clear();
    fnError_.clear();
    fnIgnoreNextChar_ = false;
    bgPanel_ = false;
    overlay_ = 4;
    invalidate();
}""",
"""void App::openFunctionDialog() {
    fnExpr_.clear();
    fnError_.clear();
    fnIgnoreNextChar_ = false;
    fnPointMode_ = false;
    editingFunction_ = -1;
    bgPanel_ = false;
    overlay_ = 4;
    invalidate();
}""")
p.write_text(s, encoding="utf-8", newline="\n")
print("function edit, plot point, connect, pinch")
