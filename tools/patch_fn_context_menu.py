from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
old_start = s.index("void App::onRButtonUp(int x, int y) {")
old_end = s.index("void App::onMouseWheel", old_start)
new = r'''void App::onRButtonUp(int x, int y) {
    commitText();
    if (overlay_) { overlay_ = 0; invalidate(); return; }
    if (bgPanel_) { bgPanel_ = false; invalidate(); return; }
    Pt wp = screenToWorld({(float)x, (float)y});
    int idx = inPage({(float)x, (float)y}) ? hitItemAt(wp, 7) : -1;
    if (idx >= 0) {
        sel_ = idx;
        if (page().items[idx].isPoint) {
            bool already = std::find(pointSel_.begin(), pointSel_.end(), idx) != pointSel_.end();
            if (!already) {
                pointSel_.clear();
                pointSel_.push_back(idx);
            }
        } else {
            pointSel_.clear();
        }
    } else {
        pointSel_.clear();
    }
    HMENU menu = CreatePopupMenu();
    if (idx >= 0) {
        const Item& it = page().items[idx];
        if (it.functionGraph) {
            AppendMenuW(menu, MF_STRING, 10, L"修改函数…");
            AppendMenuW(menu, MF_STRING, 11, L"描点…");
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        }
        AppendMenuW(menu, MF_STRING, 1, L"复制        Ctrl+D");
        AppendMenuW(menu, MF_STRING, 2, L"删除        Delete");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING, 3, L"移到最上层");
        AppendMenuW(menu, MF_STRING, 4, L"移到最下层");
    } else {
        AppendMenuW(menu, MF_STRING, 5, L"适应页面        0");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING, 6, L"清空本页");
    }
    if (pointSel_.size() == 2) {
        HMENU connect = CreatePopupMenu();
        AppendMenuW(connect, MF_STRING, 20, L"直线");
        HMENU ray = CreatePopupMenu();
        AppendMenuW(ray, MF_STRING, 31, L"向第二点方向延伸");
        AppendMenuW(ray, MF_STRING, 32, L"向第一点方向延伸");
        AppendMenuW(connect, MF_POPUP, (UINT_PTR)ray, L"射线");
        AppendMenuW(connect, MF_STRING, 40, L"线段");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_POPUP, (UINT_PTR)connect, L"连线（两个点）");
    }
    POINT pt{x, y};
    ClientToScreen(hwnd_, &pt);
    int cmd = (int)TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd_, nullptr);
    DestroyMenu(menu);
    switch (cmd) {
        case 1: duplicateSelection(); break;
        case 2: deleteSelection(); break;
        case 3: bringSelection(true); break;
        case 4: bringSelection(false); break;
        case 5: fitView(); break;
        case 6: clearPage(); break;
        case 10:
            if (idx >= 0 && idx < (int)page().items.size()) {
                editingFunction_ = idx;
                fnPointMode_ = false;
                fnExpr_ = page().items[idx].funcExpr;
                fnError_.clear();
                fnIgnoreNextChar_ = false;
                overlay_ = 4;
                invalidate();
            }
            break;
        case 11:
            if (idx >= 0 && idx < (int)page().items.size()) {
                fnPointMode_ = true;
                fnPointGraph_ = idx;
                fnExpr_ = "(0,0)";
                fnError_.clear();
                fnIgnoreNextChar_ = false;
                overlay_ = 4;
                invalidate();
            }
            break;
        case 20: connectSelectedPoints(0, 0); break;
        case 31: connectSelectedPoints(1, 0); break;
        case 32: connectSelectedPoints(1, 1); break;
        case 40: connectSelectedPoints(2, 0); break;
        default: break;
    }
}

'''
s = s[:old_start] + new + s[old_end:]
p.write_text(s, encoding="utf-8", newline="\n")
print("right-click function/point/connect menu")
