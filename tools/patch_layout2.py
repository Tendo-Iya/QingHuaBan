from pathlib import Path

# dock position and page margins
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\liquid.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("float cy = dockDefault_ ? (float)H_ - S(18) - h / 2 : (float)H_ * dockNormY_;",
              "float cy = dockDefault_ ? (float)H_ - S(58) - h / 2 : (float)H_ * dockNormY_;")
s = s.replace("float maxY = (float)H_ - h / 2 - S(12);",
              "float maxY = (float)H_ - h / 2 - S(50);")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("float availH = (float)H_ - top - S(28) - S(178);",
              "float availH = (float)H_ - top - S(28) - S(212);")
s = s.replace("float mtop = S(28), mbottom = S(178);",
              "float mtop = S(28), mbottom = S(212);")
p.write_text(s, encoding="utf-8", newline="\n")

# splash footer + toast
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("RectF(card.X + S(24), card.GetBottom() - S(34), card.Width - S(48), S(20))",
              "RectF(card.X + S(24), card.GetBottom() - S(76), card.Width - S(48), S(22))")
s = s.replace("RectF(card.GetRight() - S(164), card.GetBottom() - S(34), S(140), S(20))",
              "RectF(card.GetRight() - S(164), card.GetBottom() - S(76), S(140), S(22))")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
old = """    RectF r(x, y, w, h);
    drawShadow(g, r, S(12), S(8), S(3));
    fillRoundRect(g, r, S(12), 0x202633, 246);
    float isz = S(18);
    RectF ir(x + S(16), y + (h - isz) / 2, isz, isz);
    if (toastError_) {
        SolidBrush b(RGBA(COL_DANGER));
        g.FillEllipse(&b, ir);
        Pen p(RGBA(COL_WHITE), S(1.5f));
        g.DrawLine(&p, ir.X + S(6), ir.Y + S(6), ir.GetRight() - S(6), ir.GetBottom() - S(6));
        g.DrawLine(&p, ir.GetRight() - S(6), ir.Y + S(6), ir.X + S(6), ir.GetBottom() - S(6));
    } else {
        SolidBrush b(RGBA(COL_SUCCESS));
        g.FillEllipse(&b, ir);
        drawCheckmark(g, ir, COL_WHITE, S(1.8f));
    }
    drawTextAligned(g, toast_, RectF(x + S(44), y, w - S(58), h), fToast_, COL_WHITE,
                    StringAlignmentNear, StringAlignmentCenter, true);"""
new = """    RectF r(x, y, w, h);
    drawGlass(g, r, S(14), 238);
    float isz = S(18);
    RectF ir(x + S(16), y + (h - isz) / 2, isz, isz);
    if (toastError_) {
        SolidBrush b(RGBA(COL_DANGER));
        g.FillEllipse(&b, ir);
        Pen p(RGBA(COL_WHITE), S(1.5f));
        g.DrawLine(&p, ir.X + S(6), ir.Y + S(6), ir.GetRight() - S(6), ir.GetBottom() - S(6));
        g.DrawLine(&p, ir.GetRight() - S(6), ir.Y + S(6), ir.X + S(6), ir.GetBottom() - S(6));
    } else {
        SolidBrush b(RGBA(COL_SUCCESS));
        g.FillEllipse(&b, ir);
        drawCheckmark(g, ir, COL_WHITE, S(1.8f));
    }
    drawTextAligned(g, toast_, RectF(x + S(44), y, w - S(58), h), fToast_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, true);"""
if old not in s:
    raise SystemExit("toast anchor missing")
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("positions/toast patched")
