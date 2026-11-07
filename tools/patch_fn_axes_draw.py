from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    void drawItem(Graphics& g, const Item& it, Xf xf, bool hideThis, bool exportMode);",
              "    void drawItem(Graphics& g, const Item& it, Xf xf, bool hideThis, bool exportMode);\n    void drawFunctionAxes(Graphics& g, const Item& it, Xf xf);")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
anchor = "void App::drawItem(Graphics& g, const Item& it, Xf xf, bool hideThis, bool exportMode) {"
impl = r'''void App::drawFunctionAxes(Graphics& g, const Item& it, Xf xf) {
    if (!it.functionGraph || !it.showAxes) return;
    float left = xf.tx(it.gLeft), right = xf.tx(it.gRight);
    float top = xf.ty(it.gTop), bottom = xf.ty(it.gBottom);
    if (right - left < S(20) || bottom - top < S(20)) return;
    RectF box(left, top, right - left, bottom - top);
    GraphicsState st = g.Save();
    g.SetClip(box, CombineModeIntersect);

    float ux = it.unitX * xf.s;
    float uy = it.unitY * xf.s;
    float ox = xf.tx(it.originX), oy = xf.ty(it.originY);
    if (ux > S(7)) {
        Pen grid(RGBA(0x405A74, 26), std::max(0.6f, S(0.8f)));
        int first = (int)std::ceil((left - ox) / ux);
        int last = (int)std::floor((right - ox) / ux);
        if (last - first <= 80) {
            for (int k = first; k <= last; ++k) {
                if (k == 0) continue;
                float x = ox + k * ux;
                g.DrawLine(&grid, x, top, x, bottom);
            }
        }
    }
    if (uy > S(7)) {
        Pen grid(RGBA(0x405A74, 26), std::max(0.6f, S(0.8f)));
        int first = (int)std::ceil((top - oy) / uy);
        int last = (int)std::floor((bottom - oy) / uy);
        if (last - first <= 80) {
            for (int k = first; k <= last; ++k) {
                if (k == 0) continue;
                float y = oy + k * uy;
                g.DrawLine(&grid, left, y, right, y);
            }
        }
    }

    Pen axis(RGBA(0x8A9BAC, 225), std::max(1.0f, S(1.2f)));
    axis.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    bool hasX = oy >= top - 2 && oy <= bottom + 2;
    bool hasY = ox >= left - 2 && ox <= right + 2;
    if (hasX) g.DrawLine(&axis, left, oy, right, oy);
    if (hasY) g.DrawLine(&axis, ox, top, ox, bottom);

    Pen arrow(RGBA(0x8A9BAC, 225), std::max(1.0f, S(1.2f)));
    if (hasX) {
        g.DrawLine(&arrow, right - S(9), oy - S(4), right, oy);
        g.DrawLine(&arrow, right - S(9), oy + S(4), right, oy);
    }
    if (hasY) {
        g.DrawLine(&arrow, ox - S(4), top + S(9), ox, top);
        g.DrawLine(&arrow, ox + S(4), top + S(9), ox, top);
    }
    if (hasX && hasY) {
        drawTextAligned(g, L"O", RectF(ox + S(4), oy + S(2), S(20), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }
    if (hasX) {
        drawTextAligned(g, L"x", RectF(right - S(18), oy - S(20), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    if (hasY) {
        drawTextAligned(g, L"y", RectF(ox + S(4), top + S(2), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    g.Restore(st);
}

'''
s = s.replace(anchor, impl + anchor, 1)
s = s.replace("""        float w = std::max(0.7f, it.width * s);
        if (it.points.size() == 1) {""",
"""        if (it.functionGraph && it.showAxes) drawFunctionAxes(g, it, xf);
        float w = std::max(0.7f, it.width * s);
        if (it.points.size() == 1) {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("function axes drawing")
