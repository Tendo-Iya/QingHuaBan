from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("constexpr wchar_t APP_TAGLINE[] = L\"为灵感而生\";",
              "constexpr wchar_t APP_TAGLINE[] = L\"为灵感而生\";\nconstexpr wchar_t APP_VERSION[] = L\"1.2.0.1\";")
s = s.replace("""    bool splash_ = true;
    ULONGLONG splashStart_ = 0;""",
"""    bool splash_ = true;
    bool splashLeaving_ = false;
    ULONGLONG splashStart_ = 0;
    ULONGLONG splashLeaveStart_ = 0;
    ULONGLONG boardIntroStart_ = 0;
    ULONGLONG pageFadeStart_ = 0;
    float settingsAnim_ = 1.0f;
    bool settingsClosing_ = false;
    ULONGLONG settingsAnimStart_ = 0;
    ULONGLONG toastAnimStart_ = 0;""")
p.write_text(s, encoding="utf-8", newline="\n")
print("version and animation fields added")
