from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    item.color = 0x2457D6;
    item.width = 3.0f;
    item.strokeTool = L"pen";
    item.points.reserve(samples.size());""",
"""    item.color = 0x2457D6;
    item.width = 3.0f;
    item.strokeTool = L"pen";
    item.functionGraph = true;
    item.showAxes = fnShowAxes_;
    item.unitX = sx;
    item.unitY = sy;
    item.originX = center.x + (float)((0.0 - mx) * sx);
    item.originY = center.y - (float)((0.0 - my) * sy);
    item.gLeft = center.x - boxW * 0.5f;
    item.gRight = center.x + boxW * 0.5f;
    item.gTop = center.y - boxH * 0.5f;
    item.gBottom = center.y + boxH * 0.5f;
    if (fnShowAxes_) {
        item.gLeft = std::min(item.gLeft, item.originX - sx * 1.2f);
        item.gRight = std::max(item.gRight, item.originX + sx * 1.2f);
        item.gTop = std::min(item.gTop, item.originY - sy * 1.2f);
        item.gBottom = std::max(item.gBottom, item.originY + sy * 1.2f);
    }
    item.points.reserve(samples.size());""")
p.write_text(s, encoding="utf-8", newline="\n")
print("function item axes metadata on insert")
