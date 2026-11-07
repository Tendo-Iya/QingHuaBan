from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
old = """    if (hasX && hasY) {
        drawTextAligned(g, L"O", RectF(ox + S(4), oy + S(2), S(20), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }
    if (hasX) {
        drawTextAligned(g, L"x", RectF(right - S(18), oy - S(20), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    if (hasY) {
        drawTextAligned(g, L"y", RectF(ox + S(4), top + S(2), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }"""
new = """    if (hasX && hasY) {
        drawTextAligned(g, L"0", RectF(ox - S(18), oy + S(3), S(14), S(15)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    if (hasX) {
        drawTextAligned(g, L"x", RectF(right - S(18), oy - S(20), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    if (hasY) {
        drawTextAligned(g, L"y", RectF(ox + S(4), top + S(2), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }

    if (hasX && ux > S(2)) {
        int step = 1;
        while (ux * step < S(25)) step *= 2;
        int first = (int)std::ceil((left - ox) / ux);
        int last = (int)std::floor((right - ox) / ux);
        for (int k = first; k <= last; ++k) {
            if (k == 0 || k % step != 0) continue;
            float x = ox + k * ux;
            drawTextAligned(g, std::to_wstring(k), RectF(x - S(18), oy + S(4), S(36), S(16)), fTiny_,
                            0x7A8D9E, StringAlignmentCenter, StringAlignmentCenter, false);
        }
    }
    if (hasY && uy > S(2)) {
        int step = 1;
        while (uy * step < S(25)) step *= 2;
        int first = (int)std::ceil((oy - bottom) / uy);
        int last = (int)std::floor((oy - top) / uy);
        for (int k = first; k <= last; ++k) {
            if (k == 0 || k % step != 0) continue;
            float y = oy - k * uy;
            drawTextAligned(g, std::to_wstring(k), RectF(ox - S(34), y - S(8), S(28), S(16)), fTiny_,
                            0x7A8D9E, StringAlignmentFar, StringAlignmentCenter, true);
        }
    }"""
if old not in s: raise SystemExit("axes labels block not found")
s = s.replace(old, new, 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("axes numeric labels")
