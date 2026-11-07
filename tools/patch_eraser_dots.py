from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (t == Tool::Eraser) {
        label(L"大小");
        float widths[] = {16, 30, 48, 72};
        for (float w : widths) {
            drawWidthDot(g, idNum("width:", w), x + S(13), cy, w, std::fabs(eraserSize_ - w) < 0.01f, L"橡皮大小");
            x += S(32);
        }""",
"""    if (t == Tool::Eraser) {
        label(L"大小");
        float widths[] = {16, 30, 48, 72};
        for (float w : widths) {
            float rr = S(4.0f) + (w / 72.0f) * S(9.0f);
            float cx = x + S(14);
            bool sel = std::fabs(eraserSize_ - w) < 0.01f;
            std::string id = idNum("width:", w);
            if (sel) {
                Pen ring(RGBA(COL_ACCENT), S(1.8f));
                g.DrawEllipse(&ring, cx - rr - S(3), cy - rr - S(3), (rr + S(3)) * 2, (rr + S(3)) * 2);
            } else if (hoverId_ == id) {
                Pen ring(RGBA(0xC9D2E2), S(1.2f));
                g.DrawEllipse(&ring, cx - rr - S(3), cy - rr - S(3), (rr + S(3)) * 2, (rr + S(3)) * 2);
            }
            SolidBrush brush(RGBA(0x3B4555, sel ? 235 : 190));
            g.FillEllipse(&brush, cx - rr, cy - rr, rr * 2, rr * 2);
            addRegion(id, RectF(cx - S(17), cy - S(20), S(34), S(40)),
                      L"橡皮大小 " + std::to_wstring((int)w), CursorKind::Hand);
            x += S(36);
        }""")
p.write_text(s, encoding="utf-8", newline="\n")
print("eraser size dots now proportional")
