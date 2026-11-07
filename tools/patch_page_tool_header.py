from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    ULONGLONG lastLivePaint_ = 0;",
"""    ULONGLONG lastLivePaint_ = 0;
    int pageDragFrom_ = -1;
    int pageDragTo_ = -1;
    bool pageDragActive_ = false;
    Pt pageDragStart_{};
    bool pageScrollDrag_ = false;
    float pageScrollDragStartY_ = 0;
    float pageScrollDragStart_ = 0;
    float pageScrollMax_ = 0;
    float pageScrollTrackH_ = 0;
    float pageScrollThumbH_ = 0;
    float toolAnim_ = 1.0f;
    ULONGLONG toolAnimStart_ = 0;""")
s = s.replace("    void addPage();\n    void duplicatePage();",
              "    void addPage();\n    void movePage(int from, int to);\n    void duplicatePage();")
p.write_text(s, encoding="utf-8", newline="\n")
print("page/tool state added")
