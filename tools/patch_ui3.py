from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""void App::onLButtonDblClk(int x, int y) {
    if (overlay_ || bgPanel_) return;
    if (tool_ != Tool::Select && tool_ != Tool::Text) return;""",
"""void App::onLButtonDblClk(int x, int y) {
    if (overlay_ == 3) {
        std::string rid = regionAt(x, y);
        if (rid == "settings:version") handleRegion(rid);
        return;
    }
    if (overlay_ || bgPanel_) return;
    if (tool_ != Tool::Select && tool_ != Tool::Text) return;""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    SolidBrush b(RGBA(color));
    g.FillEllipse(&b, cx - radius, cy - radius, radius * 2, radius * 2);
    if (color == COL_WHITE) {
        Pen edge(RGBA(0xDCE2EA), S(1));
        g.DrawEllipse(&edge, cx - radius, cy - radius, radius * 2, radius * 2);
    }""",
"""    SolidBrush b(RGBA(color));
    g.FillEllipse(&b, cx - radius, cy - radius, radius * 2, radius * 2);
    int lum = (int)(0.299 * ((color >> 16) & 255) + 0.587 * ((color >> 8) & 255) + 0.114 * (color & 255));
    if (lum > 232) {
        Pen edge(RGBA(0xC3D0C8), S(1));
        g.DrawEllipse(&edge, cx - radius, cy - radius, radius * 2, radius * 2);
    }""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""                    fTiny_, dark ? 0xE8F3EE : 0x84948B,
                    StringAlignmentCenter, StringAlignmentCenter, false);""",
"""                    fMini_, dark ? 0xE8F3EE : 0x75877D,
                    StringAlignmentCenter, StringAlignmentCenter, false);""")
s = s.replace("""                    fTiny_, dark ? 0xDCEBE4 : 0x95A39B,
                    StringAlignmentNear, StringAlignmentCenter, false);""",
"""                    fMini_, dark ? 0xDCEBE4 : 0x8A9B91,
                    StringAlignmentNear, StringAlignmentCenter, false);""")
p.write_text(s, encoding="utf-8", newline="\n")
print("dblclk, swatch, footer patched")
