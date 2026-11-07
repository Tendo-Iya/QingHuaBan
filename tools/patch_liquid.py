from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\liquid.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("namespace qhb {\n", "namespace qhb {\n\nstatic float clampVal(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }\n")
s = s.replace("dockNormX_ = clampf(dockNormX_, 0.0f, 1.0f);\n    dockNormY_ = clampf(dockNormY_, 0.0f, 1.0f);",
              "dockNormX_ = clampVal(dockNormX_, 0.0f, 1.0f);\n    dockNormY_ = clampVal(dockNormY_, 0.0f, 1.0f);")
s = s.replace("cx = clampf(cx, minX, maxX);", "cx = clampVal(cx, minX, maxX);")
s = s.replace("cy = clampf(cy, minY, maxY);", "cy = clampVal(cy, minY, maxY);")
p.write_text(s, encoding="utf-8", newline="\n")
print("clamp fixed")
