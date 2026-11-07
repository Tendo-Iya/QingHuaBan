from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("enum class Tool { Select, Pen, Highlighter, Eraser, Shape, Text, Laser, Hand };",
              "enum class Tool { Select, Pen, Highlighter, Eraser, Shape, Text, Laser, Background, Hand };")
s = s.replace("    RectF settingsRect() const;",
              "    RectF settingsRect() const;\n    RectF settingsNameRect() const;")
p.write_text(s, encoding="utf-8", newline="\n")
print("tool/settings patched")
