from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    drawTextAligned(g, L"插入函数图像", RectF(px + S(28), py + S(18), S(360), S(30)), fBrand_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"输入 y = f(x)，例如 sin(x)、x^2、sqrt(x)、1/x",
                    RectF(px + S(28), py + S(47), S(540), S(22)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);""",
"""    bool pm = fnPointMode_;
    std::wstring title = pm ? L"描点" : (editingFunction_ >= 0 ? L"修改函数图像" : L"插入函数图像");
    std::wstring subtitle = pm ? L"输入点的坐标，例如 (2,3) 或 2,3；点会画在函数坐标系上"
                               : L"输入 y = f(x)，例如 sin(x)、x^2、sqrt(x)、1/x";
    drawTextAligned(g, title, RectF(px + S(28), py + S(18), S(420), S(30)), fBrand_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, subtitle, RectF(px + S(28), py + S(47), S(560), S(22)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);""")
s = s.replace("""    std::wstring shown = L"y = " + (fnExpr_.empty() ? L"" : wide(fnExpr_));
    uint32_t fieldColor = fnExpr_.empty() ? 0x9AA9A1 : COL_INK;
    drawTextAligned(g, fnExpr_.empty() ? L"y = 在这里输入解析式" : shown,
                    RectF(field.X + S(16), field.Y, field.Width - S(28), field.Height), fUI_,
                    fieldColor, StringAlignmentNear, StringAlignmentCenter, true);
    if (!fnExpr_.empty()) {
        RectF measured;
        g.MeasureString(shown.c_str(), -1, fUI_.get(), PointF(0, 0), &measured);""",
"""    std::wstring shown = pm ? wide(fnExpr_) : (L"y = " + (fnExpr_.empty() ? L"" : wide(fnExpr_)));
    std::wstring placeholder = pm ? L"输入坐标，如 (2,3)" : L"y = 在这里输入解析式";
    uint32_t fieldColor = fnExpr_.empty() ? 0x9AA9A1 : COL_INK;
    drawTextAligned(g, fnExpr_.empty() ? placeholder : shown,
                    RectF(field.X + S(16), field.Y, field.Width - S(28), field.Height), fUI_,
                    fieldColor, StringAlignmentNear, StringAlignmentCenter, true);
    if (!fnExpr_.empty()) {
        RectF measured;
        g.MeasureString(shown.c_str(), -1, fUI_.get(), PointF(0, 0), &measured);""")
s = s.replace("""        drawTextAligned(g, fnError_, RectF(px + S(28), py + S(258), pw - S(56), S(26)), fSmall_,
                        COL_DANGER, StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, L"支持 +  -  *  /  ^、平方 ²、括号和 sin/cos/tan/sqrt/abs/log/ln/exp 等函数",
                        RectF(px + S(28), py + S(258), pw - S(56), S(24)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }

    drawHeaderButton(g, "fn:insert", RectF(px + pw - S(150), py + ph - S(64), S(112), S(38)),
                     Icon::Check, L"插入", true, false, true, L"生成并插入函数图像");""",
"""        drawTextAligned(g, fnError_, RectF(px + S(28), py + S(258), pw - S(56), S(26)), fSmall_,
                        COL_DANGER, StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, pm ? L"点会按函数坐标 (x,y) 映射到图中；可 Ctrl+点击两个点后右键连线"
                               : L"支持 +  -  *  /  ^、平方 ²、括号和 sin/cos/tan/sqrt/abs/log/ln/exp 等函数",
                        RectF(px + S(28), py + S(258), pw - S(56), S(24)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }

    drawHeaderButton(g, "fn:insert", RectF(px + pw - S(150), py + ph - S(64), S(112), S(38)),
                     Icon::Check, pm ? L"描点" : L"插入", true, false, true,
                     pm ? L"把点画到函数图上" : L"生成并插入函数图像");""")
s = s.replace("""    drawTextAligned(g, L"Enter 插入   ·   Esc 取消   ·   插入后可直接拖动曲线",""",
"""    drawTextAligned(g, pm ? L"Enter 描点   ·   Esc 取消   ·   Ctrl+点击两个点后右键连线"
                          : L"Enter 插入   ·   Esc 取消   ·   插入后可直接拖动曲线",""")
p.write_text(s, encoding="utf-8", newline="\n")
print("point dialog mode labels")
