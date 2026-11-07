from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# Paste implementation after insertImageFile
anchor = """void App::insertImageDialog() {
    commitText();
    std::wstring p = chooseOpenPath(
        L"图片文件 (*.png;*.jpg;*.jpeg;*.bmp;*.gif)\\0*.png;*.jpg;*.jpeg;*.bmp;*.gif\\0所有文件 (*.*)\\0*.*\\0\\0");
    if (p.empty()) return;
    insertImageFile(p);
}
"""
paste = anchor + r'''
void App::pasteImageFromClipboard() {
    if (!OpenClipboard(hwnd_)) return;
    std::wstring file;
    std::wstring tempFile;
    if (IsClipboardFormatAvailable(CF_HDROP)) {
        HDROP hd = (HDROP)GetClipboardData(CF_HDROP);
        if (hd) {
            wchar_t buf[2048] = {};
            if (DragQueryFileW(hd, 0, buf, 2048)) file = buf;
        }
    } else if (IsClipboardFormatAvailable(CF_BITMAP)) {
        HBITMAP hb = (HBITMAP)GetClipboardData(CF_BITMAP);
        if (hb) {
            Bitmap src(hb, nullptr);
            if (src.GetLastStatus() == Ok && src.GetWidth() > 0) {
                Bitmap* copy = src.Clone(0, 0, src.GetWidth(), src.GetHeight(), PixelFormat32bppARGB);
                if (copy) {
                    wchar_t tmp[MAX_PATH] = {};
                    GetTempPathW(MAX_PATH, tmp);
                    tempFile = std::wstring(tmp) + L"hav_clip_" + std::to_wstring(GetTickCount64()) + L".png";
                    if (!saveBitmapPng(copy, tempFile)) tempFile.clear();
                    delete copy;
                    if (!tempFile.empty()) file = tempFile;
                }
            }
        }
    }
    CloseClipboard();
    if (file.empty()) {
        showToast(L"剪贴板里没有可插入的图片", true);
        return;
    }
    insertImageFile(file);
    if (!tempFile.empty()) DeleteFileW(tempFile.c_str());
}
'''
if anchor not in s:
    raise SystemExit("insertImageDialog anchor missing")
s = s.replace(anchor, paste)

# Pointer methods after onMouseLeave
anchor2 = """void App::onMouseLeave() {
    trackingMouse_ = false;
    mouseInside_ = false;
    resetHover();
    invalidate();
}
"""
pointer = anchor2 + r'''
void App::onPointerDown(UINT id, int, int) {
    POINTER_INFO pi{};
    if (!GetPointerInfo(id, &pi)) return;
    if (pi.pointerType != PT_TOUCH && pi.pointerType != PT_PEN) return;
    if (!(pi.pointerFlags & POINTER_FLAG_PRIMARY)) return;
    if (pointerActive_) return;
    pointerActive_ = true;
    activePointerId_ = id;
    POINT p = pi.ptPixelLocation;
    ScreenToClient(hwnd_, &p);
    handlingPointer_ = true;
    onLButtonDown(p.x, p.y);
    handlingPointer_ = false;
}

void App::onPointerUpdate(UINT id, int, int) {
    if (!pointerActive_ || id != activePointerId_) return;
    UINT count = 0;
    GetPointerInfoHistory(id, &count, nullptr);
    if (count > 0) {
        std::vector<POINTER_INFO> history(count);
        if (GetPointerInfoHistory(id, &count, history.data())) {
            for (UINT i = 0; i < count; ++i) {
                if (!(history[i].pointerFlags & POINTER_FLAG_INCONTACT)) continue;
                POINT p = history[i].ptPixelLocation;
                ScreenToClient(hwnd_, &p);
                handlingPointer_ = true;
                onMouseMove(p.x, p.y, 0);
                handlingPointer_ = false;
            }
            return;
        }
    }
    POINTER_INFO pi{};
    if (!GetPointerInfo(id, &pi) || !(pi.pointerFlags & POINTER_FLAG_INCONTACT)) return;
    POINT p = pi.ptPixelLocation;
    ScreenToClient(hwnd_, &p);
    handlingPointer_ = true;
    onMouseMove(p.x, p.y, 0);
    handlingPointer_ = false;
}

void App::onPointerUp(UINT id, int, int) {
    if (!pointerActive_ || id != activePointerId_) return;
    POINTER_INFO pi{};
    POINT p = {mouse_.x, mouse_.y};
    if (GetPointerInfo(id, &pi)) {
        p = pi.ptPixelLocation;
        ScreenToClient(hwnd_, &p);
    }
    handlingPointer_ = true;
    onLButtonUp(p.x, p.y);
    handlingPointer_ = false;
    pointerActive_ = false;
    activePointerId_ = 0;
    suppressMouseUntil_ = GetTickCount64() + 300;
}

void App::onPointerCaptureLost(UINT id) {
    if (!pointerActive_ || id != activePointerId_) return;
    handlingPointer_ = true;
    onLButtonUp(mouse_.x, mouse_.y);
    handlingPointer_ = false;
    pointerActive_ = false;
    activePointerId_ = 0;
    suppressMouseUntil_ = GetTickCount64() + 300;
}
'''
if anchor2 not in s:
    raise SystemExit("onMouseLeave anchor missing")
s = s.replace(anchor2, pointer)

# Ctrl+V clipboard image
s = s.replace("""        if (vk == 'O') { openDocumentDialog(); return true; }
        if (vk == 'N') { newDocument(); return true; }""",
"""        if (vk == 'O') { openDocumentDialog(); return true; }
        if (vk == 'N') { newDocument(); return true; }
        if (vk == 'V') { pasteImageFromClipboard(); return true; }""")

p.write_text(s, encoding="utf-8", newline="\n")
print("pointer input and clipboard paste added")

# main.cpp pointer messages
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\main.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""        case WM_LBUTTONDBLCLK:
            if (g_app) g_app->onLButtonDblClk(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;""",
"""        case WM_LBUTTONDBLCLK:
            if (g_app) g_app->onLButtonDblClk(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERDOWN:
            if (g_app) g_app->onPointerDown((UINT)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERUPDATE:
            if (g_app) g_app->onPointerUpdate((UINT)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERUP:
            if (g_app) g_app->onPointerUp((UINT)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERCAPTURECHANGED:
            if (g_app) g_app->onPointerCaptureLost((UINT)wp);
            return 0;""")
p.write_text(s, encoding="utf-8", newline="\n")
print("main pointer messages added")
