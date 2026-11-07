from pathlib import Path
import base64

base = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\output")
out = base / "轻画板演示.qhb"

def b64(s: str) -> str:
    return base64.b64encode(s.encode("utf-8")).decode("ascii")

def text(x, y, s, size, color, width=900):
    return f"TEXT #{color} {size} {width} {x} {y} {b64(s)}"

def shape(kind, color, width, filled, x1, y1, x2, y2):
    return f"SHAPE {kind} #{color} {width} {1 if filled else 0} {x1} {y1} {x2} {y2}"

def stroke(color, width, pts):
    return "STROKE pen #%s %s %d %s" % (color, width, len(pts), " ".join(f"{p[0]},{p[1]}" for p in pts))

def hilite(color, width, pts):
    return "STROKE highlighter #%s %s %d %s" % (color, width, len(pts), " ".join(f"{p[0]},{p[1]}" for p in pts))

items = []
# soft highlight behind the title
items.append(hilite("FFD54A", 42, [(112, 118), (300, 112), (520, 120), (720, 112), (840, 120)]))

items.append(text(102, 68, "课堂灵感，随手记录", 58, "202633", 1050))
items.append(text(106, 154, "轻画板 · 即点即写", 28, "2F6BFF", 700))
# accent underline
items.append(stroke("F59E0B", 6, [(112, 216), (240, 211), (390, 218), (540, 209), (690, 216), (770, 211)]))

# central idea box
items.append(shape("rect", "2F6BFF", 4, 0, 620, 390, 980, 530))
items.append(text(680, 428, "课堂灵感", 42, "202633", 280))

# arrows and labels
items.append(shape("arrow", "18A56F", 4, 0, 620, 460, 450, 460))
items.append(text(292, 424, "随手写", 34, "202633", 180))
items.append(shape("arrow", "F59E0B", 4, 0, 980, 438, 1160, 340))
items.append(text(1190, 306, "随时讲", 34, "202633", 180))
items.append(shape("arrow", "8B5CF6", 4, 0, 800, 530, 800, 650))
items.append(text(670, 672, "多页板书", 34, "202633", 240))

# green check + note
items.append(stroke("18A56F", 9, [(252, 680), (292, 722), (385, 625)]))
items.append(text(420, 664, "内容已保存", 27, "18A56F", 260))

# small red note
items.append(text(1180, 582, "一键导出 PNG", 30, "E5484D", 300))
items.append(stroke("E5484D", 4, [(1174, 630), (1260, 664), (1364, 622)]))

# footer
items.append(text(102, 800, "空格拖动画布  ·  Ctrl+滚轮缩放  ·  Ctrl+Z 撤销", 23, "7B8495", 900))

lines = []
lines.append("QHB2")
lines.append("TITLE " + b64("轻画板演示"))
lines.append("CURRENT 0")
lines.append("PAGES 2")
lines.append(f"PAGE blank #FFFFFF {len(items)}")
lines.extend(items)
lines.append("PAGE grid #FFFFFF 0")
out.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(out, out.stat().st_size)
