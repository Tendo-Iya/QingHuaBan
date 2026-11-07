from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    if (kind == ItemKind::Stroke) {
        if (points.empty()) return RectF(0, 0, 0, 0);
        float pad = width / 2.0f + 2.0f;
        float x0 = points[0].x, y0 = points[0].y, x1 = x0, y1 = y0;
        for (auto& p : points) {
            x0 = std::min(x0, p.x); y0 = std::min(y0, p.y);
            x1 = std::max(x1, p.x); y1 = std::max(y1, p.y);
        }
        return RectF(x0 - pad, y0 - pad, x1 - x0 + pad * 2, y1 - y0 + pad * 2);
    }""",
"""    if (kind == ItemKind::Stroke) {
        if (points.empty()) return RectF(0, 0, 0, 0);
        float pad = width / 2.0f + 2.0f;
        float x0 = points[0].x, y0 = points[0].y, x1 = x0, y1 = y0;
        for (auto& p : points) {
            x0 = std::min(x0, p.x); y0 = std::min(y0, p.y);
            x1 = std::max(x1, p.x); y1 = std::max(y1, p.y);
        }
        if (functionGraph) {
            x0 = std::min(x0, gLeft); y0 = std::min(y0, gTop);
            x1 = std::max(x1, gRight); y1 = std::max(y1, gBottom);
        }
        return RectF(x0 - pad, y0 - pad, x1 - x0 + pad * 2, y1 - y0 + pad * 2);
    }""")
s = s.replace("""    if (kind == ItemKind::Stroke) {
        for (auto& p : c.points) { p.x += dx; p.y += dy; }
    } else if (kind == ItemKind::Shape) {""",
"""    if (kind == ItemKind::Stroke) {
        for (auto& p : c.points) { p.x += dx; p.y += dy; }
        if (c.functionGraph) {
            c.originX += dx; c.originY += dy;
            c.gLeft += dx; c.gRight += dx; c.gTop += dy; c.gBottom += dy;
        }
    } else if (kind == ItemKind::Shape) {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("function bounds/shift")
