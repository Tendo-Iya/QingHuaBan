from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""float App::baseScale() const {
    float top = headerH();
    float availW = (float)W_ - S(112) - S(50);
    float availH = (float)H_ - top - S(28) - S(212);
    if (availW < 100) availW = 100;
    if (availH < 100) availH = 100;
    return std::max(0.05f, std::min(availW / PAGE_W, availH / PAGE_H));
}

Pt App::viewCenter() const {
    float top = headerH();
    float left = S(112);
    float right = S(50);
    float mtop = S(28), mbottom = S(212);
    float x = left + ((float)W_ - left - right) / 2.0f;
    float y = top + mtop + ((float)H_ - top - mtop - mbottom) / 2.0f;
    return {x, y};
}""",
"""float App::baseScale() const {
    if (W_ <= 0 || H_ <= 0) return 0.5f;
    return std::max(0.05f, std::max((float)W_ / PAGE_W, (float)H_ / PAGE_H));
}

Pt App::viewCenter() const {
    return {W_ / 2.0f, H_ / 2.0f};
}""")
p.write_text(s, encoding="utf-8", newline="\n")
print("full-screen cover canvas")
