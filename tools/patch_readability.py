from pathlib import Path

# qhb.h palette
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("constexpr uint32_t COL_MUTED = 0x7B8495;", "constexpr uint32_t COL_MUTED = 0x5F7167;")
s = s.replace("constexpr uint32_t COL_FAINT = 0xA9B2C0;", "constexpr uint32_t COL_FAINT = 0x84948B;")
s = s.replace("constexpr uint32_t COL_LINE = 0xE6EAF0;", "constexpr uint32_t COL_LINE = 0xDCE5DF;")
p.write_text(s, encoding="utf-8", newline="\n")

# app fonts
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    fBrand_ = mk(20, true);
    fUI_ = mk(12.5f);
    fSmall_ = mk(11.2f);
    fMini_ = mk(9.8f);
    fNav_ = mk(12.5f, true);
    fTitle_ = mk(13.2f, true);
    fBig_ = mk(30, true);
    fToast_ = mk(11.5f);
    fTextHint_ = mk(10.2f);
    fHero_ = mk(58, true);
    fSplashSub_ = mk(13.5f);
    fSection_ = mk(15.5f, true);
    fTiny_ = mk(9.2f);
    fCapsule_ = mk(11.0f);""",
"""    fBrand_ = mk(23, true);
    fUI_ = mk(14.8f);
    fSmall_ = mk(12.8f);
    fMini_ = mk(11.4f);
    fNav_ = mk(14.2f, true);
    fTitle_ = mk(15.0f, true);
    fBig_ = mk(32, true);
    fToast_ = mk(13.2f);
    fTextHint_ = mk(11.8f);
    fHero_ = mk(58, true);
    fSplashSub_ = mk(16.0f);
    fSection_ = mk(18.0f, true);
    fTiny_ = mk(11.0f);
    fCapsule_ = mk(13.0f);""")
p.write_text(s, encoding="utf-8", newline="\n")

# settings panel dimensions
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\liquid.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""RectF App::settingsRect() const {
    float w = S(560);
    float h = S(660);""",
"""RectF App::settingsRect() const {
    float w = S(620);
    float h = S(680);""")
p.write_text(s, encoding="utf-8", newline="\n")

# brand chip larger
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("float chipW = std::min(S(380), std::max(S(196), S(58) + contentW + S(22)));\n    RectF chip(S(16), S(12), chipW, S(46));",
              "float chipW = std::min(S(410), std::max(S(210), S(62) + contentW + S(24)));\n    RectF chip(S(16), S(12), chipW, S(52));")
s = s.replace("RectF lr(chip.X + S(8), chip.Y + S(7), S(32), S(32));",
              "RectF lr(chip.X + S(9), chip.Y + S(8), S(36), S(36));")
s = s.replace("""        drawTextAligned(g, name, RectF(tx, chip.Y, tw, chip.Height), fBrand_, COL_INK,
                        StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, name, RectF(tx, chip.Y + S(3), tw, S(24)), fBrand_, COL_INK,
                        StringAlignmentNear, StringAlignmentCenter, true);
        drawTextAligned(g, title, RectF(tx, chip.Y + S(25), tw - S(10), S(18)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, true);""",
"""        drawTextAligned(g, name, RectF(tx, chip.Y, tw, chip.Height), fBrand_, COL_INK,
                        StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, name, RectF(tx, chip.Y + S(3), tw, S(27)), fBrand_, COL_INK,
                        StringAlignmentNear, StringAlignmentCenter, true);
        drawTextAligned(g, title, RectF(tx, chip.Y + S(29), tw - S(10), S(19)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, true);""")
# settings muted/gray colors and splash footer contrast
s = s.replace("0x778A80", "0x5C6F65")
s = s.replace("0x8A9B92", "0x6B7E74")
s = s.replace("0x94A39B", "0x77887F")
s = s.replace("0x98A79F", "0x788A80")
s = s.replace("dark ? 0xE8F3EE : 0x75877D", "dark ? 0xE8F3EE : 0x5F7167")
s = s.replace("dark ? 0xDCEBE4 : 0x8A9B91", "dark ? 0xDCEBE4 : 0x6E8076")
s = s.replace("0x74877D", "0x566A60")
p.write_text(s, encoding="utf-8", newline="\n")

# installer fonts/contrast
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\installer\installer.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    fTitle = mk(29, true);
    fHero = mk(22, true);
    fUI = mk(12.2f);
    fSmall = mk(10.6f);
    fTiny = mk(9.4f);""",
"""    fTitle = mk(31, true);
    fHero = mk(24, true);
    fUI = mk(14.0f);
    fSmall = mk(12.2f);
    fTiny = mk(11.0f);""")
for a, b in [("0x7E9187", "0x5C6F65"), ("0x8A9B91", "0x6B7E74"),
             ("0x98A79F", "0x788A80"), ("0x60746A", "0x45594F"),
             ("0x778A80", "0x5C6F65")]:
    s = s.replace(a, b)
p.write_text(s, encoding="utf-8", newline="\n")
print("readability typography/contrast patched")
