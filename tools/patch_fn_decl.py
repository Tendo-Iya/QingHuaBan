from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    float toolAnim_ = 1.0f;", "    std::string fnExpr_;\n    std::wstring fnError_;\n    float toolAnim_ = 1.0f;")
s = s.replace("    void onKeyUp(WPARAM vk);", "    void onKeyUp(WPARAM vk);\n    bool onChar(wchar_t ch);")
s = s.replace("    void drawHelpOverlay(Graphics& g);", "    void drawHelpOverlay(Graphics& g);\n    void drawFunctionDialog(Graphics& g);\n    void insertFunctionGraph();")
p.write_text(s, encoding="utf-8", newline="\n")
print("function dialog declarations")
