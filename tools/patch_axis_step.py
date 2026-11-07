from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("if (!sampleExpression(fnExpr_, -8.0, 8.0, 700, samples, error)) {",
              "if (!sampleExpression(fnExpr_, -5.0, 5.0, 700, samples, error)) {")
s = s.replace("float boxH = std::min(320.0f, PAGE_H * 0.36f);",
              "float boxH = std::min(420.0f, PAGE_H * 0.47f);")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("while (ux * step < S(25)) step *= 2;", "while (ux * step < S(14)) step *= 2;")
s = s.replace("while (uy * step < S(25)) step *= 2;", "while (uy * step < S(14)) step *= 2;")
p.write_text(s, encoding="utf-8", newline="\n")
print("unit labels every 1 when possible")
