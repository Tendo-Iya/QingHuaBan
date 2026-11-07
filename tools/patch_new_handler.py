from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
old = '    if (id == "header:undo" || id == "dock:undo") { undo(); return; }'
new = '    if (id == "dock:new") { newDocument(); return; }\n    if (id == "header:undo" || id == "dock:undo") { undo(); return; }'
if old not in s:
    raise SystemExit("anchor missing")
s = s.replace(old, new, 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("new handler added")
