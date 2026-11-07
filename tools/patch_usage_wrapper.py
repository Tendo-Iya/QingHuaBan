from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\HAVEGET-白板-发布\使用说明.txt")
s = p.read_text(encoding="utf-8")
s = s.replace("""1. 双击「HAVEGET白板安装器.exe」。""",
"""1. 推荐先双击「安装HAVEGET白板.cmd」。
   它会通过系统命令启动安装器，可减少 SmartScreen 拦截。
   也可以直接双击「HAVEGET白板安装器.exe」或「HAVEGET-Setup.exe」。""")
p.write_text(s, encoding="utf-8", newline="\n")
print("usage wrapper note added")
