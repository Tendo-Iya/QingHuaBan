from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (startsWith(id, "fn:example:")) {
        fnExpr_ = id.substr(std::string("fn:example:").size());
        fnError_.clear();
        invalidate();
        return;
    }""",
"""    if (startsWith(id, "fn:example:")) {
        fnExpr_ = id.substr(std::string("fn:example:").size());
        fnError_.clear();
        invalidate();
        return;
    }
    if (id == "fn:square") {
        if (fnExpr_.empty()) fnExpr_ = "x^2";
        else fnExpr_ += "^2";
        fnError_.clear();
        invalidate();
        return;
    }
    if (id == "fn:axes") {
        fnShowAxes_ = !fnShowAxes_;
        invalidate();
        return;
    }""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    const wchar_t* examples[] = {L"sin(x)", L"x^2", L"1/x", L"e^(-x^2)", L"sqrt(x)"};
    float ex = px + S(28), ey = py + S(184);""",
"""    const wchar_t* examples[] = {L"x^2", L"1/x", L"sin(x)", L"cos(x)", L"tan(x)"};
    float ex = px + S(28), ey = py + S(184);""")
s = s.replace("""        ex += S(94);
    }

    if (!fnError_.empty()) {
        drawTextAligned(g, fnError_, RectF(px + S(28), py + S(226), pw - S(56), S(26)), fSmall_,
                        COL_DANGER, StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, L"支持 +  -  *  /  ^、括号和 sin/cos/tan/sqrt/abs/log/ln/exp 等函数",
                        RectF(px + S(28), py + S(226), pw - S(56), S(24)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }""",
"""        ex += S(94);
    }
    {
        RectF sq(ex, ey, S(74), S(30));
        if (hoverId_ == "fn:square") fillRoundRect(g, sq, S(9), 0xEAF6F1);
        else fillRoundRect(g, sq, S(9), 0xEAF2FF);
        drawTextAligned(g, L"x²", sq, fSmall_, 0x2457D6, StringAlignmentCenter,
                        StringAlignmentCenter, false);
        addRegion("fn:square", sq, L"插入平方 x²（等价 x^2）", CursorKind::Hand);
    }

    RectF cb(px + S(28), py + S(226), S(20), S(20));
    fillRoundRect(g, cb, S(6), fnShowAxes_ ? COL_ACCENT : 0xEDF2F7);
    if (fnShowAxes_) drawCheckmark(g, cb, COL_WHITE, S(1.7f));
    drawTextAligned(g, L"显示平面直角坐标系（含网格）",
                    RectF(px + S(56), py + S(222), S(360), S(28)), fSmall_, 0x4B5D53,
                    StringAlignmentNear, StringAlignmentCenter, false);
    addRegion("fn:axes", RectF(px + S(24), py + S(218), S(410), S(36)),
              L"切换坐标系显示", CursorKind::Hand);

    if (!fnError_.empty()) {
        drawTextAligned(g, fnError_, RectF(px + S(28), py + S(258), pw - S(56), S(26)), fSmall_,
                        COL_DANGER, StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, L"支持 +  -  *  /  ^、平方 ²、括号和 sin/cos/tan/sqrt/abs/log/ln/exp 等函数",
                        RectF(px + S(28), py + S(258), pw - S(56), S(24)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }""")
p.write_text(s, encoding="utf-8", newline="\n")
print("function examples, square button, axes checkbox")
