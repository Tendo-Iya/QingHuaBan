from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("g.FillRectangle(&veil, 0, 0, (float)W_, (float)H_);",
              "g.FillRectangle(&veil, RectF(0, 0, (float)W_, (float)H_));")
s = s.replace("namespace qhb {\n\n// ---------------------------------------------------------------------------",
              "namespace qhb {\n\nstatic std::string idNum(const char* prefix, float v) {\n    return std::string(prefix) + utf8(fmtFloat(v, 0));\n}\n\n// ---------------------------------------------------------------------------", 1)
s = s.replace('"width:" + fmtFloat(w, 0)', 'idNum("width:", w)')
s = s.replace('"textsize:" + fmtFloat(s, 0)', 'idNum("textsize:", s)')
p.write_text(s, encoding="utf-8", newline="\n")
print("ui id/fill fixes")
