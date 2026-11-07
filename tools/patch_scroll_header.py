from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    float pageScrollDragStart_ = 0;",
"""    float pageScrollDragStart_ = 0;
    float pageScrollFrom_ = 0;
    float pageScrollTo_ = 0;
    ULONGLONG pageScrollAnimStart_ = 0;
    bool pageScrollAnimating_ = false;
    Pt camScrollFrom_{};
    Pt camScrollTo_{};
    ULONGLONG camScrollStart_ = 0;
    bool camScrollAnimating_ = false;
    std::unique_ptr<Bitmap> pageContentBmp_;""")
p.write_text(s, encoding="utf-8", newline="\n")
print("scroll animation state added")
