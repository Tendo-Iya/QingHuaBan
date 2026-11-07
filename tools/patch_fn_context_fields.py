from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    std::wstring strokeTool = L\"pen\";",
"""    std::wstring strokeTool = L"pen";
    std::string funcExpr;
    bool isPoint = false;
    std::string pointText;""")
s = s.replace("    bool fnIgnoreNextChar_ = false;",
"""    bool fnIgnoreNextChar_ = false;
    int editingFunction_ = -1;
    bool fnPointMode_ = false;
    int fnPointGraph_ = -1;
    std::vector<int> pointSel_;
    double gestureLastZoom_ = 0;
    bool gestureUndoPushed_ = false;""")
s = s.replace("    void insertFunctionGraph();",
"""    void insertFunctionGraph();
    void submitFunctionDialog();
    void insertPointFromInput();
    void connectSelectedPoints(int mode, int direction);
    void fitFunctionToPage(Item& item);
    void scaleSelectedFunction(float factor, Pt screenAnchor);
    bool onGesture(LPARAM lp);""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\main.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""        case WM_MOUSEWHEEL: {""",
"""        case WM_GESTURE:
            if (g_app && g_app->onGesture(lp)) return 0;
            break;
        case WM_MOUSEWHEEL: {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("fields and gesture dispatch")
