from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("""    // shape
    ShapeKind shape = ShapeKind::Rect;""",
"""    // plotted function metadata (stored on Stroke items)
    bool functionGraph = false;
    bool showAxes = true;
    float originX = 0, originY = 0;
    float gLeft = 0, gTop = 0, gRight = 0, gBottom = 0;
    float unitX = 0, unitY = 0;

    // shape
    ShapeKind shape = ShapeKind::Rect;""")
s = s.replace("    std::wstring fnError_;", "    std::wstring fnError_;\n    bool fnShowAxes_ = true;")
p.write_text(s, encoding="utf-8", newline="\n")
print("function graph metadata fields")
