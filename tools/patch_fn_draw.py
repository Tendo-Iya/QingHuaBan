from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
anchor = "void App::drawHelpOverlay(Graphics& g) {"
impl = r'''void App::drawFunctionDialog(Graphics& g) {
    fillRoundRect(g, RectF(0, 0, (float)W_, (float)H_), 0, 0x0B1220, 78);
    addRegion("ov:scrim", RectF(0, 0, (float)W_, (float)H_), L"", CursorKind::Arrow);
    float pw = std::min((float)W_ - S(40), S(640));
    float ph = S(380);
    float px = ((float)W_ - pw) / 2, py = ((float)H_ - ph) / 2;
    RectF panel(px, py, pw, ph);
    drawShadow(g, panel, S(20), S(18), S(8));
    fillRoundRect(g, panel, S(18), COL_WHITE);
    addRegion("ov:panel", panel, L"", CursorKind::Arrow);

    drawTextAligned(g, L"插入函数图像", RectF(px + S(28), py + S(18), S(360), S(30)), fBrand_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"输入 y = f(x)，例如 sin(x)、x^2、sqrt(x)、1/x",
                    RectF(px + S(28), py + S(47), S(540), S(22)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
    RectF closeR(px + pw - S(48), py + S(16), S(32), S(32));
    if (hoverId_ == "fn:close") fillRoundRect(g, closeR, S(8), 0xF2F5FA);
    drawIcon(g, Icon::Close, RectF(closeR.X + S(9), closeR.Y + S(9), S(14), S(14)), 0x6B7585, S(1.6f));
    addRegion("fn:close", closeR, L"关闭", CursorKind::Hand);

    RectF field(px + S(28), py + S(86), pw - S(56), S(52));
    fillRoundRect(g, field, S(12), 0xF6F9FD);
    strokeRoundRect(g, field, S(12), 0xBFD8CE, S(1.4f));
    addRegion("fn:field", field, L"函数解析式", CursorKind::IBeam);
    std::wstring shown = L"y = " + (fnExpr_.empty() ? L"" : wide(fnExpr_));
    uint32_t fieldColor = fnExpr_.empty() ? 0x9AA9A1 : COL_INK;
    drawTextAligned(g, fnExpr_.empty() ? L"y = 在这里输入解析式" : shown,
                    RectF(field.X + S(16), field.Y, field.Width - S(28), field.Height), fUI_,
                    fieldColor, StringAlignmentNear, StringAlignmentCenter, true);
    if (!fnExpr_.empty()) {
        RectF measured;
        g.MeasureString(shown.c_str(), -1, fUI_.get(), PointF(0, 0), &measured);
        float cx = field.X + S(16) + measured.Width + S(2);
        float cy0 = field.Y + S(12), cy1 = field.GetBottom() - S(12);
        if (cx < field.GetRight() - S(10)) {
            Pen caret(RGBA(COL_ACCENT), S(1.6f));
            g.DrawLine(&caret, cx, cy0, cx, cy1);
        }
    }

    drawTextAligned(g, L"常用示例", RectF(px + S(28), py + S(158), S(200), S(20)), fSmall_, 0x53665B,
                    StringAlignmentNear, StringAlignmentCenter, false);
    const wchar_t* examples[] = {L"sin(x)", L"x^2", L"1/x", L"e^(-x^2)", L"sqrt(x)"};
    float ex = px + S(28), ey = py + S(184);
    for (const wchar_t* example : examples) {
        std::wstring label = example;
        RectF r(ex, ey, S(86), S(30));
        if (hoverId_ == "fn:example:" + std::string(wide(label).begin(), wide(label).end())) {
            fillRoundRect(g, r, S(9), 0xEAF6F1);
        } else {
            fillRoundRect(g, r, S(9), 0xF3F7FA);
        }
        drawTextAligned(g, label, r, fMini_, 0x4B6055, StringAlignmentCenter,
                        StringAlignmentCenter, false);
        addRegion("fn:example:" + std::string(wide(label).begin(), wide(label).end()), r,
                  L"使用示例 " + label, CursorKind::Hand);
        ex += S(94);
    }

    if (!fnError_.empty()) {
        drawTextAligned(g, fnError_, RectF(px + S(28), py + S(226), pw - S(56), S(26)), fSmall_,
                        COL_DANGER, StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, L"支持 +  -  *  /  ^、括号和 sin/cos/tan/sqrt/abs/log/ln/exp 等函数",
                        RectF(px + S(28), py + S(226), pw - S(56), S(24)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }

    drawHeaderButton(g, "fn:insert", RectF(px + pw - S(150), py + ph - S(64), S(112), S(38)),
                     Icon::Check, L"插入", true, false, true, L"生成并插入函数图像");
    drawHeaderButton(g, "fn:close", RectF(px + pw - S(258), py + ph - S(64), S(94), S(38)),
                     Icon::Close, L"取消", false, true, true, L"关闭函数输入");
    drawTextAligned(g, L"Enter 插入   ·   Esc 取消   ·   插入后可直接拖动曲线",
                    RectF(px + S(28), py + ph - S(58), S(380), S(26)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
}

'''
s = s.replace(anchor, impl + anchor, 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("function dialog drawn")
