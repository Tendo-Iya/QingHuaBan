from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")

s = s.replace('    toolBtn("dock:new", Icon::Plus, false, true, L"新建白板  (Ctrl+N)");\n', '')

s = s.replace("""        if (selected) {
            fillRoundRect(g, br, S(10), COL_ACCENT_SOFT, 235);
            strokeRoundRect(g, br, S(10), 0xFFFFFF, S(1), 160);
        } else if (hov) {""",
"""        float te = easeOutCubic(toolAnim_);
        if (selected) {
            float inset = S(4) * (1.0f - te);
            RectF pill = br;
            pill.Inflate(-inset, -inset);
            fillRoundRect(g, pill, S(10), COL_ACCENT_SOFT, (BYTE)(235 * te));
            strokeRoundRect(g, pill, S(10), 0xFFFFFF, S(1), (BYTE)(160 * te));
        } else if (hov) {""")

s = s.replace("""        if (selected) {
            SolidBrush dot(RGBA(COL_ACCENT));
            g.FillEllipse(&dot, br.X + btn / 2 - S(2), br.GetBottom() - S(5), S(4), S(4));
        }""",
"""        if (selected) {
            SolidBrush dot(RGBA(COL_ACCENT, (BYTE)(255 * te)));
            g.FillEllipse(&dot, br.X + btn / 2 - S(2), br.GetBottom() - S(5), S(4), S(4));
        }""")

s = s.replace("""    fillRoundRect(g, cr, S(18), 0xFFFFFF, 122);
    strokeRoundRect(g, cr, S(18), 0xFFFFFF, S(1), 120);
    drawDockContext(g, cr, tool_);""",
"""    fillRoundRect(g, cr, S(18), 0xFFFFFF, 122);
    strokeRoundRect(g, cr, S(18), 0xFFFFFF, S(1), 120);
    GraphicsState ctxState = g.Save();
    g.TranslateTransform((1.0f - easeOutCubic(toolAnim_)) * S(10), 0.0f);
    drawDockContext(g, cr, tool_);
    g.Restore(ctxState);""")

p.write_text(s, encoding="utf-8", newline="\n")
print("ui: removed new whiteboard, added tool transition")
