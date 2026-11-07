from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
old = """    for (const auto& d : toolDefs) {
        toolBtn(d.id, d.icon, tool_ == d.tool, true, d.tip);
        if (d.tool == Tool::Background) {
            toolBtn("dock:image", Icon::Image, false, true, L"插入图片  (I)");
        }
    }"""
new = old + """
    toolBtn("dock:function", Icon::Function, false, true, L"插入函数图像");"""
if old not in s:
    raise SystemExit("dock tool loop not found")
s = s.replace(old, new, 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("dock function button added")
