from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""        if (hoverId_ == "fn:example:" + std::string(wide(label).begin(), wide(label).end())) {
            fillRoundRect(g, r, S(9), 0xEAF6F1);
        } else {
            fillRoundRect(g, r, S(9), 0xF3F7FA);
        }
        drawTextAligned(g, label, r, fMini_, 0x4B6055, StringAlignmentCenter,
                        StringAlignmentCenter, false);
        addRegion("fn:example:" + std::string(wide(label).begin(), wide(label).end()), r,
                  L"使用示例 " + label, CursorKind::Hand);""",
"""        std::string exampleId = "fn:example:" + utf8(label);
        if (hoverId_ == exampleId) {
            fillRoundRect(g, r, S(9), 0xEAF6F1);
        } else {
            fillRoundRect(g, r, S(9), 0xF3F7FA);
        }
        drawTextAligned(g, label, r, fMini_, 0x4B6055, StringAlignmentCenter,
                        StringAlignmentCenter, false);
        addRegion(exampleId, r, L"使用示例 " + label, CursorKind::Hand);""")
p.write_text(s, encoding="utf-8", newline="\n")
print("example ids fixed")
