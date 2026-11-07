from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("inline Color RGBA(uint32_t rgb, BYTE a = 255) {",
"""inline float easeOutCubic(float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

inline Color RGBA(uint32_t rgb, BYTE a = 255) {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("ease helper added")
