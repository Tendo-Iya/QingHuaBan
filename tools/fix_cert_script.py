from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\tools\make_release_cert.ps1")
s = p.read_text(encoding="utf-8")
s = s.replace("-TextExtension @('2.5.29.19={text}CA=true&pathlength=1')",
              '-TextExtension @("2.5.29.19={text}CA=true`&pathlength=1")')
p.write_text(s, encoding="utf-8-sig", newline="\n")
print("script fixed/BOM")
