from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    void refreshCursor() { updateCursor(); }",
              "    void refreshCursor() { updateCursor(); }\n    void finishAppNameEdit() { commitAppName(); }\n    void cancelAppNameEdit();")
p.write_text(s, encoding="utf-8", newline="\n")
print("public name edit methods added")
