from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""            Pen pen(RGBA(it.color), w);
            pen.SetLineJoin(LineJoinRound);
            if (it.shape == ShapeKind::Ellipse) g.DrawEllipse(&pen, r);
            else g.DrawRectangle(&pen, r);
        }
    } else if (it.kind == ItemKind::Text) {""",
"""            Pen pen(RGBA(it.color), w);
            pen.SetLineJoin(LineJoinRound);
            if (it.shape == ShapeKind::Ellipse) g.DrawEllipse(&pen, r);
            else g.DrawRectangle(&pen, r);
            if (it.isPoint) {
                SolidBrush dot(RGBA(it.color));
                g.FillEllipse(&dot, r);
                if (!it.pointText.empty()) {
                    drawTextAligned(g, wide(it.pointText),
                                    RectF(r.GetRight() + S(6), r.Y - S(9), S(120), S(20)), fMini_,
                                    it.color, StringAlignmentNear, StringAlignmentCenter, false);
                }
            }
        }
    } else if (it.kind == ItemKind::Text) {""")
s = s.replace("""    for (auto& p : pts) {
        SolidBrush b(RGBA(COL_WHITE));
        g.FillRectangle(&b, p.X - hs / 2, p.Y - hs / 2, hs, hs);
        Pen bp(RGBA(COL_ACCENT), S(1.3f));
        g.DrawRectangle(&bp, p.X - hs / 2, p.Y - hs / 2, hs, hs);
    }
}""",
"""    for (auto& p : pts) {
        SolidBrush b(RGBA(COL_WHITE));
        g.FillRectangle(&b, p.X - hs / 2, p.Y - hs / 2, hs, hs);
        Pen bp(RGBA(COL_ACCENT), S(1.3f));
        g.DrawRectangle(&bp, p.X - hs / 2, p.Y - hs / 2, hs, hs);
    }
    Xf xf = pageXf();
    for (int idx : pointSel_) {
        if (idx < 0 || idx >= (int)page().items.size()) continue;
        RectF b = page().items[idx].bounds();
        RectF sr = xf.rect(b.X, b.Y, b.Width, b.Height);
        sr.Inflate(S(5), S(5));
        Pen ring(RGBA(COL_ACCENT), S(2.0f));
        g.DrawEllipse(&ring, sr);
    }
}""")
p.write_text(s, encoding="utf-8", newline="\n")
print("point rendering and selection rings")
