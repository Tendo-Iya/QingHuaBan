from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")

s = s.replace("""std::wstring toolName(Tool t) {
    switch (t) {
        case Tool::Select: return L"选择";
        case Tool::Pen: return L"画笔";
        case Tool::Highlighter: return L"荧光笔";
        case Tool::Eraser: return L"橡皮擦";
        case Tool::Shape: return L"形状";
        case Tool::Text: return L"文字";
        case Tool::Laser: return L"激光笔";
        case Tool::Hand: return L"移动画布";
    }
    return L"画笔";
}""",
"""std::wstring toolName(Tool t) {
    switch (t) {
        case Tool::Select: return L"选择";
        case Tool::Pen: return L"画笔";
        case Tool::Highlighter: return L"荧光笔";
        case Tool::Eraser: return L"橡皮擦";
        case Tool::Shape: return L"形状";
        case Tool::Text: return L"文字";
        case Tool::Laser: return L"激光笔";
        case Tool::Background: return L"画布";
        case Tool::Hand: return L"移动画布";
    }
    return L"画笔";
}""")

s = s.replace("""        case Tool::Laser: return L"按住拖动，松手渐隐";
        case Tool::Hand: return L"拖动画布";
    }
    return L"";
}""",
"""        case Tool::Laser: return L"按住拖动，松手渐隐";
        case Tool::Background: return L"选择画布颜色和底纹";
        case Tool::Hand: return L"拖动画布";
    }
    return L"";
}""")

old_render = """void App::render(Graphics& g) {
    regions_.clear();
    g.Clear(RGBA(COL_DESK));
    drawPage(g);
    drawHeader(g);
    drawSelection(g);
    drawLive(g);
    drawRail(g);
    drawOptionBar(g);
    drawZoomBar(g);
    drawStatus(g);
    drawBgPanel(g);
    drawTextEditorHint(g);
    drawToast(g);
    drawOverlay(g);
    drawTooltip(g);
}"""
new_render = """void App::render(Graphics& g) {
    regions_.clear();
    drawBackdrop(g);
    if (splash_) {
        drawSplash(g);
        return;
    }
    drawPage(g);
    drawHeader(g);
    drawSelection(g);
    drawLive(g);
    drawDock(g);
    drawTextEditorHint(g);
    drawToast(g);
    drawOverlay(g);
    drawTooltip(g);
}"""
if old_render not in s: raise SystemExit("render anchor missing")
s = s.replace(old_render, new_render)

start = s.index("void App::drawHeader(Graphics& g) {")
end = s.index("void App::drawPageNav(Graphics& g) {", start)
s = s[:start] + "void App::drawHeader(Graphics& g) {\n    drawBrand(g, false);\n}\n\n" + s[end:]

s = s.replace("""    if (page().items.empty() && !hEdit_ && undo_.empty()) {
        uint32_t hintColor = 0xB9C2D0;
    drawTextAligned(g, L"选择画笔，直接在页面上书写", RectF(pr.X, pr.GetBottom() - S(102), pr.Width, S(26)),
                        fUI_, hintColor, StringAlignmentCenter, StringAlignmentCenter, false);
        drawTextAligned(g, L"空格 + 拖动移动画布   ·   Ctrl + 滚轮缩放   ·   0 适应页面",
                        RectF(pr.X, pr.GetBottom() - S(76), pr.Width, S(22)), fMini_, 0xC4CBD6,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }""",
"""    if (page().items.empty() && !hEdit_ && undo_.empty()) {
        uint32_t hintColor = 0xAAB8B1;
        drawTextAligned(g, L"在下方工具栏选择画笔，开始创作", RectF(pr.X, pr.GetBottom() - S(102), pr.Width, S(26)),
                        fUI_, hintColor, StringAlignmentCenter, StringAlignmentCenter, false);
        drawTextAligned(g, L"拖动下方工具栏可移动   ·   Ctrl + 滚轮缩放   ·   0 适应页面",
                        RectF(pr.X, pr.GetBottom() - S(76), pr.Width, S(22)), fMini_, 0xB8C5BE,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }""")

old_overlay = """void App::drawOverlay(Graphics& g) {
    if (overlay_ == 1) drawPagesOverlay(g);
    else if (overlay_ == 2) drawHelpOverlay(g);
}"""
new_overlay = """void App::drawOverlay(Graphics& g) {
    if (overlay_ == 1) drawPagesOverlay(g);
    else if (overlay_ == 2) drawHelpOverlay(g);
    else if (overlay_ == 3) drawSettings(g);
}"""
if old_overlay not in s: raise SystemExit("overlay anchor missing")
s = s.replace(old_overlay, new_overlay)

p.write_text(s, encoding="utf-8", newline="\n")
print("render basics patched")
