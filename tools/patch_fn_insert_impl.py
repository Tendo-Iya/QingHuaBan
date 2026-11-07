from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
anchor = "bool App::onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt) {"
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
if "void App::insertFunctionGraph()" in s:
    raise SystemExit("already present")
s = s.replace(anchor, impl + anchor, 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("insertFunctionGraph added", "void App::insertFunctionGraph()" in s)
