from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\build.ps1")
s = p.read_text(encoding="utf-8-sig")
s = s.replace("$out = Join-Path $root 'HAVEGET白板.exe'", "$out = Join-Path $root 'HAVEGET-Whiteboard.exe'")
p.write_text(s, encoding="utf-8-sig", newline="\n")
print("build output ascii")
