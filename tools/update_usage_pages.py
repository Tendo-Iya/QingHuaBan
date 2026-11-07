from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\HAVEGET-白板-发布\使用说明.txt")
s = p.read_text(encoding="utf-8")
s = s.replace("""- 工具：选择、画笔、荧光笔、橡皮、形状、文字、激光笔、画布、图片、移动。
- 操作：新建、撤销/重做、页面切换/管理、打开、保存、导出、
  设置、帮助、全屏、缩放。
- 左上角显示自定义 Logo、应用名称和当前文件名。""",
"""- 工具：选择、画笔、荧光笔、橡皮、形状、文字、激光笔、画布、图片、移动。
- 操作：撤销/重做、页面切换/管理、打开、保存、导出、
  设置、帮助、全屏、缩放。
- 工具栏已移除「新建白板」；新建页面统一在「页面管理」里完成。
- 页面管理：拖动页面缩略图可调整顺序；滚轮或拖动右侧滚动条可翻页；
  新建页面会自动滚动到当前页。
- 左上角显示自定义 Logo、应用名称和当前文件名。""")
p.write_text(s, encoding="utf-8", newline="\n")
print("usage updated")
