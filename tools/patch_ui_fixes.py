from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace('"shape:" + strOf(i)', '"shape:" + std::to_string(i)')
s = s.replace("""void App::drawSettings(Graphics& g) {
    addRegion("ov:scrim", clientRect(), L"关闭设置", CursorKind::Arrow);""",
"""void App::drawSettings(Graphics& g) {
    const uint32_t paperColors[] = {0xFFFFFF, 0xFFF8EC, 0xEFFBF4, 0xF2F8FF, 0xFFF3F6, 0xF5F1FF};
    addRegion("ov:scrim", clientRect(), L"关闭设置", CursorKind::Arrow);""")
p.write_text(s, encoding="utf-8", newline="\n")

p2 = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s2 = p2.read_text(encoding="utf-8")
s2 = s2.replace("""    drawDock(g);
    drawTextEditorHint(g);
    drawToast(g);
    drawOverlay(g);
    drawTooltip(g);""",
"""    drawDock(g);
    drawTextEditorHint(g);
    drawOverlay(g);
    drawToast(g);
    drawTooltip(g);""")
p2.write_text(s2, encoding="utf-8", newline="\n")
print("ui fixes + toast order")
