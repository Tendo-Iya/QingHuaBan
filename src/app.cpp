#include "qhb.h"

#include <cstdio>
#include <cstring>

namespace qhb {

// ---------------------------------------------------------------------------
// geometry / hit testing
// ---------------------------------------------------------------------------

static float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

static float distSeg2(float px, float py, float ax, float ay, float bx, float by) {
    float dx = bx - ax, dy = by - ay;
    if (dx == 0 && dy == 0) return (px - ax) * (px - ax) + (py - ay) * (py - ay);
    float t = ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy);
    t = clampf(t, 0, 1);
    float qx = ax + t * dx, qy = ay + t * dy;
    return (px - qx) * (px - qx) + (py - qy) * (py - qy);
}

static RectF approxTextBounds(const std::wstring& text, float font, float maxWidth, float x, float y) {
    float maxW = 0, curW = 0;
    int lines = 1;
    for (size_t i = 0; i < text.size(); ++i) {
        wchar_t c = text[i];
        if (c == L'\n') {
            maxW = std::max(maxW, curW);
            curW = 0;
            ++lines;
            continue;
        }
        float cw = (c < 0x2E80) ? font * 0.60f : font * 1.0f;
        if (curW + cw > maxWidth && curW > 0) {
            maxW = std::max(maxW, curW);
            curW = cw;
            ++lines;
        } else {
            curW += cw;
        }
    }
    maxW = std::max(maxW, curW);
    float width = std::min(std::max(maxW, font * 0.8f), std::max(maxWidth, font));
    float height = lines * font * 1.42f;
    return RectF(x - 2, y - 2, width + 4, height + 4);
}

RectF Item::bounds() const {
    if (kind == ItemKind::Stroke) {
        if (points.empty()) return RectF(0, 0, 0, 0);
        float pad = width / 2.0f + 2.0f;
        float x0 = points[0].x, y0 = points[0].y, x1 = x0, y1 = y0;
        for (auto& p : points) {
            x0 = std::min(x0, p.x); y0 = std::min(y0, p.y);
            x1 = std::max(x1, p.x); y1 = std::max(y1, p.y);
        }
        if (functionGraph) {
            x0 = std::min(x0, gLeft); y0 = std::min(y0, gTop);
            x1 = std::max(x1, gRight); y1 = std::max(y1, gBottom);
        }
        return RectF(x0 - pad, y0 - pad, x1 - x0 + pad * 2, y1 - y0 + pad * 2);
    }
    if (kind == ItemKind::Shape) {
        float pad = width / 2.0f + 3;
        float x0 = std::min(x1, x2), y0 = std::min(y1, y2);
        float x3 = std::max(x1, x2), y3 = std::max(y1, y2);
        return RectF(x0 - pad, y0 - pad, x3 - x0 + pad * 2, y3 - y0 + pad * 2);
    }
    if (kind == ItemKind::Text) {
        return approxTextBounds(text, fontSize, textWidth, x, y);
    }
    return RectF(x - 2, y - 2, w + 4, h + 4);
}

bool Item::hit(float px, float py, float tolerance) const {
    float tol = std::max(1.0f, tolerance);
    if (kind == ItemKind::Stroke) {
        float half = width / 2.0f + tol;
        float half2 = half * half;
        if (points.size() == 1) {
            float dx = px - points[0].x, dy = py - points[0].y;
            return dx * dx + dy * dy <= half2;
        }
        for (size_t i = 0; i + 1 < points.size(); ++i) {
            if (distSeg2(px, py, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y) <= half2) return true;
        }
        return false;
    }
    if (kind == ItemKind::Shape) {
        if (shape == ShapeKind::Line || shape == ShapeKind::Arrow) {
            float d = std::sqrt(distSeg2(px, py, x1, y1, x2, y2));
            return d <= width / 2 + tol + 3;
        }
        float x0 = std::min(x1, x2), y0 = std::min(y1, y2);
        float x3 = std::max(x1, x2), y3 = std::max(y1, y2);
        if (px < x0 - tol || px > x3 + tol || py < y0 - tol || py > y3 + tol) return false;
        if (filled && shape == ShapeKind::Rect) return true;
        if (shape == ShapeKind::Ellipse) {
            float cx = (x0 + x3) / 2, cy = (y0 + y3) / 2;
            float rx = std::max(0.1f, (x3 - x0) / 2), ry = std::max(0.1f, (y3 - y0) / 2);
            float v = ((px - cx) / rx) * ((px - cx) / rx) + ((py - cy) / ry) * ((py - cy) / ry);
            float band = std::max(0.16f, (width + tol * 2) / std::max(std::min(rx, ry), 1.0f));
            return std::fabs(std::sqrt(v) - 1.0f) <= band;
        }
        float dx = std::max(std::max(x0 - px, 0.0f), px - x3);
        float dy = std::max(std::max(y0 - py, 0.0f), py - y3);
        bool inside = px >= x0 && px <= x3 && py >= y0 && py <= y3;
        return inside && (std::fabs(px - x0) <= tol || std::fabs(px - x3) <= tol ||
                          std::fabs(py - y0) <= tol || std::fabs(py - y3) <= tol);
    }
    RectF b = bounds();
    return px >= b.X - tol && px <= b.GetRight() + tol && py >= b.Y - tol && py <= b.GetBottom() + tol;
}

Item Item::shifted(float dx, float dy) const {
    Item c = *this;
    if (kind == ItemKind::Stroke) {
        for (auto& p : c.points) { p.x += dx; p.y += dy; }
        if (c.functionGraph) {
            c.originX += dx; c.originY += dy;
            c.gLeft += dx; c.gRight += dx; c.gTop += dy; c.gBottom += dy;
        }
    } else if (kind == ItemKind::Shape) {
        c.x1 += dx; c.x2 += dx; c.y1 += dy; c.y2 += dy;
    } else {
        c.x += dx; c.y += dy;
    }
    return c;
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

App::App(HWND hwnd, HINSTANCE inst) : hwnd_(hwnd), inst_(inst) {
    dpi_ = GetDpiForWindow(hwnd_);
    GESTURECONFIG gcZero{GID_ZOOM, 0, GC_ZOOM};
    SetGestureConfig(hwnd_, 0, 1, &gcZero, sizeof(gcZero));
    if (!dpi_) dpi_ = 96;
    sc_ = dpi_ / 96.0f;
    pages_.push_back(Page{});
    updateFonts();
    {
        HRSRC res = FindResourceW(inst_, MAKEINTRESOURCEW(IDI_LOGO), RT_RCDATA);
        if (res) {
            HGLOBAL hres = LoadResource(inst_, res);
            void* ptr = hres ? LockResource(hres) : nullptr;
            DWORD sz = SizeofResource(inst_, res);
            if (ptr && sz) {
                IStream* st = SHCreateMemStream((const BYTE*)ptr, sz);
                if (st) {
                    Bitmap* src = Bitmap::FromStream(st);
                    if (src && src->GetLastStatus() == Ok) {
                        Bitmap* copy = src->Clone(0, 0, src->GetWidth(), src->GetHeight(), PixelFormat32bppARGB);
                        if (copy) logo_.reset(copy);
                    }
                    delete src;
                    st->Release();
                }
            }
        }
    }
    loadConfig();
    fitView();
    updateWindowTitle();
    editBrush_ = CreateSolidBrush(RGB(255, 255, 255));
    splash_ = true;
    splashStart_ = GetTickCount64();
    SetTimer(hwnd_, 4, 16, nullptr);
}

App::~App() {
    for (auto& kv : imageCache_) delete kv.second;
    imageCache_.clear();
    if (editBrush_) DeleteObject(editBrush_);
    if (hEdit_) {
        DestroyWindow(hEdit_);
        hEdit_ = nullptr;
    }
    if (hNameEdit_) {
        DestroyWindow(hNameEdit_);
        hNameEdit_ = nullptr;
    }
}

void App::updateFonts() {
    auto mk = [&](float px, bool bold = false) -> std::unique_ptr<Font> {
        return std::make_unique<Font>(L"Microsoft YaHei UI", S(px),
                                      bold ? FontStyleBold : FontStyleRegular, UnitPixel);
    };
    fBrand_ = mk(23, true);
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
    fCapsule_ = mk(13.0f);
}

void App::invalidate() {
    baseDirty_ = true;
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void App::invalidateLive() {
    InvalidateRect(hwnd_, nullptr, FALSE);
    ULONGLONG now = GetTickCount64();
    if (now - lastLivePaint_ >= 10) {
        lastLivePaint_ = now;
        if (hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
    }
}

void App::invalidateLiveRect(const RECT& r) {
    InvalidateRect(hwnd_, &r, FALSE);
}

bool App::isLiveDrawing() const {
    if (splash_ || overlay_ != 0 || hEdit_) return false;
    if (drag_ == DragMode::Draw || drag_ == DragMode::Shape ||
        drag_ == DragMode::Laser || laserDown_ || !laser_.empty()) return true;
    if (tool_ == Tool::Eraser && mouseInside_) return true;
    return false;
}

void App::markDirty() {
    if (!dirty_) {
        dirty_ = true;
        updateWindowTitle();
    }
}

void App::updateWindowTitle() {
    if (!hwnd_) return;
    std::wstring t = (appName_.empty() ? std::wstring(APP_NAME) : appName_) + L" — " + title_;
    if (dirty_) t += L" *";
    SetWindowTextW(hwnd_, t.c_str());
}

// ---------------------------------------------------------------------------
// camera
// ---------------------------------------------------------------------------

float App::baseScale() const {
    if (W_ <= 0 || H_ <= 0) return 0.5f;
    return std::max(0.05f, std::max((float)W_ / PAGE_W, (float)H_ / PAGE_H));
}

Pt App::viewCenter() const {
    return {W_ / 2.0f, H_ / 2.0f};
}

Pt App::worldToScreen(Pt p) const {
    Pt vc = viewCenter();
    float s = viewScale();
    return {vc.x + (p.x - camCenter_.x) * s, vc.y + (p.y - camCenter_.y) * s};
}

Pt App::screenToWorld(Pt p) const {
    Pt vc = viewCenter();
    float s = viewScale();
    return {camCenter_.x + (p.x - vc.x) / s, camCenter_.y + (p.y - vc.y) / s};
}

Xf App::pageXf() const {
    Pt vc = viewCenter();
    float s = viewScale();
    Xf xf;
    xf.s = s;
    xf.o = {vc.x - camCenter_.x * s, vc.y - camCenter_.y * s};
    return xf;
}

RectF App::pageScreenRect() const {
    Xf xf = pageXf();
    return xf.rect(0, 0, PAGE_W, PAGE_H);
}

bool App::inPage(Pt screen) const {
    return pageScreenRect().Contains(screen.x, screen.y);
}

void App::fitView() {
    camScrollAnimating_ = false;
    zoom_ = 1.0f;
    camCenter_ = {PAGE_W / 2.0f, PAGE_H / 2.0f};
    invalidate();
}

void App::clampCam() {
    camCenter_.x = clampf(camCenter_.x, -PAGE_W * 0.45f, PAGE_W * 1.45f);
    camCenter_.y = clampf(camCenter_.y, -PAGE_H * 0.45f, PAGE_H * 1.45f);
}

void App::zoomAt(Pt screen, float newZoom) {
    newZoom = clampf(newZoom, 0.22f, 5.0f);
    Pt w = screenToWorld(screen);
    zoom_ = newZoom;
    Pt vc = viewCenter();
    float s = viewScale();
    camCenter_.x = w.x - (screen.x - vc.x) / s;
    camCenter_.y = w.y - (screen.y - vc.y) / s;
    clampCam();
    invalidate();
}

void App::zoomStep(float mul, Pt anchor) { zoomAt(anchor, zoom_ * mul); }

void App::onResize() {
    RECT rc;
    GetClientRect(hwnd_, &rc);
    int nw = rc.right - rc.left, nh = rc.bottom - rc.top;
    if (nw <= 0 || nh <= 0) return;
    W_ = nw; H_ = nh;
    if (!back_ || back_->GetWidth() != W_ || back_->GetHeight() != H_) {
        back_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
    }
    if (hEdit_) commitText();
    if (hNameEdit_) commitAppName();
    dockDragBase_.reset();
    moveBase_.reset();
    clampCam();
    invalidate();
}

void App::onDpiChanged(UINT dpi, const RECT* suggested) {
    commitText();
    commitAppName();
    dpi_ = dpi ? dpi : 96;
    sc_ = dpi_ / 96.0f;
    updateFonts();
    if (suggested) {
        SetWindowPos(hwnd_, nullptr, suggested->left, suggested->top,
                     suggested->right - suggested->left, suggested->bottom - suggested->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    onResize();
    invalidate();
}

// ---------------------------------------------------------------------------
// history / pages
// ---------------------------------------------------------------------------

void App::pushUndo() {
    pushUndoSnapshot({pages_, current_});
}

void App::pushUndoSnapshot(const Snapshot& s) {
    undo_.push_back(s);
    while (undo_.size() > 40) undo_.pop_front();
    redo_.clear();
    markDirty();
}

void App::restoreSnapshot(const Snapshot& s) {
    pages_ = s.pages;
    if (pages_.empty()) pages_.push_back(Page{});
    current_ = std::max(0, std::min(s.current, (int)pages_.size() - 1));
    sel_ = -1;
    invalidate();
}

void App::undo() {
    commitText();
    if (undo_.empty()) return;
    redo_.push_back(snapshot());
    Snapshot s = undo_.back();
    undo_.pop_back();
    restoreSnapshot(s);
    dirty_ = true;
    updateWindowTitle();
    showToast(L"已撤销", false, 1000);
}

void App::redo() {
    commitText();
    if (redo_.empty()) return;
    undo_.push_back(snapshot());
    Snapshot s = redo_.back();
    redo_.pop_back();
    restoreSnapshot(s);
    dirty_ = true;
    updateWindowTitle();
    showToast(L"已重做", false, 1000);
}

void App::switchPage(int index, bool closeOverlay) {
    commitText();
    if (index < 0 || index >= (int)pages_.size()) return;
    current_ = index;
    sel_ = -1;
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    if (closeOverlay) overlay_ = 0;
    invalidate();
}

void App::addPage() {
    commitText();
    pushUndo();
    Page p;
    p.pattern = pages_[current_].pattern;
    p.paper = pages_[current_].paper;
    pages_.insert(pages_.begin() + current_ + 1, p);
    current_ = current_ + 1;
    sel_ = -1;
    fitView();
    if (overlay_ == 1) pageScrollToCurrent_ = true;
    else {
        pageFadeStart_ = GetTickCount64();
        SetTimer(hwnd_, 5, 16, nullptr);
    }
    showToast(L"已新建第 " + std::to_wstring(current_ + 1) + L" 页", false, 1400);
    invalidate();
}

void App::movePage(int from, int to) {
    if (from < 0 || to < 0 || from >= pageCount() || to >= pageCount() || from == to) return;
    pushUndo();
    Page p = pages_[from];
    pages_.erase(pages_.begin() + from);
    pages_.insert(pages_.begin() + to, p);
    if (current_ == from) current_ = to;
    else if (from < current_ && to >= current_) --current_;
    else if (from > current_ && to <= current_) ++current_;
    sel_ = -1;
    dirty_ = true;
    updateWindowTitle();
    invalidate();
    showToast(L"页面顺序已调整", false, 1200);
}

void App::duplicatePage() {
    commitText();
    pushUndo();
    Page p = pages_[current_];
    pages_.insert(pages_.begin() + current_ + 1, p);
    current_ = current_ + 1;
    sel_ = -1;
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    showToast(L"已复制页面", false, 1400);
    invalidate();
}

void App::deletePage() {
    commitText();
    if (pages_.size() <= 1) {
        showToast(L"至少保留一个页面", true);
        return;
    }
    pushUndo();
    pages_.erase(pages_.begin() + current_);
    if (current_ >= (int)pages_.size()) current_ = (int)pages_.size() - 1;
    sel_ = -1;
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    showToast(L"已删除页面", false, 1400);
    invalidate();
}

void App::clearPage() {
    commitText();
    if (page().items.empty()) return;
    pushUndo();
    page().items.clear();
    page().touch();
    sel_ = -1;
    markDirty();
    invalidate();
    showToast(L"已清空本页 · Ctrl+Z 可撤销", false, 1900);
}

// ---------------------------------------------------------------------------
// selection
// ---------------------------------------------------------------------------

int App::hitItemAt(Pt world, float tolerancePx) const {
    float tol = tolerancePx / std::max(0.05f, viewScale());
    for (int i = (int)page().items.size() - 1; i >= 0; --i) {
        if (page().items[i].hit(world.x, world.y, tol)) return i;
    }
    return -1;
}

RectF App::selectionScreenRect() const {
    if (sel_ < 0 || sel_ >= (int)page().items.size()) return RectF(0, 0, 0, 0);
    Xf xf = pageXf();
    RectF b = page().items[sel_].bounds();
    return xf.rect(b.X, b.Y, b.Width, b.Height);
}

int App::hitHandle(int x, int y) const {
    if (sel_ < 0) return -1;
    RectF r = selectionScreenRect();
    float h = S(9);
    Pt pts[4] = {{r.X, r.Y}, {r.GetRight(), r.Y}, {r.X, r.GetBottom()}, {r.GetRight(), r.GetBottom()}};
    for (int i = 0; i < 4; ++i) {
        if (std::fabs(x - pts[i].x) <= h && std::fabs(y - pts[i].y) <= h) return i;
    }
    return -1;
}

void App::deleteSelection() {
    if (sel_ < 0 || sel_ >= (int)page().items.size()) return;
    pushUndo();
    page().items.erase(page().items.begin() + sel_);
    page().touch();
    sel_ = -1;
    markDirty();
    invalidate();
}

void App::duplicateSelection() {
    if (sel_ < 0 || sel_ >= (int)page().items.size()) return;
    pushUndo();
    Item c = page().items[sel_].shifted(22, 22);
    c.id = nextId_++;
    page().items.push_back(c);
    sel_ = (int)page().items.size() - 1;
    page().touch();
    markDirty();
    invalidate();
}

void App::bringSelection(bool front) {
    if (sel_ < 0 || sel_ >= (int)page().items.size()) return;
    pushUndo();
    Item it = page().items[sel_];
    page().items.erase(page().items.begin() + sel_);
    if (front) {
        page().items.push_back(it);
        sel_ = (int)page().items.size() - 1;
    } else {
        page().items.insert(page().items.begin(), it);
        sel_ = 0;
    }
    page().touch();
    markDirty();
    invalidate();
}

// ---------------------------------------------------------------------------
// text editor
// ---------------------------------------------------------------------------

static WNDPROC g_oldEditProc = nullptr;
static App* g_editOwner = nullptr;

static LRESULT CALLBACK EditSubclass(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN) {
        if (wp == VK_RETURN && (GetKeyState(VK_CONTROL) & 0x8000)) {
            if (g_editOwner) g_editOwner->commitText();
            return 0;
        }
        if (wp == VK_ESCAPE) {
            if (g_editOwner) g_editOwner->commitText(true);
            return 0;
        }
    }
    if (msg == WM_KILLFOCUS && g_editOwner) {
        PostMessageW(g_editOwner->hwnd(), App::WM_APP_COMMIT_TEXT, 0, 0);
    }
    return CallWindowProcW(g_oldEditProc, h, msg, wp, lp);
}

HFONT App::makeEditFont() {
    float px = (editIndex_ >= 0 && editIndex_ < (int)page().items.size())
                   ? page().items[editIndex_].fontSize * viewScale()
                   : textSize_ * viewScale();
    if (px < 10) px = 10;
    return CreateFontW(-(int)std::round(px), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
}

void App::beginTextEdit(Pt world, int existing) {
    commitText();
    if (existing >= 0) {
        if (existing >= (int)page().items.size() || page().items[existing].kind != ItemKind::Text) return;
        editIndex_ = existing;
        const Item& it = page().items[existing];
        editX_ = it.x;
        editY_ = it.y;
        editWidth_ = it.textWidth;
        editOriginal_ = it.text;
        textSize_ = it.fontSize;
        textColor_ = it.color;
    } else {
        editIndex_ = -1;
        editWidth_ = 420;
        editX_ = clampf(world.x, 4, PAGE_W - editWidth_ - 12);
        editY_ = clampf(world.y, 4, PAGE_H - textSize_ * 4.2f);
        editOriginal_.clear();
    }
    createEditControl();
    invalidate();
}

void App::createEditControl() {
    if (hEdit_) {
        DestroyWindow(hEdit_);
        hEdit_ = nullptr;
    }
    hEdit_ = CreateWindowExW(0, L"EDIT", L"",
                             WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL |
                                 ES_WANTRETURN | WS_BORDER | ES_LEFT,
                             0, 0, 100, 60, hwnd_, nullptr, inst_, nullptr);
    if (!hEdit_) return;
    HFONT hf = makeEditFont();
    SendMessageW(hEdit_, WM_SETFONT, (WPARAM)hf, TRUE);
    SendMessageW(hEdit_, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(7, 7));
    SendMessageW(hEdit_, EM_SETLIMITTEXT, 16000, 0);
    if (!editOriginal_.empty()) SetWindowTextW(hEdit_, editOriginal_.c_str());
    g_oldEditProc = (WNDPROC)SetWindowLongPtrW(hEdit_, GWLP_WNDPROC, (LONG_PTR)EditSubclass);
    g_editOwner = this;
    layoutEditControl();
    SetFocus(hEdit_);
    SendMessageW(hEdit_, EM_SETSEL, 0, -1);
    SendMessageW(hEdit_, EM_SETSEL, editOriginal_.size(), editOriginal_.size());
}

void App::layoutEditControl() {
    if (!hEdit_) return;
    float fs = (editIndex_ >= 0 && editIndex_ < (int)page().items.size())
                   ? page().items[editIndex_].fontSize
                   : textSize_;
    float s = viewScale();
    float fpx = std::max(10.0f, fs * s);
    float widthScreen = clampf(editWidth_ * s, S(220), S(680));
    editWidth_ = widthScreen / s;
    float heightScreen = std::max(S(76), fpx * 3.25f);
    Pt p = worldToScreen({editX_, editY_});
    int x = (int)std::round(p.x);
    int y = (int)std::round(p.y);
    int w = (int)std::round(widthScreen);
    int h = (int)std::round(heightScreen);
    x = (int)clampf((float)x, S(8), (float)W_ - w - S(8));
    y = (int)clampf((float)y, headerH() + S(8), (float)H_ - h - S(8));
    SetWindowPos(hEdit_, HWND_TOP, x, y, w, h, SWP_NOACTIVATE);
    HFONT hf = makeEditFont();
    SendMessageW(hEdit_, WM_SETFONT, (WPARAM)hf, TRUE);
}

void App::commitText(bool cancel) {
    if (!hEdit_ || textCommitting_) return;
    textCommitting_ = true;
    int len = GetWindowTextLengthW(hEdit_);
    std::wstring value;
    if (len > 0) {
        value.resize((size_t)len + 1);
        GetWindowTextW(hEdit_, value.data(), len + 1);
        value.resize((size_t)len);
    }
    HWND editing = hEdit_;
    hEdit_ = nullptr;
    g_editOwner = nullptr;
    DestroyWindow(editing);
    if (cancel) {
        textCommitting_ = false;
        SetFocus(hwnd_);
        invalidate();
        return;
    }
    std::wstring text = trimW(value);
    if (editIndex_ >= 0 && editIndex_ < (int)page().items.size() &&
        page().items[editIndex_].kind == ItemKind::Text) {
        if (text.empty()) {
            pushUndo();
            page().items.erase(page().items.begin() + editIndex_);
            page().touch();
            sel_ = -1;
            markDirty();
        } else if (text != editOriginal_) {
            pushUndo();
            page().items[editIndex_].text = text;
            page().items[editIndex_].textWidth = editWidth_;
            page().touch();
            markDirty();
        }
        tool_ = Tool::Select;
    } else if (!text.empty()) {
        pushUndo();
        Item it;
        it.kind = ItemKind::Text;
        it.id = nextId_++;
        it.text = text;
        it.color = textColor_;
        it.fontSize = textSize_;
        it.textWidth = editWidth_;
        it.x = editX_;
        it.y = editY_;
        page().items.push_back(it);
        page().touch();
        sel_ = -1;
        markDirty();
        tool_ = Tool::Select;
    }
    editIndex_ = -1;
    textCommitting_ = false;
    SetFocus(hwnd_);
    invalidate();
}

// ---------------------------------------------------------------------------
// serialization / files
// ---------------------------------------------------------------------------

static std::string nf(float v) { return utf8(fmtFloat(v, 2)); }

std::string App::serializeDocument() const {
    std::ostringstream ss;
    ss << "QHB2\n";
    ss << "TITLE " << b64encode(utf8(title_)) << "\n";
    ss << "CURRENT " << current_ << "\n";
    ss << "PAGES " << pages_.size() << "\n";
    for (const auto& p : pages_) {
        ss << "PAGE " << utf8(p.pattern) << " #" << hexColor(p.paper) << " " << p.items.size() << "\n";
        for (const auto& it : p.items) {
            if (it.kind == ItemKind::Stroke) {
                ss << "STROKE " << utf8(it.strokeTool) << " #" << hexColor(it.color) << " "
                   << nf(it.width) << " " << it.points.size();
                for (auto& pt : it.points) ss << " " << nf(pt.x) << "," << nf(pt.y);
                if (it.functionGraph) {
                    ss << " FUNC " << (it.showAxes ? 1 : 0) << " " << nf(it.originX) << " "
                       << nf(it.originY) << " " << nf(it.gLeft) << " " << nf(it.gTop) << " "
                       << nf(it.gRight) << " " << nf(it.gBottom) << " " << nf(it.unitX) << " "
                       << nf(it.unitY);
                }
                if (!it.funcExpr.empty()) ss << " EXPR " << b64encode(it.funcExpr);
                ss << "\n";
            } else if (it.kind == ItemKind::Shape) {
                const char* sk = "rect";
                if (it.shape == ShapeKind::Line) sk = "line";
                else if (it.shape == ShapeKind::Arrow) sk = "arrow";
                else if (it.shape == ShapeKind::Ellipse) sk = "ellipse";
                ss << "SHAPE " << sk << " #" << hexColor(it.color) << " " << nf(it.width) << " "
                   << (it.filled ? 1 : 0) << " " << nf(it.x1) << " " << nf(it.y1) << " "
                   << nf(it.x2) << " " << nf(it.y2);
                if (it.isPoint) ss << " POINT " << b64encode(it.pointText);
                ss << "\n";
            } else if (it.kind == ItemKind::Text) {
                ss << "TEXT #" << hexColor(it.color) << " " << nf(it.fontSize) << " "
                   << nf(it.textWidth) << " " << nf(it.x) << " " << nf(it.y) << " "
                   << b64encode(utf8(it.text)) << "\n";
            } else if (it.kind == ItemKind::Image) {
                ss << "IMAGE " << nf(it.x) << " " << nf(it.y) << " " << nf(it.w) << " "
                   << nf(it.h) << " " << it.data << "\n";
            }
        }
    }
    return ss.str();
}

bool App::loadDocumentData(const std::string& data, std::wstring& err) {
    std::vector<std::string> lines;
    std::string cur;
    for (char c : data) {
        if (c == '\n') {
            if (!cur.empty() && cur.back() == '\r') cur.pop_back();
            lines.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) lines.push_back(cur);
    if (lines.empty() || trimCopy(lines[0]) != "QHB2") {
        err = L"文件格式不正确或版本过旧。";
        return false;
    }
    size_t i = 1;
    auto next = [&](std::string& out) -> bool {
        if (i >= lines.size()) return false;
        out = lines[i++];
        return true;
    };
    std::string line, titleB64;
    if (!next(line)) { err = L"文件内容不完整。"; return false; }
    if (line.rfind("TITLE ", 0) == 0) titleB64 = trimCopy(line.substr(6));
    int savedCurrent = 0, pageCount = 0;
    if (!next(line)) { err = L"文件内容不完整。"; return false; }
    if (line.rfind("CURRENT ", 0) == 0) savedCurrent = std::stoi(line.substr(8));
    if (!next(line)) { err = L"文件内容不完整。"; return false; }
    if (line.rfind("PAGES ", 0) == 0) pageCount = std::stoi(line.substr(6));
    if (pageCount <= 0 || pageCount > 1000) { err = L"页面数量无效。"; return false; }

    std::vector<Page> loaded;
    loaded.reserve(pageCount);
    for (int pi = 0; pi < pageCount; ++pi) {
        if (!next(line)) { err = L"页面数据不完整。"; return false; }
        std::istringstream ph(line);
        std::string tag, pattern, paper;
        int count = 0;
        if (!(ph >> tag >> pattern >> paper >> count) || tag != "PAGE") {
            err = L"页面数据损坏。";
            return false;
        }
        Page p;
        p.pattern = wide(pattern);
        p.paper = hexColor(paper);
        p.revision = 1;
        if (count < 0 || count > 200000) { err = L"页面对象数量无效。"; return false; }
        for (int k = 0; k < count; ++k) {
            if (!next(line)) { err = L"图元数据不完整。"; return false; }
            std::istringstream is(line);
            std::string kind;
            if (!(is >> kind)) { err = L"图元数据损坏。"; return false; }
            Item it;
            it.id = nextId_++;
            if (kind == "STROKE") {
                std::string tool, color;
                int n = 0;
                if (!(is >> tool >> color >> it.width >> n)) { err = L"笔迹数据损坏。"; return false; }
                it.kind = ItemKind::Stroke;
                it.strokeTool = wide(tool);
                it.color = hexColor(color);
                it.points.reserve(std::max(0, n));
                for (int q = 0; q < n; ++q) {
                    std::string xy;
                    if (!(is >> xy)) { err = L"笔迹坐标缺失。"; return false; }
                    size_t comma = xy.find(',');
                    if (comma == std::string::npos) { err = L"笔迹坐标格式错误。"; return false; }
                    it.points.push_back({std::stof(xy.substr(0, comma)), std::stof(xy.substr(comma + 1))});
                }
                std::string extra;
                while (is >> extra) {
                    if (extra == "FUNC") {
                        int axes = 1;
                        if (!(is >> axes >> it.originX >> it.originY >> it.gLeft >> it.gTop
                              >> it.gRight >> it.gBottom >> it.unitX >> it.unitY)) {
                            err = L"函数图像数据损坏。";
                            return false;
                        }
                        it.functionGraph = true;
                        it.showAxes = axes != 0;
                    } else if (extra == "EXPR") {
                        std::string e;
                        if (is >> e) it.funcExpr = b64decode(e);
                    }
                }
            } else if (kind == "SHAPE") {
                std::string sk, color;
                int fill = 0;
                if (!(is >> sk >> color >> it.width >> fill >> it.x1 >> it.y1 >> it.x2 >> it.y2)) {
                    err = L"形状数据损坏。";
                    return false;
                }
                it.kind = ItemKind::Shape;
                it.color = hexColor(color);
                it.filled = fill != 0;
                if (sk == "line") it.shape = ShapeKind::Line;
                else if (sk == "arrow") it.shape = ShapeKind::Arrow;
                else if (sk == "ellipse") it.shape = ShapeKind::Ellipse;
                else it.shape = ShapeKind::Rect;
                std::string extra;
                if (is >> extra) {
                    if (extra == "POINT") {
                        std::string pt;
                        if (is >> pt) {
                            it.isPoint = true;
                            it.pointText = b64decode(pt);
                        }
                    }
                }
            } else if (kind == "TEXT") {
                std::string color, text64;
                if (!(is >> color >> it.fontSize >> it.textWidth >> it.x >> it.y >> text64)) {
                    err = L"文字数据损坏。";
                    return false;
                }
                it.kind = ItemKind::Text;
                it.color = hexColor(color);
                it.text = wide(b64decode(text64));
            } else if (kind == "IMAGE") {
                if (!(is >> it.x >> it.y >> it.w >> it.h >> it.data)) {
                    err = L"图片数据损坏。";
                    return false;
                }
                it.kind = ItemKind::Image;
            } else {
                err = L"包含未知图元。";
                return false;
            }
            p.items.push_back(std::move(it));
        }
        loaded.push_back(std::move(p));
    }
    if (loaded.empty()) { err = L"文件没有页面。"; return false; }
    pages_ = std::move(loaded);
    current_ = std::max(0, std::min(savedCurrent, (int)pages_.size() - 1));
    if (!titleB64.empty()) title_ = wide(b64decode(titleB64));
    sel_ = -1;
    overlay_ = 0;
    bgPanel_ = false;
    dirty_ = false;
    fitView();
    clearImageCache();
    updateWindowTitle();
    invalidate();
    return true;
}

static bool readAllBytes(const std::wstring& path, std::string& out) {
    FILE* f = _wfopen(path.c_str(), L"rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) { fclose(f); return false; }
    out.resize((size_t)size);
    size_t got = size ? fread(out.data(), 1, (size_t)size, f) : 0;
    fclose(f);
    if (got != (size_t)size) return false;
    return true;
}

std::wstring App::chooseSavePath(const wchar_t* filter, const wchar_t* defExt, const wchar_t* defaultName) {
    wchar_t buf[2048] = {};
    if (defaultName && *defaultName) wcsncpy(buf, defaultName, 2047);
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd_;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = 2048;
    ofn.lpstrDefExt = defExt;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
    if (!GetSaveFileNameW(&ofn)) return L"";
    return std::wstring(buf);
}

std::wstring App::chooseOpenPath(const wchar_t* filter) {
    wchar_t buf[2048] = {};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd_;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = 2048;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
    if (!GetOpenFileNameW(&ofn)) return L"";
    return std::wstring(buf);
}

void App::setTitleFromPath() {
    std::wstring t = stemOf(path_);
    title_ = t.empty() ? L"未命名白板" : t;
}

bool App::saveDocument(bool saveAs) {
    commitText();
    std::wstring p = path_;
    if (p.empty() || saveAs) {
        std::wstring def = (title_.empty() ? L"未命名白板" : title_) + L".hgwb";
        p = chooseSavePath(
            L"HAVEGET白板文件 (*.hgwb)\0*.hgwb\0旧版轻画板文件 (*.qhb)\0*.qhb\0所有文件 (*.*)\0*.*\0\0",
            L"hgwb", def.c_str());
        if (p.empty()) return false;
    }
    std::wstring oldTitle = title_;
    std::wstring newTitle = stemOf(p);
    title_ = newTitle.empty() ? L"未命名白板" : newTitle;
    std::string data = serializeDocument();
    FILE* f = _wfopen(p.c_str(), L"wb");
    if (!f) {
        title_ = oldTitle;
        showMessage(L"无法写入文件，请检查保存位置和权限。", true);
        return false;
    }
    size_t wrote = data.empty() ? 0 : fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    if (wrote != data.size()) {
        title_ = oldTitle;
        showMessage(L"保存文件时发生错误。", true);
        return false;
    }
    path_ = p;
    dirty_ = false;
    updateWindowTitle();
    showToast(L"已保存 · " + fileNameOf(path_), false, 2400);
    return true;
}

bool App::openDocumentDialog() {
    commitText();
    if (!confirmDiscard()) return false;
    std::wstring p = chooseOpenPath(
        L"HAVEGET白板文件 (*.hgwb)\0*.hgwb\0旧版轻画板文件 (*.qhb)\0*.qhb\0所有文件 (*.*)\0*.*\0\0");
    if (p.empty()) return false;
    std::string raw;
    if (!readAllBytes(p, raw)) {
        showMessage(L"无法读取文件。", true);
        return false;
    }
    std::wstring err;
    if (!loadDocumentData(raw, err)) {
        showMessage(err.empty() ? L"打开失败。" : err, true);
        return false;
    }
    path_ = p;
    setTitleFromPath();
    dirty_ = false;
    undo_.clear();
    redo_.clear();
    updateWindowTitle();
    invalidate();
    showToast(L"已打开 · " + fileNameOf(p), false, 2200);
    return true;
}

bool App::exportPng() {
    commitText();
    std::wstring def = (title_.empty() ? L"白板" : title_) + L".png";
    std::wstring p = chooseSavePath(
        L"PNG 图片 (*.png)\0*.png\0所有文件 (*.*)\0*.*\0\0",
        L"png", def.c_str());
    if (p.empty()) return false;
    const int OW = 2400, OH = 1350;
    Bitmap bmp(OW, OH, PixelFormat32bppARGB);
    Graphics g(&bmp);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    Xf xf;
    xf.s = (float)OW / PAGE_W;
    xf.o = {0, 0};
    drawPageContent(g, page(), xf, RectF(0, 0, (float)OW, (float)OH), false);
    if (!saveBitmapPng(&bmp, p)) {
        showMessage(L"导出 PNG 失败。", true);
        return false;
    }
    showToast(L"已导出 PNG · " + fileNameOf(p), false, 2600);
    return true;
}

void App::newDocument() {
    commitText();
    if (!confirmDiscard()) return;
    pages_.clear();
    pages_.push_back(Page{});
    current_ = 0;
    path_.clear();
    title_ = L"未命名白板";
    dirty_ = false;
    sel_ = -1;
    overlay_ = 0;
    bgPanel_ = false;
    undo_.clear();
    redo_.clear();
    clearImageCache();
    fitView();
    pageFadeStart_ = GetTickCount64();
    SetTimer(hwnd_, 5, 16, nullptr);
    updateWindowTitle();
    invalidate();
    showToast(L"已新建空白白板", false, 1600);
}

bool App::confirmDiscard() {
    commitText();
    if (!dirty_) return true;
    int r = MessageBoxW(hwnd_, L"当前白板有未保存的更改，要先保存吗？", APP_NAME,
                        MB_YESNOCANCEL | MB_ICONQUESTION);
    if (r == IDCANCEL) return false;
    if (r == IDYES) return saveDocument(false);
    return true;
}

void App::showMessage(const std::wstring& text, bool error) {
    MessageBoxW(hwnd_, text.c_str(), APP_NAME, MB_OK | (error ? MB_ICONWARNING : MB_ICONINFORMATION));
}

void App::showToast(const std::wstring& text, bool error, int ms) {
    toast_ = text;
    toastError_ = error;
    toastUntil_ = GetTickCount64() + (ULONGLONG)ms;
    toastAnimStart_ = GetTickCount64();
    SetTimer(hwnd_, 1, (UINT)ms + 30, nullptr);
    SetTimer(hwnd_, 5, 16, nullptr);
    invalidate();
}

// ---------------------------------------------------------------------------
// image
// ---------------------------------------------------------------------------

void App::clearImageCache() {
    for (auto& kv : imageCache_) delete kv.second;
    imageCache_.clear();
}

Bitmap* App::getImage(const Item& it) {
    auto found = imageCache_.find(it.id);
    if (found != imageCache_.end()) return found->second;
    if (it.data.empty()) return nullptr;
    std::string raw = b64decode(it.data);
    if (raw.empty()) return nullptr;
    IStream* stream = SHCreateMemStream((const BYTE*)raw.data(), (UINT)raw.size());
    if (!stream) return nullptr;
    Bitmap* src = Bitmap::FromStream(stream);
    Bitmap* copy = nullptr;
    if (src && src->GetLastStatus() == Ok) {
        copy = src->Clone(0, 0, src->GetWidth(), src->GetHeight(), PixelFormat32bppARGB);
    }
    delete src;
    stream->Release();
    if (copy) imageCache_[it.id] = copy;
    return copy;
}

std::string App::encodeImageBase64(const std::wstring& file, float& outW, float& outH) {
    Bitmap src(file.c_str());
    if (src.GetLastStatus() != Ok) return {};
    int w = (int)src.GetWidth(), h = (int)src.GetHeight();
    if (w <= 0 || h <= 0) return {};
    int maxDim = 2000;
    Bitmap* work = &src;
    std::unique_ptr<Bitmap> resized;
    if (std::max(w, h) > maxDim) {
        float k = (float)maxDim / (float)std::max(w, h);
        int nw = std::max(1, (int)std::round(w * k));
        int nh = std::max(1, (int)std::round(h * k));
        resized = std::make_unique<Bitmap>(nw, nh, PixelFormat32bppARGB);
        Graphics g(resized.get());
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.SetSmoothingMode(SmoothingModeHighQuality);
        g.DrawImage(&src, RectF(0, 0, (float)nw, (float)nh));
        work = resized.get();
    }
    outW = (float)w;
    outH = (float)h;
    return encodePngBase64(work);
}

void App::insertImageFile(const std::wstring& file) {
    float iw = 0, ih = 0;
    std::string data = encodeImageBase64(file, iw, ih);
    if (data.empty() || iw <= 0 || ih <= 0) {
        showMessage(L"无法读取这张图片。支持 PNG、JPG、BMP、GIF 等常见格式。", true);
        return;
    }
    float w = iw, h = ih;
    float k = std::min({PAGE_W * 0.62f / w, PAGE_H * 0.62f / h, 1.0f});
    w *= k;
    h *= k;
    Item it;
    it.kind = ItemKind::Image;
    it.id = nextId_++;
    it.x = (PAGE_W - w) / 2;
    it.y = (PAGE_H - h) / 2;
    it.w = w;
    it.h = h;
    it.data = data;
    pushUndo();
    page().items.push_back(it);
    page().touch();
    sel_ = (int)page().items.size() - 1;
    tool_ = Tool::Select;
    markDirty();
    invalidate();
    showToast(L"图片已插入，可拖动或缩放", false, 1900);
}

void App::insertImageDialog() {
    commitText();
    std::wstring p = chooseOpenPath(
        L"图片文件 (*.png;*.jpg;*.jpeg;*.bmp;*.gif)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif\0所有文件 (*.*)\0*.*\0\0");
    if (p.empty()) return;
    insertImageFile(p);
}

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

void App::onDrop(HDROP drop) {
    UINT n = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
    std::vector<std::wstring> files;
    for (UINT i = 0; i < n; ++i) {
        wchar_t buf[2048] = {};
        if (DragQueryFileW(drop, i, buf, 2048)) files.push_back(buf);
    }
    DragFinish(drop);
    if (files.empty()) return;
    std::wstring ext = lowerExt(files[0]);
    if (ext == L".hgwb" || ext == L".qhb") {
        commitText();
        if (!confirmDiscard()) return;
        std::string raw;
        if (!readAllBytes(files[0], raw)) {
            showMessage(L"无法读取拖入的文件。", true);
            return;
        }
        std::wstring err;
        if (!loadDocumentData(raw, err)) {
            showMessage(err.empty() ? L"无法打开文件。" : err, true);
            return;
        }
        path_ = files[0];
        setTitleFromPath();
        dirty_ = false;
        undo_.clear();
        redo_.clear();
        updateWindowTitle();
        showToast(L"已打开 · " + fileNameOf(path_), false, 2000);
    } else {
        for (auto& f : files) insertImageFile(f);
    }
}

// ---------------------------------------------------------------------------
// tools / UI actions
// ---------------------------------------------------------------------------

void App::setTool(Tool t) {
    bool changed = tool_ != t;
    tool_ = t;
    bgPanel_ = false;
    sel_ = -1;
    if (changed) {
        toolAnim_ = 0.0f;
        toolAnimStart_ = GetTickCount64();
        SetTimer(hwnd_, 5, 16, nullptr);
    }
    invalidate();
}

static bool startsWith(const std::string& s, const char* p) {
    return s.rfind(p, 0) == 0;
}

static uint32_t chooseColorDialog(HWND owner, uint32_t initial) {
    static COLORREF custom[16] = {};
    CHOOSECOLORW cc{};
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = owner;
    cc.rgbResult = RGB((initial >> 16) & 255, (initial >> 8) & 255, initial & 255);
    cc.lpCustColors = custom;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColorW(&cc)) {
        return ((uint32_t)GetRValue(cc.rgbResult) << 16) |
               ((uint32_t)GetGValue(cc.rgbResult) << 8) |
               (uint32_t)GetBValue(cc.rgbResult);
    }
    return initial;
}

void App::handleRegion(const std::string& id) {
    if (id.empty() || id == "ov:panel" || id == "ov:scrim:keep") return;
    commitText();
    if (hNameEdit_ && id != "settings:name_save") commitAppName();

    if (id == "tool:select") { setTool(Tool::Select); return; }
    if (id == "tool:pen") { setTool(Tool::Pen); return; }
    if (id == "tool:highlighter") { setTool(Tool::Highlighter); return; }
    if (id == "tool:eraser") { setTool(Tool::Eraser); return; }
    if (id == "tool:shape") { setTool(Tool::Shape); return; }
    if (id == "tool:text") { setTool(Tool::Text); return; }
    if (id == "tool:laser") { setTool(Tool::Laser); return; }
    if (id == "tool:hand" || id == "rail:hand") { setTool(Tool::Hand); return; }
    if (id == "tool:background" || id == "rail:bg") { setTool(Tool::Background); return; }
    if (id == "rail:image" || id == "dock:image") { insertImageDialog(); return; }
    if (id == "header:undo" || id == "dock:undo") { undo(); return; }
    if (id == "header:redo" || id == "dock:redo") { redo(); return; }
    if (id == "header:open" || id == "dock:open") { openDocumentDialog(); return; }
    if (id == "header:save" || id == "dock:save") { saveDocument(false); return; }
    if (id == "header:export" || id == "dock:export") { exportPng(); return; }
    if (id == "header:fullscreen" || id == "dock:fullscreen") { toggleFullscreen(); return; }
    if (id == "header:help" || id == "dock:help") { overlay_ = 2; bgPanel_ = false; invalidate(); return; }
    if (id == "dock:settings" || id == "settings:open") {
        commitText();
        overlay_ = 3;
        settingsAnim_ = 0.0f;
        settingsClosing_ = false;
        settingsAnimStart_ = GetTickCount64();
        bgPanel_ = false;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
        return;
    }
    if (id == "page:prev") { if (current_ > 0) switchPage(current_ - 1); return; }
    if (id == "page:next") { if (current_ + 1 < pageCount()) switchPage(current_ + 1); return; }
    if (id == "page:label") { overlay_ = 1; bgPanel_ = false; pageScrollToCurrent_ = true; invalidate(); return; }
    if (id == "dock:function") {
        fnExpr_.clear();
        fnError_.clear();
        bgPanel_ = false;
        overlay_ = 4;
        invalidate();
        return;
    }
    if (id == "devpass:ok") { verifyDevPassword(); return; }
    if (id == "devpass:cancel") {
        overlay_ = 3;
        devPass_.clear();
        devPassError_.clear();
        invalidate();
        return;
    }
    if (id == "fn:insert") { submitFunctionDialog(); return; }
    if (id == "fn:close") { overlay_ = 0; invalidate(); return; }
    if (startsWith(id, "fn:example:")) {
        fnExpr_ = id.substr(std::string("fn:example:").size());
        fnError_.clear();
        invalidate();
        return;
    }
    if (id == "fn:square") {
        if (fnExpr_.empty()) fnExpr_ = "x^2";
        else fnExpr_ += "^2";
        fnError_.clear();
        invalidate();
        return;
    }
    if (id == "fn:axes") {
        fnShowAxes_ = !fnShowAxes_;
        invalidate();
        return;
    }
    if (id == "page:add") { addPage(); return; }
    if (id == "zoom:out") { zoomStep(0.85f, viewCenter()); return; }
    if (id == "zoom:in") { zoomStep(1.18f, viewCenter()); return; }
    if (id == "zoom:reset" || id == "zoom:fit") { fitView(); return; }
    if (id == "sel:copy") { duplicateSelection(); return; }
    if (id == "sel:delete") { deleteSelection(); return; }
    if (id == "sel:front") { bringSelection(true); return; }
    if (id == "sel:back") { bringSelection(false); return; }
    if (id == "clearpage") { clearPage(); return; }
    if (id == "fill") { shapeFilled_ = !shapeFilled_; invalidate(); return; }
    if (id == "bg:close") { bgPanel_ = false; invalidate(); return; }
    if ((id == "settings:close" || id == "ov:scrim") && overlay_ == 3) {
        commitAppName();
        settingsClosing_ = true;
        settingsAnimStart_ = GetTickCount64();
        SetTimer(hwnd_, 5, 16, nullptr);
        return;
    }
    if (id == "ov:pages:close" || id == "ov:help:close" || id == "ov:scrim" || id == "settings:close") {
        commitAppName();
        overlay_ = 0;
        invalidate();
        return;
    }
    if (id == "ov:pages:new") { addPage(); invalidate(); return; }
    if (id == "ov:pages:dup") { duplicatePage(); invalidate(); return; }
    if (id == "ov:pages:del") { deletePage(); invalidate(); return; }

    if (startsWith(id, "ov:pages:page:")) {
        int idx = std::atoi(id.c_str() + 14);
        switchPage(idx);
        overlay_ = 0;
        invalidate();
        return;
    }
    if (startsWith(id, "color:")) {
        uint32_t c = hexColor(id.substr(6));
        if (tool_ == Tool::Pen) penColor_ = c;
        else if (tool_ == Tool::Highlighter) highlighterColor_ = c;
        else if (tool_ == Tool::Shape) shapeColor_ = c;
        else if (tool_ == Tool::Text) textColor_ = c;
        else if (tool_ == Tool::Laser) laserColor_ = c;
        invalidate();
        return;
    }
    if (id == "customcolor") {
        uint32_t cur = COL_INK;
        if (tool_ == Tool::Pen) cur = penColor_;
        else if (tool_ == Tool::Highlighter) cur = highlighterColor_;
        else if (tool_ == Tool::Shape) cur = shapeColor_;
        else if (tool_ == Tool::Text) cur = textColor_;
        else if (tool_ == Tool::Laser) cur = laserColor_;
        uint32_t c = chooseColorDialog(hwnd_, cur);
        if (tool_ == Tool::Pen) penColor_ = c;
        else if (tool_ == Tool::Highlighter) highlighterColor_ = c;
        else if (tool_ == Tool::Shape) shapeColor_ = c;
        else if (tool_ == Tool::Text) textColor_ = c;
        else if (tool_ == Tool::Laser) laserColor_ = c;
        invalidate();
        return;
    }
    if (startsWith(id, "width:")) {
        float v = (float)std::atof(id.c_str() + 6);
        if (tool_ == Tool::Pen) penWidth_ = v;
        else if (tool_ == Tool::Highlighter) highlighterWidth_ = v;
        else if (tool_ == Tool::Eraser) eraserSize_ = v;
        else if (tool_ == Tool::Shape) shapeWidth_ = v;
        invalidate();
        return;
    }
    if (startsWith(id, "shape:")) {
        int v = std::atoi(id.c_str() + 6);
        shapeKind_ = (ShapeKind)std::max(0, std::min(3, v));
        tool_ = Tool::Shape;
        sel_ = -1;
        invalidate();
        return;
    }
    if (startsWith(id, "textsize:")) {
        textSize_ = (float)std::atof(id.c_str() + 9);
        invalidate();
        return;
    }
    if (startsWith(id, "bg:pattern:")) {
        std::wstring pat = wide(id.substr(11));
        if (pat != page().pattern) {
            pushUndo();
            page().pattern = pat;
            page().touch();
            markDirty();
        }
        invalidate();
        return;
    }
    if (startsWith(id, "bg:paper:")) {
        uint32_t c = hexColor(id.substr(9));
        if (c != page().paper) {
            pushUndo();
            page().paper = c;
            page().touch();
            markDirty();
        }
        invalidate();
        return;
    }

    if (id == "settings:version") {
        ULONGLONG now = GetTickCount64();
        if (now - versionClickLast_ > 2200) versionClicks_ = 0;
        versionClickLast_ = now;
        ++versionClicks_;
        if (versionClicks_ >= 8) {
            versionClicks_ = 0;
            devPass_.clear();
            devPassError_.clear();
            overlay_ = 6;
            invalidate();
        } else if (versionClicks_ >= 5) {
            showToast(L"再点击 " + std::to_wstring(8 - versionClicks_) + L" 次启用开发者模式", false, 900);
        }
        return;
    }
    if (id == "settings:dock_reset") { resetDockPosition(); return; }
    if (id == "settings:apply_all") { applyAllPages_ = !applyAllPages_; invalidate(); return; }
    if (id == "settings:launcher_bg") { chooseLauncherBackground(); return; }
    if (id == "settings:launcher_bg_clear") { clearLauncherBackground(); return; }
    if (id == "settings:logo") { chooseCustomLogo(); return; }
    if (id == "settings:logo_clear") { clearCustomLogo(); return; }
    if (id == "settings:name_edit") { beginAppNameEdit(); return; }
    if (id == "settings:name_save") { commitAppName(); return; }
    if (id == "settings:dev_off") { devMode_ = false; devUnlocked_ = false; saveConfig(); invalidate(); return; }
    if (id == "settings:reset_look") {
        appName_ = APP_NAME;
        clearLauncherBackground();
        clearCustomLogo();
        saveConfig();
        updateWindowTitle();
        invalidate();
        showToast(L"外观已恢复默认", false, 1600);
        return;
    }
    if (startsWith(id, "settings:canvas_color:")) {
        applyCanvasColor(hexColor(id.substr(22)), applyAllPages_);
        return;
    }
    if (id == "settings:canvas_custom" || id == "canvas:color_custom") {
        uint32_t c = chooseColorDialog(hwnd_, page().paper);
        applyCanvasColor(c, applyAllPages_ && id == "settings:canvas_custom");
        return;
    }
    if (startsWith(id, "canvas:color:")) {
        applyCanvasColor(hexColor(id.substr(13)), false);
        return;
    }
    if (startsWith(id, "canvas:pattern:") || startsWith(id, "bg:pattern:")) {
        size_t off = startsWith(id, "canvas:pattern:") ? 15 : 11;
        std::wstring pat = wide(id.substr(off));
        if (pat != page().pattern) {
            pushUndo();
            page().pattern = pat;
            page().touch();
            markDirty();
        }
        invalidate();
        return;
    }

}

void App::addRegion(const std::string& id, RectF r, const std::wstring& tip, CursorKind cur, bool enabled) {
    if (!enabled) return;
    Region reg;
    reg.id = id;
    reg.rect = r;
    reg.tip = tip;
    reg.cursor = cur;
    reg.enabled = enabled;
    regions_.push_back(reg);
}

std::string App::regionAt(int x, int y, CursorKind* cursor) const {
    for (int i = (int)regions_.size() - 1; i >= 0; --i) {
        if (regions_[i].rect.Contains((float)x, (float)y)) {
            if (cursor) *cursor = regions_[i].cursor;
            return regions_[i].id;
        }
    }
    if (cursor) *cursor = CursorKind::Arrow;
    return "";
}

Region* App::findRegion(const std::string& id) {
    for (auto& r : regions_) if (r.id == id) return &r;
    return nullptr;
}

void App::updateHoverTooltip() {
    std::string id = regionAt(mouse_.x, mouse_.y, nullptr);
    if (id != hoverId_) {
        hoverId_ = id;
        hoverSince_ = GetTickCount64();
        KillTimer(hwnd_, 2);
        if (!id.empty()) SetTimer(hwnd_, 2, 60, nullptr);
        invalidate();
    }
    updateCursor();
}

void App::updateCursor() {
    if (!mouseInside_) return;
    CursorKind cur = CursorKind::Arrow;
    std::string id = regionAt(mouse_.x, mouse_.y, &cur);
    if (id.empty()) {
        if (spaceDown_ || tool_ == Tool::Hand) cur = CursorKind::SizeAll;
        else if (tool_ == Tool::Text) cur = CursorKind::IBeam;
        else if (tool_ == Tool::Select) cur = CursorKind::Arrow;
        else if (tool_ == Tool::Laser) cur = CursorKind::Hand;
        else if (tool_ == Tool::Background) cur = CursorKind::Arrow;
        else cur = CursorKind::Cross;
    }
    HCURSOR hc = LoadCursorW(nullptr, IDC_ARROW);
    if (cur == CursorKind::Cross) hc = LoadCursorW(nullptr, IDC_CROSS);
    else if (cur == CursorKind::Hand) hc = LoadCursorW(nullptr, IDC_HAND);
    else if (cur == CursorKind::SizeAll) hc = LoadCursorW(nullptr, IDC_SIZEALL);
    else if (cur == CursorKind::IBeam) hc = LoadCursorW(nullptr, IDC_IBEAM);
    SetCursor(hc);
}

void App::resetHover() {
    hoverId_.clear();
    hoverSince_ = 0;
    KillTimer(hwnd_, 2);
}

// ---------------------------------------------------------------------------
// input
// ---------------------------------------------------------------------------

void App::startPan(Pt screen) {
    drag_ = DragMode::Pan;
    panStartMouse_ = screen;
    panStartCenter_ = camCenter_;
    SetCapture(hwnd_);
}

void App::addLiveSample(Pt world) {
    if (drag_ != DragMode::Draw) return;
    Pt c{clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)};
    if (!liveStroke_.points.empty()) {
        Pt last = liveStroke_.points.back();
        if (std::hypot(c.x - last.x, c.y - last.y) <= 0.7f / viewScale()) return;
    }
    Pt prev = liveStroke_.points.empty() ? c : liveStroke_.points.back();
    liveStroke_.points.push_back(c);
    Pt a = worldToScreen(prev);
    Pt b = worldToScreen(c);
    float pad = liveStroke_.width * viewScale() * 0.5f + S(6);
    RECT nr;
    nr.left = std::max(0L, (LONG)std::floor(std::min(a.x, b.x) - pad));
    nr.top = std::max(0L, (LONG)std::floor(std::min(a.y, b.y) - pad));
    nr.right = std::min((LONG)W_, (LONG)std::ceil(std::max(a.x, b.x) + pad));
    nr.bottom = std::min((LONG)H_, (LONG)std::ceil(std::max(a.y, b.y) + pad));
    if (liveDirtyValid_) {
        liveDirty_.left = std::min(liveDirty_.left, nr.left);
        liveDirty_.top = std::min(liveDirty_.top, nr.top);
        liveDirty_.right = std::max(liveDirty_.right, nr.right);
        liveDirty_.bottom = std::max(liveDirty_.bottom, nr.bottom);
    } else {
        liveDirty_ = nr;
        liveDirtyValid_ = true;
    }
    invalidateLiveRect(liveDirty_);
}

void App::startDraw(Pt world) {
    drag_ = DragMode::Draw;
    liveStroke_ = Item{};
    liveStroke_.kind = ItemKind::Stroke;
    liveStroke_.color = (tool_ == Tool::Highlighter) ? highlighterColor_ : penColor_;
    liveStroke_.width = (tool_ == Tool::Highlighter) ? highlighterWidth_ : penWidth_;
    liveStroke_.strokeTool = (tool_ == Tool::Highlighter) ? L"highlighter" : L"pen";
    liveStroke_.points.push_back({clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)});
    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;
    SetCapture(hwnd_);
    SetTimer(hwnd_, 6, 8, nullptr);
    Pt s = worldToScreen(liveStroke_.points[0]);
    float pad = liveStroke_.width * viewScale() * 0.5f + S(6);
    RECT nr;
    nr.left = std::max(0L, (LONG)std::floor(s.x - pad));
    nr.top = std::max(0L, (LONG)std::floor(s.y - pad));
    nr.right = std::min((LONG)W_, (LONG)std::ceil(s.x + pad));
    nr.bottom = std::min((LONG)H_, (LONG)std::ceil(s.y + pad));
    liveDirty_ = nr;
    liveDirtyValid_ = true;
    invalidateLiveRect(nr);
}

void App::startShape(Pt world) {
    drag_ = DragMode::Shape;
    previewShape_ = Item{};
    previewShape_.kind = ItemKind::Shape;
    previewShape_.shape = shapeKind_;
    previewShape_.color = shapeColor_;
    previewShape_.width = shapeWidth_;
    previewShape_.filled = shapeFilled_;
    previewShape_.x1 = previewShape_.x2 = clampf(world.x, 1, PAGE_W - 1);
    previewShape_.y1 = previewShape_.y2 = clampf(world.y, 1, PAGE_H - 1);
    SetCapture(hwnd_);
    invalidateLive();
}

void App::startErase(Pt world) {
    drag_ = DragMode::Erase;
    pendingSnapshot_ = snapshot();
    pendingSnapshotValid_ = true;
    eraserChanged_ = false;
    eraserPath_.clear();
    eraserPath_.push_back(world);
    eraserDrawn_ = 0;
    dragLastWorld_ = world;
    SetCapture(hwnd_);
    SetTimer(hwnd_, 6, 8, nullptr);
}

void App::addEraserSample(Pt world) {
    if (drag_ != DragMode::Erase) return;
    Pt c{clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)};
    if (!eraserPath_.empty()) {
        Pt last = eraserPath_.back();
        float dist = std::hypot(c.x - last.x, c.y - last.y);
        if (dist < 0.4f / viewScale()) return;
        int steps = std::max(1, (int)std::ceil(dist / std::max(2.0f, eraserSize_ * 0.35f)));
        for (int i = 1; i <= steps; ++i) {
            float t = (float)i / steps;
            eraserPath_.push_back({last.x + (c.x - last.x) * t,
                                   last.y + (c.y - last.y) * t});
        }
    } else {
        eraserPath_.push_back(c);
    }
    dragLastWorld_ = c;
}

bool App::eraseGeometry(Pt world) {
    const float tol = eraserSize_ / 2.0f;
    const float tol2 = tol * tol;
    std::vector<Item> out;
    out.reserve(page().items.size() + 4);
    bool changed = false;

    for (const Item& item : page().items) {
        RectF ib = item.bounds();
        if (world.x < ib.X - tol || world.x > ib.GetRight() + tol ||
            world.y < ib.Y - tol || world.y > ib.GetBottom() + tol) {
            out.push_back(item);
            continue;
        }
        if (item.kind == ItemKind::Stroke && item.points.size() > 1) {
            std::vector<Pt> dense;
            if (item.points.size() >= 2) {
                float step = std::max(1.2f, tol * 0.4f);
                dense.push_back(item.points[0]);
                for (size_t si = 1; si < item.points.size(); ++si) {
                    Pt a = item.points[si - 1], b = item.points[si];
                    float d = std::hypot(b.x - a.x, b.y - a.y);
                    int n = std::max(1, std::min(24, (int)std::ceil(d / step)));
                    for (int k = 1; k <= n; ++k) {
                        float t = (float)k / n;
                        dense.push_back({a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t});
                    }
                }
            }
            std::vector<Pt> run;
            std::vector<std::vector<Pt>> pieces;
            bool cut = false;
            for (const Pt& p : dense) {
                float dx = p.x - world.x, dy = p.y - world.y;
                if (dx * dx + dy * dy <= tol2) {
                    cut = true;
                    if (run.size() >= 2) pieces.push_back(run);
                    run.clear();
                } else {
                    run.push_back(p);
                }
            }
            if (!cut) {
                out.push_back(item);
                continue;
            }
            if (run.size() >= 2) pieces.push_back(run);
            changed = true;
            for (auto& piece : pieces) {
                Item ni = item;
                ni.id = nextId_++;
                ni.points = std::move(piece);
                out.push_back(std::move(ni));
            }
            continue;
        }

        if (item.kind == ItemKind::Shape && item.shape == ShapeKind::Line) {
            float ax = item.x1, ay = item.y1, bx = item.x2, by = item.y2;
            float vx = bx - ax, vy = by - ay;
            float len2 = vx * vx + vy * vy;
            if (len2 > 0.0001f) {
                float t = ((world.x - ax) * vx + (world.y - ay) * vy) / len2;
                t = clampf(t, 0.0f, 1.0f);
                float cx = ax + vx * t, cy = ay + vy * t;
                float dx = cx - world.x, dy = cy - world.y;
                if (dx * dx + dy * dy <= tol2) {
                    changed = true;
                    float len = std::sqrt(len2);
                    float half = std::sqrt(std::max(0.0f, tol2 - (dx * dx + dy * dy))) / len;
                    float t1 = std::max(0.0f, t - half);
                    float t2 = std::min(1.0f, t + half);
                    auto makeLine = [&](float u1, float u2) {
                        if ((u2 - u1) * len <= 2.0f) return;
                        Item ni = item;
                        ni.id = nextId_++;
                        ni.shape = ShapeKind::Line;
                        ni.x1 = ax + vx * u1; ni.y1 = ay + vy * u1;
                        ni.x2 = ax + vx * u2; ni.y2 = ay + vy * u2;
                        out.push_back(std::move(ni));
                    };
                    makeLine(0.0f, t1);
                    makeLine(t2, 1.0f);
                    continue;
                }
            }
            out.push_back(item);
            continue;
        }

        if (item.hit(world.x, world.y, tol)) {
            changed = true;
            continue;
        }
        out.push_back(item);
    }

    if (!changed) return false;
    page().items = std::move(out);
    sel_ = -1;
    return true;
}

void App::startLaser(Pt screen) {
    drag_ = DragMode::Laser;
    laserDown_ = true;
    laser_.clear();
    laser_.push_back({screen, GetTickCount64()});
    lastLaserTime_ = GetTickCount64();
    SetTimer(hwnd_, 3, 30, nullptr);
    SetCapture(hwnd_);
    invalidateLive();
}

void App::updateLaser() {
    // Kept for API symmetry; trail upkeep happens in the timer.
    invalidateLive();
}

void App::canvasMouseDown(int x, int y) {
    Pt sp{(float)x, (float)y};
    Pt wp = screenToWorld(sp);
    bool onPage = inPage(sp);

    if (spaceDown_ || tool_ == Tool::Hand) { startPan(sp); return; }
    if (!onPage) {
        if (tool_ == Tool::Select) { sel_ = -1; pointSel_.clear(); invalidate(); }
        return;
    }
    switch (tool_) {
        case Tool::Select: {
            int pHit = hitItemAt(wp, 9.0f);
            if (pHit >= 0 && page().items[pHit].isPoint) {
                bool add = (GetKeyState(VK_CONTROL) & 0x8000) != 0 || (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                if (add) {
                    auto at = std::find(pointSel_.begin(), pointSel_.end(), pHit);
                    if (at == pointSel_.end()) pointSel_.push_back(pHit);
                    else pointSel_.erase(at);
                    sel_ = pHit;
                    invalidate();
                    return;
                }
                pointSel_.clear();
                pointSel_.push_back(pHit);
            } else if (pHit >= 0) {
                pointSel_.clear();
            }
            int h = hitHandle(x, y);
            if (sel_ >= 0 && h >= 0) {
                drag_ = DragMode::Resize;
                moveBase_.reset();
                SetTimer(hwnd_, 6, 8, nullptr);
                dragOrig_ = page().items[sel_];
                dragOrigBounds_ = dragOrig_.bounds();
                resizeCorner_ = h;
                pendingSnapshot_ = snapshot();
                pendingSnapshotValid_ = true;
                gestureUndoPushed_ = false;
                SetCapture(hwnd_);
                return;
            }
            int idx = hitItemAt(wp, 7.0f);
            if (idx >= 0) {
                sel_ = idx;
                drag_ = DragMode::Move;
                moveBase_.reset();
                SetTimer(hwnd_, 6, 8, nullptr);
                dragOrig_ = page().items[idx];
                dragStartWorld_ = wp;
                pendingSnapshot_ = snapshot();
                pendingSnapshotValid_ = true;
                gestureUndoPushed_ = false;
                SetCapture(hwnd_);
                invalidate();
            } else {
                sel_ = -1;
                invalidate();
            }
            break;
        }
        case Tool::Pen:
        case Tool::Highlighter: startDraw(wp); break;
        case Tool::Eraser: startErase(wp); break;
        case Tool::Shape: startShape(wp); break;
        case Tool::Text: beginTextEdit(wp); break;
        case Tool::Laser: startLaser(sp); break;
        case Tool::Background: break;
        case Tool::Hand: startPan(sp); break;
    }
}

void App::onLButtonDown(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (splash_) {
        enterBoard();
        return;
    }
    mouse_ = {x, y};
    mouseInside_ = true;

    if (hNameEdit_) {
        std::string rid0 = regionAt(x, y);
        if (rid0 == "settings:name_save") {
            commitAppName();
            handleRegion(rid0);
            return;
        }
        commitAppName();
    }
    if (hEdit_) {
        std::string editRegion = regionAt(x, y);
        commitText();
        if (editRegion == "text:done") {
            invalidate();
            return;
        }
    }
    SetFocus(hwnd_);

    if (overlay_ == 6) {
        std::string rid = regionAt(x, y);
        if (rid == "devpass:ok" || rid == "devpass:cancel" || rid == "ov:scrim") handleRegion(rid);
        return;
    }
    if (overlay_ == 4) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "fn:") || rid == "ov:scrim") handleRegion(rid);
        return;
    }
    if (overlay_ == 3) {
        if (settingsClosing_ || settingsAnim_ < 0.96f) return;
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "settings:") || rid == "ov:scrim" || rid == "settings:close") {
            handleRegion(rid);
        }
        return;
    }
    if (overlay_ == 1) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "ov:pages:page:")) {
            int idx = std::atoi(rid.c_str() + 14);
            pageDragFrom_ = idx;
            pageDragTo_ = idx;
            pageDragActive_ = false;
            pageDragStart_ = {(float)x, (float)y};
            SetCapture(hwnd_);
            return;
        }
        if (rid == "ov:pages:scroll") {
            pageScrollDrag_ = true;
            pageScrollAnimating_ = false;
            pageScrollDragStartY_ = (float)y;
            pageScrollDragStart_ = pageScroll_;
            pageScrollFrom_ = pageScrollTo_ = pageScroll_;
            SetCapture(hwnd_);
            return;
        }
        if (startsWith(rid, "ov:")) handleRegion(rid);
        return;
    }
    if (overlay_) {
        std::string rid = regionAt(x, y);
        if (startsWith(rid, "ov:")) handleRegion(rid);
        return;
    }
    if (bgPanel_) {
        bgPanel_ = false;
        invalidate();
        return;
    }
    std::string rid = regionAt(x, y);
    if (rid == "dock:grip") {
        startDockDrag({(float)x, (float)y});
        return;
    }
    if (!rid.empty()) {
        handleRegion(rid);
        return;
    }
    canvasMouseDown(x, y);
}

void App::onLButtonUp(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (overlay_ == 1 && pageDragFrom_ >= 0) {
        int from = pageDragFrom_;
        int to = pageDragTo_;
        bool active = pageDragActive_;
        pageDragFrom_ = pageDragTo_ = -1;
        pageDragActive_ = false;
        if (GetCapture() == hwnd_) ReleaseCapture();
        if (active && to >= 0 && to != from) movePage(from, to);
        else if (!active && from >= 0) {
            switchPage(from);
            overlay_ = 0;
        }
        invalidate();
        return;
    }
    if (overlay_ == 1 && pageScrollDrag_) {
        pageScrollDrag_ = false;
        if (GetCapture() == hwnd_) ReleaseCapture();
        invalidate();
        return;
    }
    if (drag_ == DragMode::None) return;
    finishDrag({(float)x, (float)y});
}

void App::finishDrag(Pt screen) {
    Pt world = screenToWorld(screen);
    if (drag_ == DragMode::Draw) {
        if (!liveStroke_.points.empty()) {
            pushUndo();
            liveStroke_.id = nextId_++;
            page().items.push_back(liveStroke_);
            page().touch();
            markDirty();
            sel_ = -1;
        }
        liveStroke_ = Item{};
    } else if (drag_ == DragMode::Shape) {
        float w = std::fabs(previewShape_.x2 - previewShape_.x1);
        float h = std::fabs(previewShape_.y2 - previewShape_.y1);
        bool line = previewShape_.shape == ShapeKind::Line || previewShape_.shape == ShapeKind::Arrow;
        bool big = line ? (std::hypot(w, h) > 3.0f / viewScale()) : (w > 3.0f / viewScale() || h > 3.0f / viewScale());
        if (big) {
            pushUndo();
            previewShape_.id = nextId_++;
            page().items.push_back(previewShape_);
            page().touch();
            markDirty();
            sel_ = -1;
        }
        previewShape_ = Item{};
    } else if (drag_ == DragMode::Erase) {
        KillTimer(hwnd_, 6);
        bool changed = false;
        for (const Pt& p : eraserPath_) {
            if (eraseGeometry(p)) changed = true;
        }
        if (changed) {
            if (pendingSnapshotValid_) pushUndoSnapshot(pendingSnapshot_);
            page().touch();
            markDirty();
            eraserChanged_ = true;
        }
        eraserPath_.clear();
        eraserDrawn_ = 0;
        pendingSnapshotValid_ = false;
    } else if (drag_ == DragMode::Move || drag_ == DragMode::Resize) {
        if (gestureUndoPushed_) {
            page().touch();
            markDirty();
        }
        pendingSnapshotValid_ = false;
    } else if (drag_ == DragMode::Laser) {
        laserDown_ = false;
        lastLaserTime_ = GetTickCount64();
    } else if (drag_ == DragMode::Dock) {
        saveConfig();
    }
    drag_ = DragMode::None;
    KillTimer(hwnd_, 6);
    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;
    moveBase_.reset();
    gestureUndoPushed_ = false;
    resizeCorner_ = -1;
    if (GetCapture() == hwnd_) ReleaseCapture();
    invalidate();
}

void App::onMouseMove(int x, int y, WPARAM) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    mouse_ = {x, y};
    mouseInside_ = true;
    if (!trackingMouse_) {
        TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, hwnd_, 0};
        TrackMouseEvent(&tme);
        trackingMouse_ = true;
    }
    Pt sp{(float)x, (float)y};
    Pt wp = screenToWorld(sp);

    if (overlay_ == 1 && pageDragFrom_ >= 0) {
        if (!pageDragActive_ &&
            std::hypot(sp.x - pageDragStart_.x, sp.y - pageDragStart_.y) > S(5)) {
            pageDragActive_ = true;
        }
        if (pageDragActive_) {
            std::string rid = regionAt(x, y);
            if (startsWith(rid, "ov:pages:page:")) {
                pageDragTo_ = std::atoi(rid.c_str() + 14);
            } else {
                pageDragTo_ = -1;
            }
            invalidate();
        }
        return;
    }
    if (overlay_ == 1 && pageScrollDrag_) {
        float denom = std::max(S(1), pageScrollTrackH_ - pageScrollThumbH_);
        pageScroll_ = pageScrollDragStart_ +
                      (sp.y - pageScrollDragStartY_) * pageScrollMax_ / denom;
        pageScroll_ = std::max(0.0f, std::min(pageScroll_, pageScrollMax_));
        pageScrollFrom_ = pageScrollTo_ = pageScroll_;
        invalidate();
        return;
    }

    if (drag_ == DragMode::Dock) {
        float cx = dockDragStartCenter_.x + (sp.x - dockDragStart_.x);
        float cy = dockDragStartCenter_.y + (sp.y - dockDragStart_.y);
        dockDefault_ = false;
        dockNormX_ = clampf(cx / std::max(1.0f, (float)W_), 0.0f, 1.0f);
        dockNormY_ = clampf(cy / std::max(1.0f, (float)H_), 0.0f, 1.0f);
        updateCursor();
        invalidate();
        return;
    }

    if (drag_ != DragMode::None) {
        updateCursor();
        if (drag_ == DragMode::Draw) {
            addLiveSample(wp);
        } else if (drag_ == DragMode::Shape) {
            previewShape_.x2 = clampf(wp.x, 1, PAGE_W - 1);
            previewShape_.y2 = clampf(wp.y, 1, PAGE_H - 1);
            if (GetKeyState(VK_SHIFT) & 0x8000) {
                float dx = previewShape_.x2 - previewShape_.x1;
                float dy = previewShape_.y2 - previewShape_.y1;
                if (shapeKind_ == ShapeKind::Line || shapeKind_ == ShapeKind::Arrow) {
                    float len = std::hypot(dx, dy);
                    float ang = std::round(std::atan2(dy, dx) / (float)(M_PI / 4.0)) * (float)(M_PI / 4.0);
                    previewShape_.x2 = previewShape_.x1 + std::cos(ang) * len;
                    previewShape_.y2 = previewShape_.y1 + std::sin(ang) * len;
                } else {
                    float m = std::max(std::fabs(dx), std::fabs(dy));
                    previewShape_.x2 = previewShape_.x1 + (dx < 0 ? -m : m);
                    previewShape_.y2 = previewShape_.y1 + (dy < 0 ? -m : m);
                }
            }
            invalidateLive();
        } else if (drag_ == DragMode::Erase) {
            addEraserSample(wp);
        } else if (drag_ == DragMode::Move) {
            float dx = wp.x - dragStartWorld_.x;
            float dy = wp.y - dragStartWorld_.y;
            RectF ob = dragOrig_.bounds();
            dx = clampf(dx, -ob.X + 4, PAGE_W - ob.GetRight() - 4);
            dy = clampf(dy, -ob.Y + 4, PAGE_H - ob.GetBottom() - 4);
            if (!gestureUndoPushed_ && (std::fabs(dx) > 0.4f || std::fabs(dy) > 0.4f)) {
                pushUndoSnapshot(pendingSnapshot_);
                gestureUndoPushed_ = true;
            }
            if (gestureUndoPushed_) {
                page().items[sel_] = dragOrig_.shifted(dx, dy);
                page().touch();
                markDirty();
                invalidate();
            }
        } else if (drag_ == DragMode::Resize) {
            if (sel_ < 0) { drag_ = DragMode::None; return; }
            float ax = 0, ay = 0;
            if (resizeCorner_ == 0) { ax = dragOrigBounds_.GetRight(); ay = dragOrigBounds_.GetBottom(); }
            else if (resizeCorner_ == 1) { ax = dragOrigBounds_.X; ay = dragOrigBounds_.GetBottom(); }
            else if (resizeCorner_ == 2) { ax = dragOrigBounds_.GetRight(); ay = dragOrigBounds_.Y; }
            else { ax = dragOrigBounds_.X; ay = dragOrigBounds_.Y; }
            float ow = std::max(1.0f, dragOrigBounds_.Width);
            float oh = std::max(1.0f, dragOrigBounds_.Height);
            float sx = 1, sy = 1;
            if (resizeCorner_ == 0) { sx = (ax - wp.x) / ow; sy = (ay - wp.y) / oh; }
            else if (resizeCorner_ == 1) { sx = (wp.x - ax) / ow; sy = (ay - wp.y) / oh; }
            else if (resizeCorner_ == 2) { sx = (ax - wp.x) / ow; sy = (wp.y - ay) / oh; }
            else { sx = (wp.x - ax) / ow; sy = (wp.y - ay) / oh; }
            sx = clampf(sx, 0.04f, 20.0f);
            sy = clampf(sy, 0.04f, 20.0f);
            if (!gestureUndoPushed_ && (std::fabs(sx - 1) > 0.005f || std::fabs(sy - 1) > 0.005f)) {
                pushUndoSnapshot(pendingSnapshot_);
                gestureUndoPushed_ = true;
            }
            if (gestureUndoPushed_) {
                Item it = dragOrig_;
                if (it.kind == ItemKind::Stroke) {
                    for (auto& p : it.points) {
                        p.x = ax + (p.x - ax) * sx;
                        p.y = ay + (p.y - ay) * sy;
                    }
                    it.width = std::max(1.0f, it.width * std::pow((sx + sy) / 2, 0.25f));
                    if (it.functionGraph) {
                        it.originX = ax + (it.originX - ax) * sx;
                        it.originY = ay + (it.originY - ay) * sy;
                        it.gLeft = ax + (it.gLeft - ax) * sx;
                        it.gRight = ax + (it.gRight - ax) * sx;
                        it.gTop = ay + (it.gTop - ay) * sy;
                        it.gBottom = ay + (it.gBottom - ay) * sy;
                        it.unitX *= std::fabs(sx);
                        it.unitY *= std::fabs(sy);
                        fitFunctionToPage(it);
                    }
                } else if (it.kind == ItemKind::Shape) {
                    it.x1 = ax + (it.x1 - ax) * sx;
                    it.x2 = ax + (it.x2 - ax) * sx;
                    it.y1 = ay + (it.y1 - ay) * sy;
                    it.y2 = ay + (it.y2 - ay) * sy;
                } else if (it.kind == ItemKind::Text) {
                    float u = clampf((sx + sy) / 2, 0.1f, 20.0f);
                    it.x = ax + (it.x - ax) * sx;
                    it.y = ay + (it.y - ay) * sy;
                    it.fontSize = std::max(8.0f, it.fontSize * u);
                    it.textWidth = std::max(40.0f, it.textWidth * sx);
                } else if (it.kind == ItemKind::Image) {
                    it.x = ax + (it.x - ax) * sx;
                    it.y = ay + (it.y - ay) * sy;
                    it.w = std::max(20.0f, it.w * sx);
                    it.h = std::max(20.0f, it.h * sy);
                }
                page().items[sel_] = it;
                page().touch();
                markDirty();
                invalidate();
            }
        } else if (drag_ == DragMode::Pan) {
            float s = viewScale();
            camCenter_.x = panStartCenter_.x - (sp.x - panStartMouse_.x) / s;
            camCenter_.y = panStartCenter_.y - (sp.y - panStartMouse_.y) / s;
            clampCam();
            invalidate();
        } else if (drag_ == DragMode::Laser) {
            if (laser_.empty() || std::hypot(sp.x - laser_.back().p.x, sp.y - laser_.back().p.y) > 2.0f) {
                laser_.push_back({sp, GetTickCount64()});
                lastLaserTime_ = GetTickCount64();
            }
            invalidateLive();
        }
        return;
    }

    updateHoverTooltip();
    if (tool_ == Tool::Eraser) invalidateLive();
}

void App::onMouseLeave() {
    trackingMouse_ = false;
    mouseInside_ = false;
    resetHover();
    invalidate();
}

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

void App::onLButtonDblClk(int x, int y) {
    if (!handlingPointer_ && (pointerActive_ || GetTickCount64() < suppressMouseUntil_)) return;
    if (overlay_ == 3) {
        std::string rid = regionAt(x, y);
        if (rid == "settings:version") handleRegion(rid);
        return;
    }
    if (overlay_ || bgPanel_) return;
    if (tool_ != Tool::Select && tool_ != Tool::Text) return;
    Pt wp = screenToWorld({(float)x, (float)y});
    if (!inPage({(float)x, (float)y})) return;
    int idx = hitItemAt(wp, 7);
    if (idx >= 0 && page().items[idx].kind == ItemKind::Text) {
        sel_ = idx;
        beginTextEdit(wp, idx);
    }
}

void App::onRButtonUp(int x, int y) {
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

void App::onMouseWheel(int x, int y, int delta, bool ctrl, bool shift) {
    if (overlay_ == 1) {
        float maxS = std::max(0.0f, pageScrollMax_);
        float base = pageScrollAnimating_ ? pageScrollTo_ : pageScroll_;
        pageScrollFrom_ = pageScroll_;
        pageScrollTo_ = std::max(0.0f, std::min(base - (float)delta / 120.0f * S(110), maxS));
        pageScrollAnimStart_ = GetTickCount64();
        pageScrollAnimating_ = true;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
        return;
    }
    if (overlay_ || splash_) return;
    if (dockRect().Contains((float)x, (float)y)) return;
    Pt sp{(float)x, (float)y};
    if (ctrl) {
        camScrollAnimating_ = false;
        float mul = delta > 0 ? 1.12f : 1.0f / 1.12f;
        zoomAt(sp, zoom_ * mul);
    } else {
        float s = viewScale();
        Pt target = camScrollAnimating_ ? camScrollTo_ : camCenter_;
        if (shift) target.x -= (float)delta / 120.0f * 80.0f / s;
        else target.y -= (float)delta / 120.0f * 80.0f / s;
        Pt saved = camCenter_;
        camCenter_ = target;
        clampCam();
        target = camCenter_;
        camCenter_ = saved;
        camScrollFrom_ = camCenter_;
        camScrollTo_ = target;
        camScrollStart_ = GetTickCount64();
        camScrollAnimating_ = true;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
    }
}

void App::verifyDevPassword() {
    if (devPass_ == "HAVEGETNEVERDIE") {
        devMode_ = true;
        devUnlocked_ = true;
        saveConfig();
        overlay_ = 3;
        devPass_.clear();
        devPassError_.clear();
        showToast(L"开发者选项已解锁", false, 1600);
        invalidate();
    } else {
        devPassError_ = L"密码错误，请重试";
        devPass_.clear();
        invalidate();
    }
}

void App::openFunctionDialog() {
    fnExpr_.clear();
    fnError_.clear();
    fnIgnoreNextChar_ = false;
    fnPointMode_ = false;
    editingFunction_ = -1;
    bgPanel_ = false;
    overlay_ = 4;
    invalidate();
}

void App::fitFunctionToPage(Item& item) {
    if (!item.functionGraph) return;
    const float margin = 24.0f;
    for (int pass = 0; pass < 3; ++pass) {
        RectF b = item.bounds();
        if (b.Width <= 0 || b.Height <= 0) return;
        float sx = (PAGE_W - margin * 2) / b.Width;
        float sy = (PAGE_H - margin * 2) / b.Height;
        float sc = std::min(1.0f, std::min(sx, sy));
        if (sc < 0.999f) {
            Pt c{b.X + b.Width / 2, b.Y + b.Height / 2};
            for (auto& p : item.points) {
                p.x = c.x + (p.x - c.x) * sc;
                p.y = c.y + (p.y - c.y) * sc;
            }
            item.originX = c.x + (item.originX - c.x) * sc;
            item.originY = c.y + (item.originY - c.y) * sc;
            item.gLeft = c.x + (item.gLeft - c.x) * sc;
            item.gRight = c.x + (item.gRight - c.x) * sc;
            item.gTop = c.y + (item.gTop - c.y) * sc;
            item.gBottom = c.y + (item.gBottom - c.y) * sc;
            item.unitX *= sc;
            item.unitY *= sc;
            continue;
        }
        b = item.bounds();
        float dx = 0, dy = 0;
        if (b.X < margin) dx = margin - b.X;
        else if (b.GetRight() > PAGE_W - margin) dx = PAGE_W - margin - b.GetRight();
        if (b.Y < margin) dy = margin - b.Y;
        else if (b.GetBottom() > PAGE_H - margin) dy = PAGE_H - margin - b.GetBottom();
        if (std::fabs(dx) > 0.01f || std::fabs(dy) > 0.01f) {
            for (auto& p : item.points) { p.x += dx; p.y += dy; }
            item.originX += dx; item.originY += dy;
            item.gLeft += dx; item.gRight += dx; item.gTop += dy; item.gBottom += dy;
        }
        break;
    }
}

void App::scaleSelectedFunction(float factor, Pt screenAnchor) {
    if (sel_ < 0 || sel_ >= (int)page().items.size()) return;
    Item& it = page().items[sel_];
    if (!it.functionGraph) return;
    if (!gestureUndoPushed_) {
        pushUndo();
        gestureUndoPushed_ = true;
    }
    Pt w = screenToWorld(screenAnchor);
    factor = clampf(factor, 0.6f, 1.7f);
    for (auto& p : it.points) {
        p.x = w.x + (p.x - w.x) * factor;
        p.y = w.y + (p.y - w.y) * factor;
    }
    it.originX = w.x + (it.originX - w.x) * factor;
    it.originY = w.y + (it.originY - w.y) * factor;
    it.gLeft = w.x + (it.gLeft - w.x) * factor;
    it.gRight = w.x + (it.gRight - w.x) * factor;
    it.gTop = w.y + (it.gTop - w.y) * factor;
    it.gBottom = w.y + (it.gBottom - w.y) * factor;
    it.unitX *= factor;
    it.unitY *= factor;
    fitFunctionToPage(it);
    page().touch();
    markDirty();
    invalidate();
}

bool App::onGesture(LPARAM lp) {
    GESTUREINFO gi{};
    gi.cbSize = sizeof(gi);
    if (!GetGestureInfo((HGESTUREINFO)lp, &gi)) return false;
    bool handled = false;
    if (gi.dwID == GID_ZOOM) {
        handled = true;
        if (gi.dwFlags & GF_BEGIN) {
            gestureLastZoom_ = (double)gi.ullArguments;
            gestureUndoPushed_ = false;
        } else if (gestureLastZoom_ > 0 && gi.ullArguments > 0) {
            double factor = (double)gi.ullArguments / gestureLastZoom_;
            gestureLastZoom_ = (double)gi.ullArguments;
            POINT pt{gi.ptsLocation.x, gi.ptsLocation.y};
            ScreenToClient(hwnd_, &pt);
            scaleSelectedFunction((float)factor, {(float)pt.x, (float)pt.y});
        }
    }
    CloseGestureInfoHandle((HGESTUREINFO)lp);
    return handled;
}

void App::submitFunctionDialog() {
    if (fnPointMode_) insertPointFromInput();
    else insertFunctionGraph();
}

void App::insertPointFromInput() {
    std::string t = fnExpr_;
    for (char& c : t) {
        if (c == '(' || c == ')' || c == '[' || c == ']' || c == ' ') c = ' ';
    }
    size_t comma = t.find(',');
    if (comma == std::string::npos) comma = t.find(';');
    if (comma == std::string::npos) {
        fnError_ = L"请输入两个坐标，例如 (2,3)";
        invalidate();
        return;
    }
    try {
        double gx = std::stod(t.substr(0, comma));
        double gy = std::stod(t.substr(comma + 1));
        Pt world;
        if (fnPointGraph_ >= 0 && fnPointGraph_ < (int)page().items.size() &&
            page().items[fnPointGraph_].functionGraph) {
            Item& f = page().items[fnPointGraph_];
            world = {f.originX + (float)(gx * f.unitX), f.originY - (float)(gy * f.unitY)};
        } else {
            Pt c = screenToWorld(viewCenter());
            world = {c.x + (float)(gx * 40), c.y - (float)(gy * 40)};
        }
        Item p;
        p.kind = ItemKind::Shape;
        p.shape = ShapeKind::Ellipse;
        p.filled = true;
        p.width = 2.0f;
        p.color = 0xC0392B;
        p.x1 = world.x - 5.5f; p.y1 = world.y - 5.5f;
        p.x2 = world.x + 5.5f; p.y2 = world.y + 5.5f;
        p.isPoint = true;
        char buf[80];
        snprintf(buf, sizeof(buf), "(%.4g, %.4g)", gx, gy);
        p.pointText = buf;
        p.id = nextId_++;
        pushUndo();
        page().items.push_back(std::move(p));
        int idx = (int)page().items.size() - 1;
        setTool(Tool::Select);
        sel_ = idx;
        pointSel_.clear();
        pointSel_.push_back(idx);
        overlay_ = 0;
        page().touch();
        markDirty();
        invalidate();
    } catch (...) {
        fnError_ = L"坐标格式不正确，例如 (2,3)";
        invalidate();
    }
}

void App::connectSelectedPoints(int mode, int direction) {
    if (pointSel_.size() != 2) return;
    auto center = [&](int idx) {
        RectF b = page().items[idx].bounds();
        return Pt{b.X + b.Width / 2, b.Y + b.Height / 2};
    };
    int i1 = pointSel_[0], i2 = pointSel_[1];
    if (i1 < 0 || i2 < 0 || i1 >= (int)page().items.size() || i2 >= (int)page().items.size()) return;
    Pt a = center(i1), b = center(i2);
    Pt p1 = a, p2 = b;
    if (mode != 2) {
        float dx = b.x - a.x, dy = b.y - a.y;
        if (std::fabs(dx) + std::fabs(dy) < 1e-4f) return;
        float tmin = -1e9f, tmax = 1e9f;
        auto clipAxis = [&](float p0, float d, float lo, float hi) {
            if (std::fabs(d) < 1e-6f) return p0 >= lo && p0 <= hi;
            float t1 = (lo - p0) / d, t2 = (hi - p0) / d;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            return tmin <= tmax;
        };
        if (!clipAxis(a.x, dx, 0, PAGE_W) || !clipAxis(a.y, dy, 0, PAGE_H)) return;
        if (mode == 0) {
            p1 = {a.x + dx * tmin, a.y + dy * tmin};
            p2 = {a.x + dx * tmax, a.y + dy * tmax};
        } else {
            Pt start = direction == 0 ? a : b;
            Pt dir = direction == 0 ? Pt{dx, dy} : Pt{-dx, -dy};
            float tEnd = 0;
            if (std::fabs(dir.x) > 1e-6f) {
                float tx = dir.x > 0 ? (PAGE_W - start.x) / dir.x : (0 - start.x) / dir.x;
                tEnd = std::max(tEnd, tx);
            }
            if (std::fabs(dir.y) > 1e-6f) {
                float ty = dir.y > 0 ? (PAGE_H - start.y) / dir.y : (0 - start.y) / dir.y;
                tEnd = std::max(tEnd, ty);
            }
            p1 = start;
            p2 = {start.x + dir.x * tEnd, start.y + dir.y * tEnd};
        }
    }
    Item line;
    line.kind = ItemKind::Shape;
    line.shape = ShapeKind::Line;
    line.color = 0x3B4555;
    line.width = 2.0f;
    line.x1 = p1.x; line.y1 = p1.y; line.x2 = p2.x; line.y2 = p2.y;
    line.id = nextId_++;
    pushUndo();
    page().items.push_back(std::move(line));
    int idx = (int)page().items.size() - 1;
    setTool(Tool::Select);
    sel_ = idx;
    pointSel_.clear();
    page().touch();
    markDirty();
    invalidate();
}

void App::insertFunctionGraph() {
    std::vector<std::pair<double, double>> samples;
    std::string error;
    if (!sampleExpression(fnExpr_, -5.0, 5.0, 700, samples, error)) {
        fnError_ = wide(error);
        invalidate();
        return;
    }
    double xmin = samples.front().first, xmax = samples.back().first;
    double ymin = samples.front().second, ymax = samples.front().second;
    for (auto& p : samples) {
        ymin = std::min(ymin, p.second);
        ymax = std::max(ymax, p.second);
    }
    double xspan = std::max(1.0e-6, xmax - xmin);
    double yspan = std::max(1.0e-6, ymax - ymin);
    float boxW = std::min(620.0f, PAGE_W * 0.42f);
    float boxH = std::min(420.0f, PAGE_H * 0.47f);
    float sx = boxW / (float)xspan;
    float sy = boxH / (float)yspan;
    if (yspan < 1.0e-5) sy = sx;
    sy = clampf(sy, sx * 0.10f, sx * 3.0f);
    Pt center = screenToWorld(viewCenter());
    double mx = (xmin + xmax) * 0.5, my = (ymin + ymax) * 0.5;

    Item item;
    item.kind = ItemKind::Stroke;
    item.id = nextId_++;
    item.color = 0x2457D6;
    item.width = 3.0f;
    item.strokeTool = L"pen";
    item.functionGraph = true;
    item.showAxes = fnShowAxes_;
    item.funcExpr = fnExpr_;
    item.unitX = sx;
    item.unitY = sy;
    item.originX = center.x + (float)((0.0 - mx) * sx);
    item.originY = center.y - (float)((0.0 - my) * sy);
    item.gLeft = center.x - boxW * 0.5f;
    item.gRight = center.x + boxW * 0.5f;
    item.gTop = center.y - boxH * 0.5f;
    item.gBottom = center.y + boxH * 0.5f;
    if (fnShowAxes_) {
        item.gLeft = std::min(item.gLeft, item.originX - sx * 1.2f);
        item.gRight = std::max(item.gRight, item.originX + sx * 1.2f);
        item.gTop = std::min(item.gTop, item.originY - sy * 1.2f);
        item.gBottom = std::max(item.gBottom, item.originY + sy * 1.2f);
    }
    item.points.reserve(samples.size());
    for (auto& p : samples) {
        float x = center.x + (float)((p.first - mx) * sx);
        float y = center.y - (float)((p.second - my) * sy);
        item.points.push_back({x, y});
    }
    if (item.points.size() < 2) {
        fnError_ = L"函数点太少，无法绘制";
        invalidate();
        return;
    }
    fitFunctionToPage(item);
    int index = -1;
    if (editingFunction_ >= 0 && editingFunction_ < (int)page().items.size()) {
        pushUndo();
        index = editingFunction_;
        page().items[index] = std::move(item);
        editingFunction_ = -1;
    } else {
        pushUndo();
        page().items.push_back(std::move(item));
        index = (int)page().items.size() - 1;
    }
    setTool(Tool::Select);
    sel_ = index;
    pointSel_.clear();
    overlay_ = 0;
    page().touch();
    markDirty();
    showToast(L"已插入函数 y = " + wide(fnExpr_), false, 1800);
    invalidate();
}

bool App::onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt) {
    if (splash_) { enterBoard(); return true; }
    if (overlay_ == 6) {
        if (vk == VK_ESCAPE) { overlay_ = 3; devPass_.clear(); devPassError_.clear(); invalidate(); return true; }
        if (vk == VK_RETURN) { verifyDevPassword(); return true; }
        if (vk == VK_BACK) {
            if (!devPass_.empty()) { devPass_.pop_back(); devPassError_.clear(); invalidate(); }
            return true;
        }
        return true;
    }
    if (overlay_ == 4) {
        if (vk == VK_ESCAPE) { overlay_ = 0; invalidate(); return true; }
        if (vk == VK_RETURN) { submitFunctionDialog(); return true; }
        if (vk == VK_BACK) {
            if (!fnExpr_.empty()) {
                fnExpr_.pop_back();
                fnError_.clear();
                invalidate();
            }
            return true;
        }
        return true;
    }
    if (hEdit_) return false;
    if (!ctrl && vk == 'F') { openFunctionDialog(); fnIgnoreNextChar_ = true; return true; }
    if (ctrl) {
        if (vk == 'Z') { if (shift) redo(); else undo(); return true; }
        if (vk == 'Y') { redo(); return true; }
        if (vk == 'S') { saveDocument(shift); return true; }
        if (vk == 'O') { openDocumentDialog(); return true; }
        if (vk == VK_OEM_COMMA) { commitText(); overlay_ = 3; bgPanel_ = false; invalidate(); return true; }
        if (vk == 'E') { exportPng(); return true; }
        if (vk == 'D') { duplicateSelection(); return true; }
        return false;
    }
    if (vk == VK_SPACE) {
        if (!spaceDown_) {
            spaceDown_ = true;
            updateCursor();
        }
        return true;
    }
    if (vk == VK_F11) { toggleFullscreen(); return true; }
    if (vk == VK_F1) { overlay_ = overlay_ == 2 ? 0 : 2; bgPanel_ = false; invalidate(); return true; }
    if (vk == VK_ESCAPE) {
        if (overlay_) {
            if (overlay_ == 3) {
                commitAppName();
                settingsClosing_ = true;
                settingsAnimStart_ = GetTickCount64();
                SetTimer(hwnd_, 5, 16, nullptr);
            } else {
                overlay_ = 0;
            }
        }
        else if (bgPanel_) bgPanel_ = false;
        else if (fullscreen_) toggleFullscreen();
        else if (sel_ >= 0) sel_ = -1;
        else { tool_ = Tool::Select; }
        invalidate();
        return true;
    }
    if (vk == VK_DELETE || vk == VK_BACK) { deleteSelection(); return true; }
    if (vk == VK_PRIOR) { if (current_ > 0) switchPage(current_ - 1); return true; }
    if (vk == VK_NEXT) { if (current_ + 1 < pageCount()) switchPage(current_ + 1); return true; }
    if (vk == '0') { fitView(); return true; }
    if (vk == VK_OEM_PLUS || vk == VK_ADD) { zoomStep(1.18f, viewCenter()); return true; }
    if (vk == VK_OEM_MINUS || vk == VK_SUBTRACT) { zoomStep(1 / 1.18f, viewCenter()); return true; }
    if (vk == 'V') { setTool(Tool::Select); return true; }
    if (vk == 'P') { setTool(Tool::Pen); return true; }
    if (vk == 'H') { setTool(Tool::Highlighter); return true; }
    if (vk == 'E') { setTool(Tool::Eraser); return true; }
    if (vk == 'R') { setTool(Tool::Shape); return true; }
    if (vk == 'T') { setTool(Tool::Text); return true; }
    if (vk == 'L') { setTool(Tool::Laser); return true; }
    if (vk == 'B') { setTool(Tool::Background); return true; }
    if (vk == 'I') { insertImageDialog(); return true; }
    return false;
}

void App::onKeyUp(WPARAM vk) {
    if (vk == VK_SPACE) {
        spaceDown_ = false;
        updateCursor();
    }
}

bool App::onChar(wchar_t ch) {
    if (overlay_ == 6) {
        if (ch >= 32 && ch < 127 && devPass_.size() < 64) {
            devPass_.push_back((char)ch);
            devPassError_.clear();
            invalidate();
            return true;
        }
        return false;
    }
    if (overlay_ != 4) return false;
    if (fnIgnoreNextChar_) { fnIgnoreNextChar_ = false; return true; }
    std::string add;
    if (ch >= 32 && ch < 127) add.push_back((char)ch);
    else if (ch == 0x00B2) add = "^2";
    else if (ch == 0x00B3) add = "^3";
    else if (ch == 0x00D7) add = "*";
    else if (ch == 0x00F7) add = "/";
    else if (ch == 0x2212) add = "-";
    else if (ch == 0x03C0) add = "pi";
    if (!add.empty() && fnExpr_.size() + add.size() < 120) {
        fnExpr_ += add;
        fnError_.clear();
        invalidate();
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// timer / fullscreen / close
// ---------------------------------------------------------------------------

void App::onTimer(UINT_PTR id) {
    if (id == 1) {
        KillTimer(hwnd_, 1);
        toast_.clear();
        invalidate();
    } else if (id == 2) {
        KillTimer(hwnd_, 2);
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
    } else if (id == 3) {
        ULONGLONG now = GetTickCount64();
        while (!laser_.empty() && now - laser_.front().t > 850) laser_.erase(laser_.begin());
        if (laser_.empty() && !laserDown_) {
            KillTimer(hwnd_, 3);
        }
        invalidateLive();
    } else if (id == 4) {
        ULONGLONG now = GetTickCount64();
        if (splash_) {
            if (splashLeaving_) {
                if (now - splashLeaveStart_ >= 240) {
                    splash_ = false;
                    splashLeaving_ = false;
                    KillTimer(hwnd_, 4);
                    fitView();
                    SetFocus(hwnd_);
                    boardIntroStart_ = now;
                    showToast(L"欢迎使用 " + appName_, false, 2000);
                    SetTimer(hwnd_, 5, 16, nullptr);
                    invalidate();
                } else {
                    invalidate();
                }
            } else if (now - splashStart_ > 2400) {
                enterBoard();
            } else {
                invalidate();
            }
        } else {
            KillTimer(hwnd_, 4);
        }
    } else if (id == 6) {
        bool draw = drag_ == DragMode::Draw;
        bool erase = drag_ == DragMode::Erase;
        bool moving = drag_ == DragMode::Move || drag_ == DragMode::Resize;
        if ((draw || erase || moving) && hwnd_ && IsWindowVisible(hwnd_)) {
            POINT pt{};
            bool got = false;
            if (pointerActive_) {
                POINTER_INFO pi{};
                if (GetPointerInfo(activePointerId_, &pi)) {
                    pt.x = pi.ptPixelLocation.x;
                    pt.y = pi.ptPixelLocation.y;
                    got = true;
                }
            } else if (GetCursorPos(&pt)) {
                got = true;
            }
            if (got && (draw || erase)) {
                ScreenToClient(hwnd_, &pt);
                Pt w = screenToWorld({(float)pt.x, (float)pt.y});
                if (draw) addLiveSample(w);
                else addEraserSample(w);
            }
            UpdateWindow(hwnd_);
        } else {
            KillTimer(hwnd_, 6);
        }
    } else if (id == 5) {
        ULONGLONG now = GetTickCount64();
        bool had = false;
        if (overlay_ == 3 && settingsAnimStart_ != 0) {
            had = true;
            float t = std::min(1.0f, (float)(now - settingsAnimStart_) / 180.0f);
            float e = easeOutCubic(t);
            settingsAnim_ = settingsClosing_ ? (1.0f - e) : e;
            if (t >= 1.0f) {
                settingsAnimStart_ = 0;
                if (settingsClosing_) {
                    settingsClosing_ = false;
                    overlay_ = 0;
                }
                settingsAnim_ = settingsClosing_ ? 0.0f : 1.0f;
            }
        }
        if (pageScrollAnimating_) {
            float t = std::min(1.0f, (float)(now - pageScrollAnimStart_) / 240.0f);
            float e = easeOutCubic(t);
            pageScroll_ = pageScrollFrom_ + (pageScrollTo_ - pageScrollFrom_) * e;
            had = true;
            if (t >= 1.0f) {
                pageScroll_ = pageScrollTo_;
                pageScrollAnimating_ = false;
            }
        }
        if (camScrollAnimating_) {
            float t = std::min(1.0f, (float)(now - camScrollStart_) / 200.0f);
            float e = easeOutCubic(t);
            camCenter_.x = camScrollFrom_.x + (camScrollTo_.x - camScrollFrom_.x) * e;
            camCenter_.y = camScrollFrom_.y + (camScrollTo_.y - camScrollFrom_.y) * e;
            had = true;
            if (t >= 1.0f) {
                camCenter_ = camScrollTo_;
                camScrollAnimating_ = false;
            }
        }
        if (pageFadeStart_ != 0) {
            if (now - pageFadeStart_ < 220) had = true;
            else pageFadeStart_ = 0;
        }
        if (boardIntroStart_ != 0) {
            if (now - boardIntroStart_ < 260) had = true;
            else boardIntroStart_ = 0;
        }
        {
            ULONGLONG elapsed = now - hoverSince_;
            bool tip = !hoverId_.empty() && mouseInside_ && elapsed >= 45 && elapsed < 190;
            if (tip) had = true;
        }
        if (toolAnimStart_ != 0) {
            had = true;
            float t = std::min(1.0f, (float)(now - toolAnimStart_) / 140.0f);
            toolAnim_ = easeOutCubic(t);
            if (t >= 1.0f) {
                toolAnim_ = 1.0f;
                toolAnimStart_ = 0;
            }
        }
        if (toastAnimStart_ != 0 && now - toastAnimStart_ < 180) had = true;
        if (had) invalidate();
        else KillTimer(hwnd_, 5);
    }
}

void App::toggleFullscreen() {
    if (!fullscreen_) {
        savedPlacement_.length = sizeof(savedPlacement_);
        GetWindowPlacement(hwnd_, &savedPlacement_);
        LONG_PTR style = GetWindowLongPtrW(hwnd_, GWL_STYLE);
        SetWindowLongPtrW(hwnd_, GWL_STYLE, (style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP | WS_VISIBLE);
        HMONITOR mon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(mon, &mi);
        SetWindowPos(hwnd_, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED);
        fullscreen_ = true;
    } else {
        LONG_PTR style = GetWindowLongPtrW(hwnd_, GWL_STYLE);
        style &= ~(WS_POPUP);
        style |= WS_OVERLAPPEDWINDOW;
        SetWindowLongPtrW(hwnd_, GWL_STYLE, style);
        SetWindowPlacement(hwnd_, &savedPlacement_);
        SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        fullscreen_ = false;
    }
    onResize();
    invalidate();
}

void App::onCloseRequest() {
    commitAppName();
    saveConfig();
    if (confirmDiscard()) DestroyWindow(hwnd_);
}

}  // namespace qhb
